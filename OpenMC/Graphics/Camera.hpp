#pragma once

#include <LE/LegendEngine.hpp>

namespace mc
{
	class Camera : public le::InputListener
	{
	public:
		Camera();

		[[nodiscard]] le::Entity GetEntity() const;
	private:
		void OnUpdate(const le::UpdateEvent& e);
		void OnKey(le::KeyInfo& info) override;
		void OnRawMouseMove(le::RawMouseMoveInfo& info) override;
		void OnMouseClick(le::MouseClickInfo& info) override;

		le::Entity m_cameraEntity;
		le::EventBusSubscriber m_Sub;

		struct Keys
		{
			bool w = false;
			bool a = false;
			bool s = false;
			bool d = false;
			bool space = false;
			bool shift = false;
			bool ctrl = false;
		};
		Keys keys;

		float horizontal = 0.0f;
		float vertical = 0.0f;
		bool m_CaptureMouse = true;

		int prevCX = 0, prevCZ = 0;
	};
}
