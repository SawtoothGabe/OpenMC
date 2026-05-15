#pragma once

#include <LE/LegendEngine.hpp>
#include "World/Block.hpp"

namespace mc
{
	class World;
	class Player : public le::InputListener
	{
	public:
		explicit Player(const le::Ref<le::Material>& blockSelectorMat,
			const le::Ref<le::MeshData>& blockSelectorMesh, World& world);
		~Player() override;

		[[nodiscard]] le::Vector3f GetPosition() const;
		[[nodiscard]] le::Entity GetCamera() const;
	private:
		void OnMouseClick(Tether::Input::MouseClickInfo& info) override;
		void OnUpdate(const le::UpdateEvent& e);
		void OnKey(le::KeyInfo& info) override;
		void OnRawMouseMove(le::RawMouseMoveInfo& info) override;

		void UpdateBlockSelector();

		void ProcessMovement(float delta);
		[[nodiscard]] le::Vector2f GetMoveDirection() const;

		std::optional<le::Vector3f> FindLookAtPos();

		World& m_world;

		le::Entity m_entity;
		le::Entity m_camera;
		le::Entity m_blockSelector;

		le::Ref<le::Material> m_blockSelectorMat;
		le::Ref<le::MeshData> m_blockSelectorMesh;

		std::optional<le::Vector3f> m_lookAtPos;
		Block::Face m_targetFace = Block::Face::NORTH;

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

		le::EventBusSubscriber m_sub;
	};
}
