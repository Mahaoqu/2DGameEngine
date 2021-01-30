#pragma once

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "..\Events\CollisionEvent.h"

#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"


class CollisionSystem : public System
{
public:
    CollisionSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxColliderComponent>();
    }

    static bool intersect(TransformComponent& a_tc, BoxColliderComponent& a_bc, TransformComponent& b_tc, BoxColliderComponent& b_bc) {
        return (
            a_tc.position.x + a_bc.offset.x <= b_tc.position.x + b_bc.offset.x + b_bc.width &&
            a_tc.position.x + a_bc.offset.x + a_bc.width >= b_tc.position.x + b_bc.offset.x &&
            a_tc.position.y + a_bc.offset.y <= b_tc.position.y + b_bc.offset.y + b_bc.height &&
            a_tc.position.y + a_bc.offset.y + a_bc.height >= b_tc.position.y + b_bc.offset.y
            );
    }

    void Update(std::unique_ptr<EventBus>& event_bus) {
        auto entities = GetSystemEntities();
        for (auto i = entities.begin(); i != entities.end(); i++) {
            Entity a = *i;
            auto& a_tc = a.GetComponent<TransformComponent>();
            auto& a_bc = a.GetComponent<BoxColliderComponent>();
            for (auto j = i; j != entities.end(); j++) {
                Entity b = *j;
                if (a == b) {
                    continue;
                }
                auto& b_tc = b.GetComponent<TransformComponent>();
                auto& b_bc = b.GetComponent<BoxColliderComponent>();
                bool is_collision = intersect(a_tc, a_bc, b_tc, b_bc);
                if (is_collision) {
                    Logger::Log("entity " + std::to_string(a.GetId()) + " collided with entity " + std::to_string(b.GetId()));
                    event_bus->EmitEvent<CollisionEvent>(a, b);
                }
            }
        }
    }
};