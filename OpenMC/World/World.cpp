#include "World.hpp"

#include <ranges>
#include <utility>

#include "../Entity/Player.hpp"

namespace mc
{
	World::World(Player& player, StitchedTerrainMaterial& worldMat)
		:
		m_Player(player),
		m_WorldMat(worldMat)
	{
		m_IsRunning.store(true, std::memory_order_relaxed);
		m_WorldGenThread = std::jthread(&World::RunGeneration, this);
	}

	World::~World()
	{
		m_IsRunning.store(false, std::memory_order_release);

		if (m_WorldGenThread.joinable())
			m_WorldGenThread.join();
	}

	uint8_t World::GetBlockAt(const int x, const int y, const int z) const
	{
		int chunkX = std::floor(x / static_cast<float>(Chunk::WIDTH));
		int chunkZ = std::floor(z / static_cast<float>(Chunk::LENGTH));
		const int chunkPosX = static_cast<uint64_t>(x) % Chunk::WIDTH;
		const int chunkPosZ = static_cast<uint64_t>(z) % Chunk::LENGTH;
		const std::pair<int, int> coord = std::make_pair(chunkX, chunkZ);

		if (!m_Chunks.contains(coord))
			return 0;

		const Chunk& chunk = m_Chunks.at(coord);
		return chunk.GetBlock(chunkPosX, y, chunkPosZ);
	}

	Chunk& World::GetChunkAt(int cx, int cz)
	{
		return m_Chunks.at(std::make_pair(cx, cz));
	}

	bool World::IsChunkLoaded(int cx, int cz) const
	{
		return m_Chunks.contains(std::make_pair(cx, cz));
	}

	void World::RunGeneration()
	{
		while (m_IsRunning.load(std::memory_order_acquire))
		{
			const le::Vector3f playerPos = m_Player.GetPosAtomic();
			const int playerChunkX = static_cast<int>(playerPos.x) / Chunk::WIDTH;
			const int playerChunkZ = static_cast<int>(playerPos.z) / Chunk::LENGTH;

			for (int z = playerChunkZ - VIEW_DISTANCE; z < playerChunkZ + VIEW_DISTANCE; z++)
				for (int x = playerChunkX - VIEW_DISTANCE; x < playerChunkX + VIEW_DISTANCE; x++)
				{
					if (IsChunkLoaded(x, z))
						continue;

					if (!m_IsRunning.load(std::memory_order_acquire))
						return;

					const int xDiff = x - playerChunkX;
					const int zDiff = z - playerChunkZ;
					const int xPow2 = xDiff * xDiff;
					const int zPow2 = zDiff * zDiff;
					if (const double realDistance = std::sqrt(xPow2 + zPow2); realDistance > VIEW_DISTANCE)
						continue;

					std::pair<int, int> key = std::make_pair(x, z);
					const Chunk& chunk = m_Chunks.insert(std::make_pair(key,
						Chunk(*this, m_WorldMat, x, z))).first->second;

					UpdateIfNeighborsPresent(chunk, x, z);
					UpdateNeighbors(x, z);
				}

			std::vector<std::pair<int, int>> toRemove;
			for (const auto& key : m_Chunks | std::views::keys)
				if (DistanceFromChunk(key, playerPos) > VIEW_DISTANCE)
					toRemove.push_back(key);

			for (auto& key : toRemove)
				m_Chunks.erase(key);

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void World::UpdateNeighbors(const int x, const int z)
	{
		if (IsChunkLoaded(x - 1, z))
			GetChunkAt(x - 1, z).UpdateMesh();
		if (IsChunkLoaded(x + 1, z))
			GetChunkAt(x + 1, z).UpdateMesh();
		if (IsChunkLoaded(x, z - 1))
			GetChunkAt(x, z - 1).UpdateMesh();
		if (IsChunkLoaded(x, z + 1))
			GetChunkAt(x, z + 1).UpdateMesh();
	}

	void World::UpdateIfNeighborsPresent(const Chunk& chunk, int x, int z) const
	{
		bool present = IsChunkLoaded(x - 1, z);
		present |= IsChunkLoaded(x + 1, z);
		present |= IsChunkLoaded(x, z - 1);
		present |= IsChunkLoaded(x, z + 1);

		if (present)
			chunk.UpdateMesh();
	}

	double World::DistanceFromChunk(const std::pair<int, int>& chunkPos, const le::Vector3f& playerPos)
	{
 		const int chunkWorldX = chunkPos.first * Chunk::WIDTH;
		const int chunkWorldZ = chunkPos.second * Chunk::LENGTH;
		const int xDistSqr = std::pow(chunkWorldX - playerPos.x, 2);
		const int zDistSqr = std::pow(chunkWorldZ - playerPos.z, 2);

		return std::sqrt(std::abs(xDistSqr + zDistSqr));
	}
}
