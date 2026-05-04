#include "OpenMC.hpp"

namespace mc
{
	OpenMC::OpenMC(le::Application& app)
		:
		m_App(app),
		m_Sub(app.GetEventBus()),
		m_World(m_TerrainMat),
		m_blockSelectorMat(le::Material::Create()),
		m_blockSelectorMesh(CreateBlockSelectorMesh()),
		m_Player(m_blockSelectorMat, m_blockSelectorMesh, m_World)
	{
		m_blockSelectorMesh->SetTopology(le::PrimitiveTopology::LINE_LIST);

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

		m_World.playerPos = m_Player.GetPosition();

		m_Frames++;
	}

	le::Ref<le::MeshData> OpenMC::CreateBlockSelectorMesh()
	{
		le::MeshData::Vertex3 testVertices[] =
		{
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
			{1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 1.0f, 0.0f, 0.0f},
			{1.0f, 1.0f, 1.0f, 0.0f, 0.0f},
		};

		uint32_t indices[] =
		{
			// Bottom
			0, 1,
			1, 4,
			4, 3,
			3, 0,

			// Top
			4, 5,
			5, 7,
			7, 6,
			6, 4,

			// Sides
			0, 4,
			1, 5,
			2, 6,
			3, 7,
		};

		return le::MeshData::Create(
			std::span<le::MeshData::Vertex3>(testVertices),
			std::span<uint32_t>(indices), le::MeshData::UpdateFrequency::UPDATES_ONCE
		);
	}
}
