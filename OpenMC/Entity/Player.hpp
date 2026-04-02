#pragma once

#include "../Graphics/Camera.hpp"

namespace mc
{
	class Player
	{
	public:
		le::Vector3f GetPosAtomic() const;
		const Camera& GetCamera() const;
	private:
		Camera m_Camera;
	};
}
