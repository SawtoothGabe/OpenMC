#include "Player.hpp"

#include "Components/PhysicsBody.hpp"
#include "World/World.hpp"

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
		m_entity = m_world.scene.CreateEntity();
		m_entity.AddComponent<le::Transform>();
		m_entity.AddComponent<PhysicsBody>();
		m_entity.QueryComponents<le::Transform, PhysicsBody>([](le::Transform& transform, PhysicsBody& body)
		{
			transform.SetPosition(le::Vector3f(0.0f, 60.0f, 0.0f));

			body.aabb.min = le::Vector3f(-0.15f, 0.0f, -0.15f);
			body.aabb.max = le::Vector3f(0.15f, 1.8f, 0.15f);
		});

		le::Window& window = le::Application::Get().GetWindowManager().GetWindow();
		window.AddInputListener(*this, le::InputType::KEY);
		window.AddInputListener(*this, le::InputType::RAW_MOUSE_MOVE);
		window.AddInputListener(*this, le::InputType::MOUSE_CLICK);

		m_blockSelector = world.scene.CreateEntity();
		m_blockSelector.AddComponent<le::Transform>();
		m_blockSelector.AddComponent<le::Mesh>(le::Mesh{true, blockSelectorMesh, blockSelectorMat});

		m_camera = world.scene.CreateEntity();
		m_camera.AddComponent<le::Transform>();
		m_camera.AddComponent<le::Camera>();
		m_camera.QueryComponents<le::Camera>([](le::Camera& camera)
		{
			camera.SetNearZ(0.05f);
			camera.SetFarZ(500.0f);
		});

		m_sub.AddEventHandler<le::UpdateEvent>([this](const le::UpdateEvent& e){ OnUpdate(e); });
	}

	Player::~Player()
	{
		m_world.scene.DeleteEntity(m_entity);
		m_world.scene.DeleteEntity(m_blockSelector);
		m_world.scene.DeleteEntity(m_camera);
	}

	le::Vector3f Player::GetPosition() const
	{
		return m_entity.GetComponentData<le::Transform>().GetPosition();
	}

	le::Entity Player::GetCamera() const
	{
		return m_camera;
	}

	void Player::OnMouseClick(Tether::Input::MouseClickInfo& info)
	{
		if (!m_CaptureMouse)
		{
			le::Application::Get().GetWindowManager().GetWindow().SetCursorMode(Tether::Window::CursorMode::DISABLED);
			m_CaptureMouse = true;
			return;
		}

		if (!info.IsPressed())
			return;

		using ClickType = le::Input::MouseClickInfo::ClickType;
		const ClickType clickType = info.GetClickType();
		if (clickType != ClickType::LEFT_BUTTON && clickType != ClickType::RIGHT_BUTTON)
			return;

		if (!m_lookAtPos)
			return;

		le::Vector3f block = m_lookAtPos.value();

		if (clickType == ClickType::LEFT_BUTTON)
			m_world.SetBlock(std::floor(block.x), std::floor(block.y), std::floor(block.z), 0);
		else
		{
			switch (m_targetFace)
			{
				case Block::Face::NORTH: block.z -= 1.0f; break;
				case Block::Face::SOUTH: block.z += 1.0f; break;
				case Block::Face::EAST: block.x += 1.0f; break;
				case Block::Face::WEST: block.x -= 1.0f; break;
				case Block::Face::TOP: block.y += 1.0f; break;
				case Block::Face::BOTTOM: block.y -= 1.0f; break;
			}

			const auto playerPos = m_entity.GetComponentData<le::Transform>().GetPosition();
			const auto playerAABB = m_entity.GetComponentData<PhysicsBody>().aabb.Moved(playerPos);
			const auto blockAABB = le::AABB(block, block + le::Vector3f(1.0f));
			if (le::PhysicsTests::Intersects(playerAABB, blockAABB))
				return;

			m_world.SetBlock(std::floor(block.x), std::floor(block.y), std::floor(block.z), 1);
		}

		m_world.RebuildChunkAt(block);
	}

	void Player::OnUpdate(const le::UpdateEvent& e)
	{
		UpdateBlockSelector();
		ProcessMovement(e.GetDeltaTime());
	}

	void Player::OnKey(le::KeyInfo& info)
	{
		const bool pressed = info.IsPressed();
		switch (info.GetKey())
		{
			case le::Keycodes::KEY_W: keys.w = pressed; break;
			case le::Keycodes::KEY_A: keys.a = pressed; break;
			case le::Keycodes::KEY_S: keys.s = pressed; break;
			case le::Keycodes::KEY_D: keys.d = pressed; break;

			case le::Keycodes::KEY_SPACE: keys.space = pressed; break;
			case le::Keycodes::KEY_LEFT_SHIFT: keys.shift = pressed; break;
			case le::Keycodes::KEY_LEFT_CONTROL: keys.ctrl = pressed; break;

			case le::Keycodes::KEY_ESCAPE:
			{
				le::Application::Get().GetWindowManager().GetWindow().SetCursorMode(Tether::Window::CursorMode::NORMAL);
				m_CaptureMouse = false;
			}
			break;

			default: break;
		}
	}

	void Player::OnRawMouseMove(le::RawMouseMoveInfo& info)
	{
		if (!m_CaptureMouse)
			return;

		constexpr float sense = 0.04f;

		horizontal += static_cast<float>(info.GetRawX()) * sense;
		vertical += static_cast<float>(info.GetRawY()) * sense;

		if (vertical > 89.9f)
			vertical = 89.9f;
		if (vertical < -89.9f)
			vertical = -89.9f;

		le::Quaternion q = le::Math::AngleAxis(le::Math::Radians(vertical), le::Vector3f(1, 0, 0));
		q *= le::Math::AngleAxis(le::Math::Radians(horizontal), le::Vector3f(0, 1, 0));

		le::Window& window = le::Application::Get().GetWindowManager().GetWindow();
		window.SetCursorPos(window.GetWidth() / 2, window.GetHeight() / 2);

		m_camera.QueryComponents<le::Transform>([&](le::Transform& transform)
		{
			transform.SetRotation(q);
		});
	}

	void Player::UpdateBlockSelector()
	{
		m_lookAtPos = FindLookAtPos();
		m_blockSelector.QueryComponents<le::Mesh, le::Transform>([&](le::Mesh& mesh, le::Transform& transform)
		{
			mesh.enabled = m_lookAtPos.has_value();
			if (mesh.enabled)
				transform.SetPosition(*m_lookAtPos);
		});
	}

	void Player::ProcessMovement(float delta)
	{
		auto cameraData = m_camera.GetComponentData<le::Camera>();
		le::Vector2f direction = GetMoveDirection();
		le::Vector3f forward = cameraData.GetForwardVector();
		forward.y = 0;
		forward.Normalize();

		float speed = 4.3f;
		if (keys.ctrl)
			speed *= 1.3f;

		direction *= speed;

		m_entity.QueryComponents<PhysicsBody>([&](PhysicsBody& body)
		{
			body.velocity += forward * direction.x * delta;
			body.velocity += cameraData.GetRightVector() * direction.y * delta;

			// Vertical movement
			if (keys.space && body.onGround)
				body.velocity += le::Vector3f(0, 0.5f, 0);
		});

		m_camera.QueryComponents<le::Transform>([&](le::Transform& transform)
		{
			const le::Vector3f newPos = transform.GetPosition();
			const int cx = std::floor(newPos.x / static_cast<float>(Chunk::WIDTH));
			const int cz = std::floor(newPos.z / static_cast<float>(Chunk::LENGTH));

			if (cx != prevCX || cz != prevCZ)
			{
				LE_INFO("Moved to chunk X{} Z{}", cx, cz);
				prevCX = cx;
				prevCZ = cz;
			}

			constexpr le::Vector3f eyeOffset(0.0f, 1.5f, 0.0f);

			const auto playerTransform = m_entity.GetComponentData<le::Transform>();
			transform.SetPosition(playerTransform.GetPosition() + eyeOffset);
		});
	}

	le::Vector2f Player::GetMoveDirection() const
	{
		le::Vector2f moveDir;
		if (keys.w)
			moveDir += le::Vector2f(1, 0);
		if (keys.a)
			moveDir += le::Vector2f(0, -1);
		if (keys.s)
			moveDir += le::Vector2f(-1, 0);
		if (keys.d)
			moveDir += le::Vector2f(0, 1);

		le::Vector2f normMoveDir = le::Math::Normalize(moveDir);

		return normMoveDir;
	}

	std::optional<le::Vector3f> Player::FindLookAtPos()
	{
		const le::Vector3f camPos = m_camera.GetComponentData<le::Transform>().GetPosition();
		const le::Vector3f camDir = m_camera.GetComponentData<le::Camera>().GetForwardVector();
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

		int axis = 0;
		while (World::IsWithinWorld(coord) && ray.org.Distance(coord) < reach)
		{
			if (m_world.GetBlockAt(std::floor(coord.x), std::floor(coord.y), std::floor(coord.z)))
			{
				switch (axis)
				{
					case 0: m_targetFace = step.x < 0.0f ? Block::Face::EAST : Block::Face::WEST; break;
					case 1: m_targetFace = step.y < 0.0f ? Block::Face::TOP : Block::Face::BOTTOM; break;
					case 2: m_targetFace = step.z < 0.0f ? Block::Face::SOUTH : Block::Face::NORTH; break;

					default: LE_ASSERT(false, "Invalid axis");
				}

				return coord.Floored();
			}

			if (t.x < t.y)
			{
				if (t.x < t.z)
				{
					coord.x += step.x;
					t.x += delta.x;
					axis = 0;
				}
				else
				{
					coord.z += step.z;
					t.z += delta.z;
					axis = 2;
				}
			}
			else
			{
				if (t.y < t.z)
				{
					coord.y += step.y;
					t.y += delta.y;
					axis = 1;
				}
				else
				{
					coord.z += step.z;
					t.z += delta.z;
					axis = 2;
				}
			}
		}

		return std::nullopt;
	}
}
