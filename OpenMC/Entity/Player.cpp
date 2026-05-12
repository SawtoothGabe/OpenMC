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
		window.AddInputListener(*this, le::InputType::KEY);

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
					transform.SetPosition(*m_lookAtPos);
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
		const le::Vector3f camPos = m_Camera.GetEntity().GetComponentData<le::Transform>().GetPosition();
		const le::Vector3f camDir = m_Camera.GetEntity().GetComponentData<le::Camera>().GetForwardVector();
		const le::Ray ray(camPos, camDir);

		le::Vector3f coord = camPos;
		if (!World::IsWithinWorld(camPos))
		{
			float t1 = -ray.org.y * ray.inv.y;
            float t2 = (static_cast<float>(Chunk::HEIGHT) - ray.org.y) * ray.inv.y;
            float tmin = std::max(0.0f, std::min(t1, t2));

			if (std::max(t1, t2) < tmin)
				return std::nullopt;

			coord = World::CoordClamped(ray.org + ray.dir * tmin);
		}

		le::Vector3f step = ray.inv.Map([](const float a) { return le::Math::Signum(a); });
		le::Vector3f select = ray.inv.Map([](const float a) { return 0.5f + 0.5f * le::Math::Signum(a); });
		le::Vector3f planes = coord.Floored() + select;
		le::Vector3f t = (planes - ray.org) * ray.inv;

		// t is increased by the sign of the direction times the inverse of the direction,
		// which is the same thing as the absolute value of the inverse direction.
		le::Vector3f delta = ray.inv.Abs();

		constexpr float reach = 5.0f;

		while (World::IsWithinWorld(coord) && ray.org.Distance(coord) < reach)
		{
			if (m_world.GetBlockAt(std::floor(coord.x), std::floor(coord.y), std::floor(coord.z)))
				return coord.Floored();

			if (t.x < t.y)
			{
				if (t.x < t.z)
				{
					coord.x += step.x;
					t.x += delta.x;
				}
				else
				{
					coord.z += step.z;
					t.z += delta.z;
				}
			}
			else
			{
				if (t.y < t.z)
				{
					coord.y += step.y;
					t.y += delta.y;
				}
				else
				{
					coord.z += step.z;
					t.z += delta.z;
				}
			}
		}

		return std::nullopt;
	}
}
