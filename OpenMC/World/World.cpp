#include "World.hpp"

#include <ranges>
#include <utility>

#include "Components/PhysicsBody.hpp"
#include "Entity/Player.hpp"

namespace mc
{
	World::World(StitchedTerrainMaterial& worldMat)
		:
		m_WorldMat(worldMat),
		m_sub(le::Application::Get().GetEventBus())
	{
		m_IsRunning.store(true, std::memory_order_relaxed);
		m_WorldGenThread = std::jthread(&World::RunGeneration, this);
		scene.SetAmbientLight(1.0f);

		m_sub.AddEventHandler<le::UpdateEvent>([this](const le::UpdateEvent& e) { ProcessPhysics(e.GetDeltaTime()); });
	}

	World::~World()
	{
		m_IsRunning.store(false, std::memory_order_release);

		if (m_WorldGenThread.joinable())
			m_WorldGenThread.join();
	}

	void World::SetBlock(const int x, const int y, const int z, const Chunk::BlockID block)
	{
		int chunkX = std::floor(x / static_cast<float>(Chunk::WIDTH));
		int chunkZ = std::floor(z / static_cast<float>(Chunk::LENGTH));
		const int chunkPosX = static_cast<uint64_t>(x) % Chunk::WIDTH;
		const int chunkPosZ = static_cast<uint64_t>(z) % Chunk::LENGTH;
		const std::pair<int, int> coord = std::make_pair(chunkX, chunkZ);

		std::scoped_lock lock(m_chunksMutex);

		if (!m_Chunks.contains(coord))
			return;

		Chunk& chunk = *m_Chunks.at(coord);
		chunk.SetBlock(chunkPosX, y, chunkPosZ, block);
	}

	uint8_t World::GetBlockAt(const int x, const int y, const int z)
	{
		int chunkX = std::floor(x / static_cast<float>(Chunk::WIDTH));
		int chunkZ = std::floor(z / static_cast<float>(Chunk::LENGTH));
		const int chunkPosX = static_cast<uint64_t>(x) % Chunk::WIDTH;
		const int chunkPosZ = static_cast<uint64_t>(z) % Chunk::LENGTH;
		const std::pair<int, int> coord = std::make_pair(chunkX, chunkZ);

		std::scoped_lock lock(m_chunksMutex);

		if (!m_Chunks.contains(coord))
			return 0;

		const Chunk& chunk = *m_Chunks.at(coord);
		return chunk.GetBlock(chunkPosX, y, chunkPosZ);
	}

	void World::RebuildChunkAt(const le::Vector3f& position)
	{
		RebuildChunkAt(
			std::floor(position.x),
			std::floor(position.z)
		);
	}

	void World::RebuildChunkAt(const int x, const int z)
	{
		int chunkX = std::floor(x / static_cast<float>(Chunk::WIDTH));
		int chunkZ = std::floor(z / static_cast<float>(Chunk::LENGTH));
		const std::pair<int, int> coord = std::make_pair(chunkX, chunkZ);

		std::scoped_lock lock(m_chunksMutex);

		if (!m_Chunks.contains(coord))
			return;

		const Chunk& chunk = *m_Chunks.at(coord);
		chunk.UpdateMesh();
	}

	Chunk& World::GetChunkAt(int cx, int cz) const
	{
		return *m_Chunks.at(std::make_pair(cx, cz));
	}

	bool World::IsChunkLoaded(int cx, int cz)
	{
		return m_Chunks.contains(std::make_pair(cx, cz));
	}

	void World::RunGeneration()
	{
		while (m_IsRunning.load(std::memory_order_acquire))
		{
			const int playerChunkX = static_cast<int>(playerPos.x) / Chunk::WIDTH;
			const int playerChunkZ = static_cast<int>(playerPos.z) / Chunk::LENGTH;

			for (int z = playerChunkZ - VIEW_DISTANCE; z < playerChunkZ + VIEW_DISTANCE; z++)
				for (int x = playerChunkX - VIEW_DISTANCE; x < playerChunkX + VIEW_DISTANCE; x++)
				{
					{
						std::scoped_lock lock(m_chunksMutex);
						if (IsChunkLoaded(x, z))
							continue;
					}

					if (!m_IsRunning.load(std::memory_order_acquire))
						return;

					const int xDiff = x - playerChunkX;
					const int zDiff = z - playerChunkZ;
					const int xPow2 = xDiff * xDiff;
					const int zPow2 = zDiff * zDiff;
					if (const double realDistance = std::sqrt(xPow2 + zPow2); realDistance > VIEW_DISTANCE)
						continue;

					std::pair<int, int> key = std::make_pair(x, z);

					auto chunk = std::make_unique<Chunk>(*this, m_WorldMat, x, z);

					std::scoped_lock lock(m_chunksMutex);
					const le::Scope<Chunk>& newChunk = m_Chunks.insert(std::make_pair(key, std::move(chunk))).first->second;

					UpdateIfNeighborsPresent(*newChunk, x, z);
					UpdateNeighbors(x, z);
				}

			constexpr float deletionMarginBlocks = 17.0f;
			std::vector<std::pair<int, int>> toRemove;
			for (const auto& key : m_Chunks | std::views::keys)
				if (DistanceFromChunk(key) > VIEW_DISTANCE * Chunk::LENGTH + deletionMarginBlocks)
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

	void World::UpdateIfNeighborsPresent(const Chunk& chunk, const int x, const int z)
	{
		bool present = IsChunkLoaded(x - 1, z);
		present |= IsChunkLoaded(x + 1, z);
		present |= IsChunkLoaded(x, z - 1);
		present |= IsChunkLoaded(x, z + 1);

		if (present)
			chunk.UpdateMesh();
	}

	double World::DistanceFromChunk(const std::pair<int, int>& chunkPos) const
	{
 		const int chunkWorldX = chunkPos.first * Chunk::WIDTH;
		const int chunkWorldZ = chunkPos.second * Chunk::LENGTH;
		const int xDistSqr = std::pow(chunkWorldX - playerPos.x, 2);
		const int zDistSqr = std::pow(chunkWorldZ - playerPos.z, 2);

		return std::sqrt(std::abs(xDistSqr + zDistSqr));
	}

	bool World::IsWithinWorld(const le::Vector3f& position)
	{
		return position.y >= 0.0f && position.y < static_cast<float>(Chunk::HEIGHT);
	}

	le::Vector3f World::CoordClamped(const le::Vector3f& position)
	{
		return {
			position.x,
			std::clamp(position.y, 0.0f, static_cast<float>(Chunk::HEIGHT)),
			position.z
		};
	}

	void World::ProcessPhysics(float delta)
	{
		scene.QueryComponents<le::Transform, PhysicsBody>([this, delta](le::Transform& transform, PhysicsBody& body)
		{
			float dragCoefficient = std::pow(body.drag, delta * 1000.0f);
			body.velocity.x *= dragCoefficient;
			body.velocity.z *= dragCoefficient;

			body.velocity.y += body.gravity * delta / 20.0f;

			le::Vector3f movement = body.velocity * delta * 20.0f;
			const le::Vector3f originalMovement = movement;

			le::AABB aabb = body.aabb.Moved(transform.GetPosition());
			FindColliders(aabb.Expanded(body.velocity).Grown(1.0f));

			for (le::AABB collider : m_colliders)
				movement.x = aabb.GetClipX(collider, movement.x);

			aabb.Move(le::Vector3f(movement.x, 0.0f, 0.0f));

			for (le::AABB collider : m_colliders)
				movement.y = aabb.GetClipY(collider, movement.y);

			aabb.Move(le::Vector3f(0.0f, movement.y, 0.0f));

			for (le::AABB collider : m_colliders)
				movement.z = aabb.GetClipZ(collider, movement.z);

			body.onGround = movement.y != originalMovement.y;

			if (movement.x != originalMovement.x)
				body.velocity.x = 0.0f;
			if (body.onGround)
				body.velocity.y = 0.0f;
			if (movement.z != originalMovement.z)
				body.velocity.z = 0.0f;

			transform.AddPosition(movement);
		});
	}

	void World::FindColliders(le::AABB region)
	{
		m_colliders.clear();

		// This assumes the coordinates of min are always smaller than max

		for (int z = std::floor(region.min.z); z <= std::ceil(region.max.z); z++)
			for (int y = std::floor(region.min.y); y <= std::ceil(region.max.y); y++)
				for (int x = std::floor(region.min.x); x <= std::ceil(region.max.x); x++)
				{
					if (!GetBlockAt(x, y, z))
						continue;

					m_colliders.emplace_back(le::Vector3f(x, y, z), le::Vector3f(x + 1, y + 1, z + 1));
				}
	}
}
