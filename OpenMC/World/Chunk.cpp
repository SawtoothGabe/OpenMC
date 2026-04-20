#include "Chunk.hpp"

#include <LE/Application.hpp>

#include "World.hpp"
#include "../Graphics/StitchedTerrainMaterial.hpp"

namespace mc
{
    Chunk::Chunk(World &world, StitchedTerrainMaterial& material, const int cx, const int cz)
        :
        m_World(world),
        m_Cx(cx),
        m_Cz(cz),
        m_Material(material)
    {
        le::EntityCreator creator;

        le::Application& app = le::Application::Get();
        m_entityID = creator.GetUID();
        m_mesh = le::MeshData::Create(BLOCK_COUNT, BLOCK_COUNT,
            le::MeshData::UpdateFrequency::UPDATES_OCCASIONALLY);
        m_Data = std::make_unique<uint8_t[]>(BLOCK_COUNT);

        le::Transform transform;
        transform.SetPosition(le::Vector3f(cx * WIDTH, 0, cz * LENGTH));

        le::Mesh mesh;
        //mesh.data = m_mesh;
        mesh.material = material.Get();

        creator.AddComponent<le::Transform>(transform);
        creator.AddComponent<le::Mesh>(mesh);
        app.GetGlobalScene().EnqueueEntityCreation(std::move(creator));

        Generate();
    }

    Chunk::~Chunk()
    {
        if (m_entityID == 0)
            return;

        le::Application::Get().GetGlobalScene().EnqueueEntityDeletion(m_entityID);
    }

    void Chunk::Generate()
    {
        for (uint16_t y = 0; y < HEIGHT; y++)
            for (uint16_t z = 0; z < LENGTH; z++)
                for (uint16_t x = 0; x < WIDTH; x++)
                    m_Data[GetBlockIndex(x, y, z)] = GenerateBlockAt(x, y, z);
    }

    void Chunk::UpdateMesh() const
    {
        Vertices vertices;
        Indices indices;

        vertices.reserve(BLOCK_COUNT);
        indices.reserve(BLOCK_COUNT);

        // There is a major problem with generating chunks: redundant data.
        // The obvious one is: if two solid blocks are neighbors,
        // the m_Material.GetCoordsAtIndex(face.north) connecting them shouldn't be rendered since it isn't visible.

        for (uint16_t y = 0; y < HEIGHT; y++)
            for (uint16_t z = 0; z < LENGTH; z++)
            {
                for (uint16_t x = 0; x < WIDTH; x++)
                {
                    le::Vector3f pos(x, y, z);
                    const size_t index = GetBlockIndex(x, y, z);
                    const BlockID id = m_Data[index];
                    if (id == 0)
                        continue;
                    
                    auto [top, bottom, north, south, east, west] =
                        mc::StitchedTerrainMaterial::GetFaceSubtextureIndices(id);
                    
                    // North
                    if (!GetBlockInBounds(x, y, z - 1))
                        AddNorthFace(vertices, indices, pos, m_Material.GetCoordsAtIndex(north));

                    // South
                    if (!GetBlockInBounds(x, y, z + 1))
                        AddSouthFace(vertices, indices, pos, m_Material.GetCoordsAtIndex(south));

                    // East
                    if (!GetBlockInBounds(x + 1, y, z))
                        AddEastFace(vertices, indices, pos, m_Material.GetCoordsAtIndex(east));

                    // West
                    if (!GetBlockInBounds(x - 1, y, z))
                        AddWestFace(vertices, indices, pos, m_Material.GetCoordsAtIndex(west));

                    // Top
                    if (!GetBlockInBounds(x, y + 1, z))
                        AddTopFace(vertices, indices, pos, m_Material.GetCoordsAtIndex(top));

                    // Bottom
                    if (!GetBlockInBounds(x, y - 1, z))
                        AddBottomFace(vertices, indices, pos, m_Material.GetCoordsAtIndex(bottom));
                }
            }

        m_mesh->Update(std::span(vertices), std::span(indices));
    }

    uint8_t Chunk::GetBlockInBounds(const int x, const int y, const int z) const
    {
        if (y < 0)
            return 0;

        if (x < 0 || x >= WIDTH || y >= HEIGHT || z < 0 || z >= LENGTH)
        {
            const int absChunkX = m_Cx * WIDTH;
            const int absChunkZ = m_Cz * LENGTH;
            return m_World.GetBlockAt(x + absChunkX, y, z + absChunkZ);
        }

        return m_Data[GetBlockIndex(x, y, z)];
    }

    uint8_t Chunk::GetBlock(int x, int y, int z) const
    {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || z < 0 || z >= LENGTH)
            return 0;

        return m_Data[GetBlockIndex(x, y, z)];
    }

    uint8_t Chunk::GenerateBlockAt(const int x, const int y, const int z) const
    {
        const float worldX = x + m_Cx * WIDTH;
        const float worldZ = z + m_Cz * LENGTH;
        const float perlin = m_World.m_Noise.octave2D_01(worldX * 0.01f, worldZ * 0.01f, 3);
        const float yValue = perlin * 30 + 20;

        if (y > yValue)
            return 0;

        return rand() % 5 + 1;
    }

    size_t Chunk::GetBlockIndex(const uint16_t x, const uint16_t y, const uint16_t z)
    {
        size_t index = x;
        index += z * WIDTH;
        index += y * WIDTH * LENGTH;

        return index;
    }

    size_t Chunk::GetCornerIndex(uint16_t x, uint16_t y, uint16_t z)
    {
        constexpr size_t WIDTH_P1 = WIDTH + 1;
        constexpr size_t WP1XLP1 = WIDTH_P1 * (LENGTH + 1);

        size_t index = x;
        index += z * WIDTH_P1;
        index += y * WP1XLP1;

        return index;
    }

    size_t Chunk::AddVertex(Vertices& v, const le::Vector3f& pos,
        const le::Vector3f& block, const le::Vector2f& coord)
    {
        const le::Vector3f world = pos + block;

        le::MeshData::Vertex3 vertex;
        vertex.position[0] = world.x;
        vertex.position[1] = world.y;
        vertex.position[2] = world.z;
        vertex.texcoord[0] = coord.x;
        vertex.texcoord[1] = coord.y;

        const size_t index = v.size();
        v.push_back(vertex);

        return index;
    }

    void Chunk::AddFace(Indices& i, const size_t i0, const size_t i1,
        const size_t i2, const size_t i3)
    {
        i.push_back(i0);
        i.push_back(i1);
        i.push_back(i2);
        i.push_back(i1);
        i.push_back(i3);
        i.push_back(i2);
    }

    void Chunk::AddNorthFace(Vertices& v, Indices& i, const le::Vector3f& pos, 
			const Texcoords& coords)
    {
        const size_t v0 = AddVertex(v, { 1.0f, 1.0f, 0.0f }, pos, coords.topLeft);
        const size_t v1 = AddVertex(v, { 0.0f, 1.0f, 0.0f }, pos, coords.topRight);
        const size_t v2 = AddVertex(v, { 1.0f, 0.0f, 0.0f }, pos, coords.bottomLeft);
        const size_t v3 = AddVertex(v, { 0.0f, 0.0f, 0.0f }, pos, coords.bottomRight);
        
        AddFace(i, v0, v1, v2, v3);
    }

    void Chunk::AddSouthFace(Vertices& v, Indices& i, const le::Vector3f& pos, 
			const Texcoords& coords)
    {
        const size_t v0 = AddVertex(v, { 0.0f, 1.0f, 1.0f }, pos, coords.topLeft);
        const size_t v1 = AddVertex(v, { 1.0f, 1.0f, 1.0f }, pos, coords.topRight);
        const size_t v2 = AddVertex(v, { 0.0f, 0.0f, 1.0f }, pos, coords.bottomLeft);
        const size_t v3 = AddVertex(v, { 1.0f, 0.0f, 1.0f }, pos, coords.bottomRight);

        AddFace(i, v0, v1, v2, v3);
    }

    void Chunk::AddEastFace(Vertices& v, Indices& i, const le::Vector3f& pos, 
			const Texcoords& coords)
    {
        const size_t v0 = AddVertex(v, { 1.0f, 1.0f, 1.0f }, pos, coords.topLeft);
        const size_t v1 = AddVertex(v, { 1.0f, 1.0f, 0.0f }, pos, coords.topRight);
        const size_t v2 = AddVertex(v, { 1.0f, 0.0f, 1.0f }, pos, coords.bottomLeft);
        const size_t v3 = AddVertex(v, { 1.0f, 0.0f, 0.0f }, pos, coords.bottomRight);

        AddFace(i, v0, v1, v2, v3);
    }

    void Chunk::AddWestFace(Vertices& v, Indices& i, const le::Vector3f& pos, 
			const Texcoords& coords)
    {
        const size_t v0 = AddVertex(v, { 0.0f, 1.0f, 0.0f }, pos, coords.topLeft);
        const size_t v1 = AddVertex(v, { 0.0f, 1.0f, 1.0f }, pos, coords.topRight);
        const size_t v2 = AddVertex(v, { 0.0f, 0.0f, 0.0f }, pos, coords.bottomLeft);
        const size_t v3 = AddVertex(v, { 0.0f, 0.0f, 1.0f }, pos, coords.bottomRight);

        AddFace(i, v0, v1, v2, v3);
    }

    void Chunk::AddTopFace(Vertices& v, Indices& i, const le::Vector3f& pos, 
			const Texcoords& coords)
    {
        const size_t v0 = AddVertex(v, { 1.0f, 1.0f, 1.0f }, pos, coords.topLeft);
        const size_t v1 = AddVertex(v, { 0.0f, 1.0f, 1.0f }, pos, coords.topRight);
        const size_t v2 = AddVertex(v, { 1.0f, 1.0f, 0.0f }, pos, coords.bottomLeft);
        const size_t v3 = AddVertex(v, { 0.0f, 1.0f, 0.0f }, pos, coords.bottomRight);

        AddFace(i, v0, v1, v2, v3);
    }

    void Chunk::AddBottomFace(Vertices& v, Indices& i, const le::Vector3f& pos, 
			const Texcoords& coords)
    {
        const size_t v0 = AddVertex(v, { 1.0f, 0.0f, 0.0f }, pos, coords.topLeft);
        const size_t v1 = AddVertex(v, { 0.0f, 0.0f, 0.0f }, pos, coords.topRight);
        const size_t v2 = AddVertex(v, { 1.0f, 0.0f, 1.0f }, pos, coords.bottomLeft);
        const size_t v3 = AddVertex(v, { 0.0f, 0.0f, 1.0f }, pos, coords.bottomRight);

        AddFace(i, v0, v1, v2, v3);
    }
}
