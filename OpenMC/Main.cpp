#include <LE/LegendEngine.hpp>

#include "OpenMC.hpp"

int main()
{
	le::Application::Create(le::GraphicsAPI::VULKAN, "OpenMC", 1280, 720);

	{
		mc::OpenMC mc(le::Application::Get());
		le::Application::Run();
	}

	le::Application::Destroy();
	return 0;
}
