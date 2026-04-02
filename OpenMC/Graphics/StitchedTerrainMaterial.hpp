#pragma once

#include "LE/LegendEngine.hpp"

namespace mc
{
    class StitchedTerrainMaterial
    {
    public:
        struct SubtextureCoords
        {
            le::Vector2f topLeft;
            le::Vector2f topRight;
            le::Vector2f bottomLeft;
            le::Vector2f bottomRight;
        };

        struct FaceSubtextureIndices
        {
            size_t top = 0;
            size_t bottom = 0;
            size_t north = 0;
            size_t south = 0;
            size_t east = 0;
            size_t west = 0;
        };

        StitchedTerrainMaterial();

        le::Material* Get() const;
        SubtextureCoords GetCoordsAtIndex(size_t index) const;
        static FaceSubtextureIndices GetFaceSubtextureIndices(uint8_t blockID);
    private:
        le::Ref<le::Material> m_Material;
        le::Ref<le::Texture2D> m_Texture;

        size_t m_Width = 0;
        size_t m_Height = 0;
        size_t m_Columns = 0;
        size_t m_Rows = 0;

        static constexpr size_t BLOCK_TEXTURE_WIDTH = 16;
        static constexpr size_t BLOCK_TEXTURE_HEIGHT = 16;
    };
}
