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
		void OnKey(le::Input::KeyInfo& info) override;
		void FindLookAtPos();
		void StepBlockSearch();

		World& m_world;
		Camera m_Camera;

		le::Ref<le::Material> m_blockSelectorMat;
		le::Ref<le::MeshData> m_blockSelectorMesh;
		le::Entity m_blockSelector;
		le::Entity m_raycast;

		le::Ref<le::Material> m_raycastMat;
		le::Ref<le::MeshData> m_raycastMesh;

		std::optional<le::Vector3f> m_lookAtPos;

		le::EventBusSubscriber m_sub;

		le::Vector3f pos;
		le::Vector3f end;
		le::Vector3f forward;
		le::Vector3f delta;
		le::Vector3f currentPos;
		le::Vector3f roundedPos;
		float step = 0;
		int i = 0;
	};
}
