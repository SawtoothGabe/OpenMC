#include "Camera.hpp"

#include "../World/Chunk.hpp"

namespace mc
{
	Camera::Camera()
		:
		m_cameraEntity(le::Application::Get().GetGlobalScene().CreateEntity()),
		m_Sub(le::Application::Get().GetEventBus())
	{
		le::Window& window = le::Application::Get().GetWindowManager().GetWindow();
		window.AddInputListener(*this, le::InputType::KEY);
		window.AddInputListener(*this, le::InputType::RAW_MOUSE_MOVE);
		window.AddInputListener(*this, le::InputType::MOUSE_CLICK);

		m_cameraEntity.AddComponent<le::Transform>();
		m_cameraEntity.AddComponent<le::Camera>();

		m_cameraEntity.QueryComponents<le::Transform>([](le::Transform& transform)
		{
			transform.SetPosition(le::Vector3f(-1.0f, 2.0f, 4.0f));
		});

		m_Sub.AddEventHandler<le::UpdateEvent>(
			[this](const le::UpdateEvent& e) { OnUpdate(e); });
	}

	le::Entity Camera::GetEntity() const
	{
		return m_cameraEntity;
	}

	void Camera::OnUpdate(const le::UpdateEvent& e)
	{
		const float delta = e.GetDeltaTime();

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

		float speed = 4.3f;
		if (keys.ctrl)
			speed *= 8.0f;

		normMoveDir *= speed;

		m_cameraEntity.QueryComponents<le::Camera, le::Transform>(
		[&](le::Camera& camera, le::Transform& transform)
		{
			le::Vector3f forward = camera.GetForwardVector();
			forward.y = 0;
			forward = le::Math::Normalize(forward);

			transform.AddPosition(forward * normMoveDir.x * delta);
			transform.AddPosition(camera.GetRightVector() * normMoveDir.y * delta);

			// Vertical movement
			if (keys.space)
				transform.AddPosition(le::Vector3f(0, speed, 0) * delta);
			if (keys.shift)
				transform.AddPosition(le::Vector3f(0, -speed, 0) * delta);

			const le::Vector3f newPos = transform.GetPosition();
			const int cx = std::floor(newPos.x / static_cast<float>(Chunk::WIDTH));
			const int cz = std::floor(newPos.z / static_cast<float>(Chunk::LENGTH));

			if (cx != prevCX || cz != prevCZ)
			{
				LE_INFO("Moved to chunk X{} Z{}", cx, cz);
				prevCX = cx;
				prevCZ = cz;
			}
		});
	}

	void Camera::OnKey(le::KeyInfo& info)
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

	void Camera::OnRawMouseMove(le::RawMouseMoveInfo& info)
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

		m_cameraEntity.QueryComponents<le::Transform>([&](le::Transform& transform)
		{
			transform.SetRotation(q);
		});
	}

	void Camera::OnMouseClick(le::MouseClickInfo& info)
	{
		if (m_CaptureMouse)
			return;

		le::Application::Get().GetWindowManager().GetWindow().SetCursorMode(Tether::Window::CursorMode::DISABLED);
		m_CaptureMouse = true;
	}
}
