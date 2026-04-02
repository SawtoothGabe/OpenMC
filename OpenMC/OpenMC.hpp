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
		le::Application& m_App;
		le::EventBusSubscriber m_Sub;

		StitchedTerrainMaterial m_TerrainMat;

		Player m_Player;
		World m_World;

		le::Stopwatch m_FpsTimer;
		size_t m_Frames = 0;
	};
}
