#include "Player.hpp"

#include "../World/World.hpp"

namespace mc
{
	Player::Player(const le::Ref<le::Material>& blockSelectorMat,
			const le::Ref<le::MeshData>& blockSelectorMesh, World& world)
		:
		m_world(world),
		m_blockSelectorMat(blockSelectorMat),
		m_blockSelectorMesh(blockSelectorMesh),
		m_sub(le::Application::Get().GetEventBus())
	{
		m_raycastMat = le::Material::Create();
		m_raycastMesh = le::MeshData::Create(2, 2, le::MeshData::UpdateFrequency::UPDATES_OCCASIONALLY);
		m_raycastMesh->SetTopology(le::PrimitiveTopology::LINE_LIST);
		m_raycastMat->SetColor(le::Color(0.0f, 1.0f, 0.0f, 1.0f));

		le::Application& app = le::Application::Get();
		le::Window& window = app.GetWindowManager().GetWindow();
		window.AddInputListener(*this, le::InputType::MOUSE_CLICK);
		window.AddInputListener(*this, le::InputType::KEY);

		m_blockSelector = app.GetGlobalScene().CreateEntity();
		m_blockSelector.AddComponent<le::Transform>();
		m_blockSelector.AddComponent<le::Mesh>(le::Mesh{true, blockSelectorMesh, blockSelectorMat});

		m_raycast = app.GetGlobalScene().CreateEntity();
		m_raycast.AddComponent<le::Transform>();
		m_raycast.AddComponent<le::Mesh>();

		m_raycast.QueryComponents<le::Mesh>([this](le::Mesh& mesh)
		{
			mesh.material = m_raycastMat;
			mesh.data = m_raycastMesh;
		});

		m_sub.AddEventHandler<le::UpdateEvent>([this](const le::UpdateEvent&)
		{
			m_blockSelector.QueryComponents<le::Mesh, le::Transform>([&](le::Mesh& mesh, le::Transform& transform)
			{
				transform.SetPosition(roundedPos);
			});
		});
	}

	Player::~Player()
	{
		le::Application::Get().GetGlobalScene().DeleteEntity(m_blockSelector);
		le::Application::Get().GetGlobalScene().DeleteEntity(m_raycast);
	}

	le::Vector3f Player::GetPosition() const
	{
		return m_Camera.GetEntity().GetComponentData<le::Transform>().GetPosition();
	}

	const Camera& Player::GetCamera() const
	{
		return m_Camera;
	}

	void Player::OnMouseClick(Tether::Input::MouseClickInfo& info)
	{
		if (!info.IsPressed())
			return;

		using ClickType = le::Input::MouseClickInfo::ClickType;
		const ClickType clickType = info.GetClickType();
		if (clickType != ClickType::LEFT_BUTTON && clickType != ClickType::RIGHT_BUTTON)
			return;

		if (!m_lookAtPos)
			return;

		const le::Vector3f currentPos = m_lookAtPos.value();

		const Chunk::BlockID block = clickType == ClickType::LEFT_BUTTON ? 0 : 1;
		m_world.SetBlock(currentPos.x, currentPos.y, currentPos.z, block);
		m_world.RebuildChunkAt(currentPos);
	}

	void Player::OnKey(le::Input::KeyInfo& info)
	{
		if (!info.IsPressed())
			return;

		if (info.GetKey() == Tether::KEY_F)
		{
			FindLookAtPos();

			le::MeshData::Vertex3 testVertices[] =
			{
				{pos.x, pos.y, pos.z, 0.0f, 0.0f},
				{end.x, end.y, end.z, 0.0f, 0.0f},
			};

			uint32_t indices[] = { 0, 1 };

			m_raycastMesh->Update(std::span<le::MeshData::Vertex3>(testVertices),
				std::span<uint32_t>(indices));
		}

		if (info.GetKey() == Tether::KEY_R)
			StepBlockSearch();
	}

	void Player::FindLookAtPos()
	{
		constexpr float reachBlocks = 50.0f;

		pos = GetPosition();
		forward = m_Camera.GetEntity().GetComponentData<le::Camera>().GetForwardVector();
		delta = forward * reachBlocks;
		end = pos + delta;

		step = std::abs(delta.Max());
		delta /= step;

		currentPos = pos;
		i = 0;
	}

	void Player::StepBlockSearch()
	{
		if (i > static_cast<int>(std::floor(step)))
			return;

		roundedPos = le::Vector3f(std::floor(currentPos.x), std::floor(currentPos.y), std::floor(currentPos.z));
		if (m_world.GetBlockAt(
			static_cast<int>(roundedPos.x),
			static_cast<int>(roundedPos.y),
			static_cast<int>(roundedPos.z)))
			return;

		currentPos += delta;
		i++;
	}
}
