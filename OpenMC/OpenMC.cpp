#include "OpenMC.hpp"

namespace mc
{
	OpenMC::OpenMC(le::Application& app)
		:
		m_App(app),
		m_Sub(app.GetEventBus()),
		m_World(m_Player, m_TerrainMat)
	{
		le::RenderTarget& renderTarget = app.GetWindowManager().GetRenderTarget();
		renderTarget.SetClearColor({ 0.47f, 0.65f, 1.0f, 1.0f });
		renderTarget.SetActiveCameraID(m_Player.GetCamera().GetEntity());

		app.GetGlobalScene().SetAmbientLight(1.0f);

		m_Sub.AddEventHandler<le::UpdateEvent>(
			[this](const le::UpdateEvent& event) { Update(event); });

		le::Window& window = m_App.GetWindowManager().GetWindow();
		// window.SetCursorMode(Tether::Window::CursorMode::DISABLED);
		window.SetRawInputEnabled(true);
	}

	OpenMC::~OpenMC()
	= default;

	void OpenMC::Update(const le::UpdateEvent& event)
	{
		if (m_FpsTimer.GetElapsedMillis() >= 5000.0f)
		{
			LE_INFO("FPS: {}", m_Frames / 5);
			m_FpsTimer.Set();
			m_Frames = 0;
		}

		m_Frames++;
	}
}
