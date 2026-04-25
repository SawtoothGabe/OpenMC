#pragma once
#include <thread>

#include "Chunk.hpp"

#include <typeindex>

#include "../Common/PerlinNoise.hpp"

namespace mc
{
	class StitchedTerrainMaterial;
	class Player;

	struct PairHash
	{
		size_t operator()(const std::pair<int64_t, int64_t>& p) const
		{
			const size_t h1 = std::hash<int64_t>()(p.first);
			const size_t h2 = std::hash<int64_t>()(p.second);
			return h1 ^ (h2 * 0x9e3779b97f4a7c15 + 0x6c62272e07bb0142 + (h1 << 6) + (h1 >> 2));
		}
	};

	class World
	{
	public:
		static constexpr int VIEW_DISTANCE = 4;

		explicit World(StitchedTerrainMaterial& worldMat);
		~World();

		void SetBlock(int x, int y, int z, Chunk::BlockID block) const;
		uint8_t GetBlock(const le::Vector3f& position) const;
		uint8_t GetBlockAt(int x, int y, int z) const;
		void RebuildChunkAt(const le::Vector3f& position);
		void RebuildChunkAt(int x, int y, int z) const;

		le::Vector3f playerPos;
		siv::PerlinNoise m_Noise;
	protected:
		void UpdateNeighbors(int x, int z);
		void UpdateIfNeighborsPresent(const Chunk& chunk, int x, int z) const;
	private:
		Chunk& GetChunkAt(int cx, int cz) const;
		bool IsChunkLoaded(int cx, int cz) const;
		double DistanceFromChunk(const std::pair<int, int>& chunkPos) const;
		void RunGeneration();

		std::jthread m_WorldGenThread;

		std::atomic_bool m_IsRunning = false;
		std::unordered_map<std::pair<int, int>, le::Scope<Chunk>, PairHash> m_Chunks;

		StitchedTerrainMaterial& m_WorldMat;
	};
}
