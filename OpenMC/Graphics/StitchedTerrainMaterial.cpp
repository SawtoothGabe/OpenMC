#include "StitchedTerrainMaterial.hpp"

namespace mc
{
    StitchedTerrainMaterial::StitchedTerrainMaterial()
        :
        m_Material(le::Material::Create())
    {
        const std::string path = "Assets/Common/res/TitleUpdate/res/terrain.png";
        m_Texture = le::Texture2D::Create(le::TextureData::FromFile(path).get());

        m_Material->SetTexture(m_Texture);

        m_Width = m_Texture->GetWidth();
        m_Height = m_Texture->GetHeight();
        m_Columns = m_Width / BLOCK_TEXTURE_WIDTH;
        m_Rows = m_Height / BLOCK_TEXTURE_HEIGHT;
    }

    le::Ref<le::Material> StitchedTerrainMaterial::Get() const
    {
        return m_Material;
    }

    StitchedTerrainMaterial::SubtextureCoords StitchedTerrainMaterial::GetCoordsAtIndex(const size_t index) const
    {
        const float u = static_cast<float>(index % m_Columns) / static_cast<float>(m_Columns);
        const float v = static_cast<float>(index / m_Columns) / static_cast<float>(m_Rows);

        const float subtextureWidth = 1.0f / static_cast<float>(m_Columns);
        const float subtextureHeight = 1.0f / static_cast<float>(m_Rows);

        SubtextureCoords result
        {
            .topLeft     = { u, v },
            .topRight    = { u + subtextureWidth, v },
            .bottomLeft  = { u, v + subtextureHeight },
            .bottomRight = { u + subtextureWidth, v + subtextureHeight },
        };

        return result;
    }

    StitchedTerrainMaterial::FaceSubtextureIndices StitchedTerrainMaterial::GetFaceSubtextureIndices(const uint8_t blockID)
    {
        static FaceSubtextureIndices indices[] =
        {
            { 1, 1, 1, 1, 1, 1 },
            { 2, 2, 2, 2, 2, 2 },
            { 0, 2, 3, 3, 3, 3 },
            { 4, 4, 4, 4, 4, 4 },
            { 5, 5, 5, 5, 5, 5 },
            { 6, 6, 6, 6, 6, 6 },
        };

        if (blockID > std::size(indices))
            return {};

        // -1 is to ignore air
        return indices[blockID - 1];
    }
}
