#pragma once

#include <LE/LegendEngine.hpp>

#include "Entity/Player.hpp"
#include "Graphics/StitchedTerrainMaterial.hpp"
#include "World/World.hpp"

namespace mc
{
	class OpenMC
	{
	public:
		explicit OpenMC(le::Application& app);
		~OpenMC();

		void Update(const le::UpdateEvent& event);
	private:
		static le::Ref<le::MeshData> CreateBlockSelectorMesh();

		le::Application& m_App;
		le::EventBusSubscriber m_Sub;

		StitchedTerrainMaterial m_TerrainMat;

		le::Ref<le::Material> m_blockSelectorMat;
		le::Ref<le::MeshData> m_blockSelectorMesh;

		World m_World;
		Player m_Player;

		le::Stopwatch m_FpsTimer;
		size_t m_Frames = 0;
	};
}
