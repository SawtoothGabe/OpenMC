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
		le::Application& app = le::Application::Get();
		le::Window& window = app.GetWindowManager().GetWindow();
		window.AddInputListener(*this, le::InputType::MOUSE_CLICK);

		m_blockSelector = app.GetGlobalScene().CreateEntity();
		m_blockSelector.AddComponent<le::Transform>();
		m_blockSelector.AddComponent<le::Mesh>(le::Mesh{true, blockSelectorMesh, blockSelectorMat});

		m_sub.AddEventHandler<le::UpdateEvent>([this](const le::UpdateEvent&)
		{
			m_lookAtPos = FindLookAtPos();
			m_blockSelector.QueryComponents<le::Mesh, le::Transform>([&](le::Mesh& mesh, le::Transform& transform)
			{
				mesh.enabled = m_lookAtPos.has_value();
				if (mesh.enabled)
				{
					const le::Vector3f pos = m_lookAtPos.value();
					transform.SetPosition({
						std::floor(pos.x),
						std::floor(pos.y),
						std::floor(pos.z),
					});
				}
			});
		});
	}

	Player::~Player()
	{
		le::Application::Get().GetGlobalScene().DeleteEntity(m_blockSelector);
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

	std::optional<le::Vector3f> Player::FindLookAtPos() const
	{
		constexpr float reachBlocks = 50.0f;

		const le::Vector3f pos = GetPosition();
		const le::Vector3f forward = m_Camera.GetEntity().GetComponentData<le::Camera>().GetForwardVector();
		const le::Vector3f end = pos + forward * reachBlocks;
		le::Vector3f delta = end - pos;

		const float step = std::abs(delta.Max());
		delta /= step;

		le::Vector3f currentPos = pos;
		for (int i = 0; i <= static_cast<int>(std::floor(step)); i++)
		{
			if (m_world.GetBlock(currentPos))
				return currentPos;

			currentPos += delta;
		}

		return std::nullopt;
	}
}
