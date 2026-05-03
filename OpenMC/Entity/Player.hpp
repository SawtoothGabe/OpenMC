#pragma once

#include "../Graphics/Camera.hpp"

namespace mc
{
	class World;
	class Player : public le::InputListener
	{
	public:
		explicit Player(const le::Ref<le::Material>& blockSelectorMat,
			const le::Ref<le::MeshData>& blockSelectorMesh, World& world);
		~Player() override;

		le::Vector3f GetPosition() const;
		const Camera& GetCamera() const;
	private:
		void OnMouseClick(Tether::Input::MouseClickInfo& info) override;
		std::optional<le::Vector3f> FindLookAtPos() const;

		World& m_world;
		Camera m_Camera;

		le::Ref<le::Material> m_blockSelectorMat;
		le::Ref<le::MeshData> m_blockSelectorMesh;
		le::Entity m_blockSelector;

		std::optional<le::Vector3f> m_lookAtPos;

		le::EventBusSubscriber m_sub;
	};
}
