#pragma once
#include <LE/Components/Component.hpp>
#include <LE/Math/Types.hpp>
#include <LE/Physics/AABB.hpp>

namespace mc
{
    struct PhysicsBody : le::Component
    {
        le::Vector3f velocity;
        float drag = 0.98f;
        float gravity = -32.0f;
        le::AABB aabb;
        bool onGround = false;
    };
}
