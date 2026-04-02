#pragma once

#include <LE/LegendEngine.hpp>
#include "../Graphics/StitchedTerrainMaterial.hpp"

namespace mc
{
	class StitchedTerrainMaterial;
	class World;

	class Chunk
	{
	public:
		static constexpr int64_t WIDTH = 16;
		static constexpr int64_t LENGTH = 16;
		static constexpr int64_t HEIGHT = 256;
		static constexpr size_t BLOCK_COUNT = WIDTH * HEIGHT * LENGTH;
		
		using BlockID = uint8_t;

		Chunk(World& world, StitchedTerrainMaterial& material, int cx, int cz);
		Chunk(Chunk&& other) noexcept;
		~Chunk();

		void Generate();
		void UpdateMesh() const;
		uint8_t GetBlock(int x, int y, int z) const;

		le::Entity m_entity;
	private:
		using Index = uint32_t;
		using Indices = std::vector<Index>;
		using Vertices = std::vector<le::MeshData::Vertex3>;
		using Texcoords = StitchedTerrainMaterial::SubtextureCoords;
		
		uint8_t GetBlockInBounds(int x, int y, int z) const;
		uint8_t GenerateBlockAt(int x, int y, int z) const;
		static size_t GetBlockIndex(uint16_t x, uint16_t y, uint16_t z);
		static size_t GetCornerIndex(uint16_t x, uint16_t y, uint16_t z);

		static size_t AddVertex(Vertices& v, const le::Vector3f& pos,
			const le::Vector3f& block, const le::Vector2f& coord);
		static void AddFace(Indices& i, size_t i0, size_t i1, size_t i2, size_t i3);

		static void AddNorthFace(Vertices& v, Indices& i, const le::Vector3f& pos,
			const Texcoords& coords);
		static void AddSouthFace(Vertices& v, Indices& i, const le::Vector3f& pos,
			const Texcoords& coords);
		static void AddEastFace(Vertices& v, Indices& i, const le::Vector3f& pos,
			const Texcoords& coords);
		static void AddWestFace(Vertices& v, Indices& i, const le::Vector3f& pos,
			const Texcoords& coords);
		static void AddTopFace(Vertices& v, Indices& i, const le::Vector3f& pos,
			const Texcoords& coords);
		static void AddBottomFace(Vertices& v, Indices& i, const le::Vector3f& pos,
			const Texcoords& coords);

		World& m_World;
		int m_Cx, m_Cz;

		le::Ref<le::MeshData> m_mesh;

		le::Scope<BlockID[]> m_Data;
		StitchedTerrainMaterial& m_Material;
	};
}
