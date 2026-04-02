#include "Player.hpp"

namespace mc
{
	le::Vector3f Player::GetPosAtomic() const
	{
		std::atomic_thread_fence(std::memory_order_acquire);
		return m_Camera.GetEntity().GetComponentData<le::Transform>().GetPosition();
	}

	const Camera& Player::GetCamera() const
	{
		return m_Camera;
	}
}
