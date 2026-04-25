#pragma once

#include "../Graphics/Camera.hpp"

namespace mc
{
	class World;
	class Player : public le::InputListener
	{
	public:
		explicit Player(World& world);

		le::Vector3f GetPosition() const;
		const Camera& GetCamera() const;
	private:
		void OnMouseClick(Tether::Input::MouseClickInfo& info) override;

		World& m_world;
		Camera m_Camera;
	};
}
