#include "Player.hpp"

#include "../World/World.hpp"

namespace mc
{
	Player::Player(World& world)
		:
		m_world(world)
	{
		le::Window& window = le::Application::Get().GetWindowManager().GetWindow();
		window.AddInputListener(*this, le::InputType::MOUSE_CLICK);
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
		ClickType clickType = info.GetClickType();
		if (clickType != ClickType::LEFT_BUTTON && clickType != ClickType::RIGHT_BUTTON)
			return;

		constexpr float reachBlocks = 5.0f;

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
			{
				LE_INFO("Hit block x{} y{} z{}", currentPos.x, currentPos.y, currentPos.z);

				Chunk::BlockID block = clickType == ClickType::LEFT_BUTTON ? 0 : 1;
				m_world.SetBlock(currentPos.x, currentPos.y, currentPos.z, block);
				m_world.RebuildChunkAt(currentPos);
				return;
			}

			currentPos += delta;
		}
	}
}
