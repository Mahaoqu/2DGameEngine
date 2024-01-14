#include "ECS.h"
#include "../Logger/Logger.h"

int IComponent::next_id = 0;

int Entity::GetId() const { return id; }

void Entity::Tag(const std::string& tag) {
    registry->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const {
    return registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group) {
    registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const {
    return registry->EntityBelongsToGroup(*this, group);
}

void Entity::Kill() {
    registry->KillEntity(*this);
}

void System::AddEntityToSystem(Entity entity) {
    entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity) {
    entities.erase(std::remove_if(entities.begin(), entities.end(),
                                  [&entity](Entity other) { return entity == other; }), entities.end());
}

std::vector<Entity> System::GetSystemEntities() const {
    return entities;
}

const Signature& System::GetComponentSignature() const {
    return component_signature;
}

Entity Registry::CreateEntity() {
    int entity_id;

    if (free_ids.empty()) {
        // If there are no free IDs waiting to be reused, create new entity.
        entity_id = num_entities++;
        if (static_cast<size_t>(entity_id) >= entity_component_signatures.size()) {
            entity_component_signatures.resize(entity_id + 1);
        }
    } else {
        entity_id = free_ids.front();
        free_ids.pop_front();
    }

    Entity entity(entity_id);
    entity.registry = this;
    entities_to_be_added.insert(entity);

    //Logger::Log("Entity created with id = " + std::to_string(entity_id));

    return entity;
}

void Registry::KillEntity(Entity entity) {
    entities_to_be_killed.insert(entity);
}

/*
* Tag Management
*/
void Registry::TagEntity(Entity entity, const std::string& tag) {
    entity_per_tag.emplace(tag, entity);
    tag_per_entity.emplace(entity.GetId(), tag);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const {
    if (tag_per_entity.find(entity.GetId()) == tag_per_entity.end()) {
        return false;
    }
    return entity_per_tag.find(tag)->second == entity;
}

Entity Registry::GetEntityByTag(const std::string& tag) const {
    return entity_per_tag.at(tag);
}

void Registry::RemoveEntityTag(Entity entity) {
    auto tagged_entity = tag_per_entity.find(entity.GetId());
    if (tagged_entity != tag_per_entity.end()) {
        auto tag = tagged_entity->second;
        entity_per_tag.erase(tag);
        tag_per_entity.erase(tagged_entity);
    }
}

/*
* Group Management
*/
void Registry::GroupEntity(Entity entity, const std::string& group) {
    entities_per_group.emplace(group, std::set<Entity>());
    entities_per_group[group].emplace(entity);
    group_per_entity.emplace(entity.GetId(), group);
}

bool Registry::EntityBelongsToGroup(Entity entity, const std::string& group) const {
    if (entities_per_group.find(group) == entities_per_group.end()) {
        return false;
    }
    auto group_entities = entities_per_group.at(group);
    return group_entities.find(entity.GetId()) != group_entities.end();
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string& group) const {
    auto& set_of_entities = entities_per_group.at(group);
    return std::vector<Entity>(set_of_entities.begin(), set_of_entities.end());
}

void Registry::RemoveEntityGroup(Entity entity) {
    // if in group, remove entity from group management.
    auto grouped_entity = group_per_entity.find(entity.GetId());
    if (grouped_entity != group_per_entity.end()) {
        auto group = entities_per_group.find(grouped_entity->second);
        if (group != entities_per_group.end()) {
            auto entity_in_group = group->second.find(entity);
            if (entity_in_group != group->second.end()) {
                group->second.erase(entity_in_group);
            }
        }
        group_per_entity.erase(grouped_entity);
    }
}

void Registry::AddEntityToSystems(Entity entity) {
    const auto entity_id = entity.GetId();
    // Match entity_component_signature <--> system_component_signature.
    const auto& entity_component_signature = entity_component_signatures[entity_id];
    // Loop all the systems.
    for (auto& system : systems) {
        const auto& system_component_signature = system.second->GetComponentSignature();
        bool is_interested = (entity_component_signature & system_component_signature) == system_component_signature;
        if (is_interested) {
            system.second->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity) {
    for (auto system : systems) {
        system.second->RemoveEntityFromSystem(entity);
    }
}

void Registry::Update() {
    // Add the entities that are waiting to be created to the active Systems.
    for (auto& entity : entities_to_be_added) {
        AddEntityToSystems(entity);
    }
    entities_to_be_added.clear();

    // Remove the entities that are waiting to be killed from the active Systems.
    for (auto entity : entities_to_be_killed) {
        RemoveEntityFromSystems(entity);
        entity_component_signatures[entity.GetId()].reset();

        // Remove the entity from the component pools.
        for (auto pool : component_pools) {
            if (pool) {
                pool->RemoveEntityFromPool(entity.GetId());
            }
        }

        // Make the entity ID available to be reused.
        free_ids.push_back(entity.GetId());

        // Remove any traces of that entity from the tag/group maps.
        RemoveEntityTag(entity);
        RemoveEntityGroup(entity);
    }
    entities_to_be_killed.clear();
}