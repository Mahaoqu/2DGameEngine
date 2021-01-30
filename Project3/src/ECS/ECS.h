#pragma once

#include <bitset>
#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "../Logger/Logger.h"
#include <deque>

#include <iostream>

const unsigned int MAX_COMPONENTS = 32;

// We use a bitset (1s and 0s) to keep track of which components an entity has.
// Also, this helps keep track of which entities a system is interested in.
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent
{
protected:
    static int next_id;
};

// Used to assign a unique id per component type.
template <typename T>
class Component : public IComponent
{
public:
    // Returns the unique id of Component<TComponent>
    static int GetId() {
        static auto id = next_id++;
        return id;
    }
};

class Entity
{
private:
    int id;

public:
    Entity(int id) : id(id) {};
    Entity(const Entity& entity) = default;
    void Kill();
    int GetId() const;

    // Manage entity tags and groups.
    void Tag(const std::string& tag);
    bool HasTag(const std::string& tag) const;
    void Group(const std::string& group);
    bool BelongsToGroup(const std::string& group) const;

    Entity& operator =(const Entity& other) = default;
    bool operator ==(const Entity& other) const { return id == other.id; };
    bool operator !=(const Entity& other) const { return id != other.id; };
    bool operator >(const Entity& other) const { return id > other.id; };
    bool operator <(const Entity& other) const { return id < other.id; };

    template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
    template <typename TComponent> void RemoveComponent();
    template <typename TComponent> bool HasComponent() const;
    template <typename TComponent> TComponent& GetComponent() const;

    // Hold a pointer to the entity's owner registry.
    class Registry* registry;
};

// The system processes entities that contain a specific signature.
class System
{
private:
    Signature component_signature;
    std::vector<Entity> entities;

public:
    System() = default;
    ~System() = default;

    void AddEntityToSystem(Entity entity);
    void RemoveEntityFromSystem(Entity entity);
    std::vector<Entity> GetSystemEntities() const;
    const Signature& GetComponentSignature() const;

    // Define the component type T that entities must have to be considered by the system.
    template <typename TComponent> void RequireComponent();
};

class IPool
{
public:
    virtual ~IPool() = default;
    virtual void RemoveEntityFromPool(int entity_id) = 0;
};

// A pool is just a vector (contiguous data) of objects of type T.
template <typename T>
class Pool : public IPool
{
private:

    // Keep track of the vector of objects and the current number of elements.
    std::vector<T> data;
    int size;
    
    // Helper maps to keep track of entity IDs per index, so the vector is always packed.
    std::unordered_map<int, int> entity_id_to_index;
    std::unordered_map<int, int> index_to_entity_id;

public:

    Pool(int capacity = 100) {
        size = 0;
        data.resize(capacity);
    }

    virtual ~Pool() = default;

    bool IsEmpty() const {
        return size == 0;
    }

    int GetSize() const {
        return size;
    }

    void Resize(int n) {
        data.resize(n);
    }

    void Clear() {
        data.clear();
        size = 0;
    }

    void Add(T object) {
        data.push_back(object);
    }

    void Set(int entity_id, T object) {
        if (entity_id_to_index.find(entity_id) != entity_id_to_index.end()) {
            // If the element already exists, replace the component object.
            int index = entity_id_to_index[entity_id];
            data[index] = object;
        } else {
            // When adding a new object, keep track of the entity IDs and their vector indicies.
            int index = size;
            entity_id_to_index.emplace(entity_id, index);
            index_to_entity_id.emplace(index, entity_id);
            if (index >= data.capacity()) {
                // If necessary, resize by doubling the current capacity.
                data.resize(size * 2);
            }
            data[index] = object;
            size++;
        }
    }

    void Remove(int entity_id) {
        // Copy the last element to the deleted position to keep the array packed.
        int index_of_removed = entity_id_to_index[entity_id];
        int index_of_last = size - 1;
        data[index_of_removed] = data[index_of_last];

        // Update the index-entity maps to point to the correct elements.
        int entity_id_of_last_element = index_to_entity_id[index_of_last];
        entity_id_to_index[entity_id_of_last_element] = index_of_removed;
        index_to_entity_id[index_of_removed] = entity_id_of_last_element;

        entity_id_to_index.erase(entity_id);
        index_to_entity_id.erase(index_of_last);

        size--;
    }

    void RemoveEntityFromPool(int entity_id) override {
        if (entity_id_to_index.find(entity_id) != entity_id_to_index.end()) {
            Remove(entity_id);
        }
    }

    T& Get(int entity_id) {
        int index = entity_id_to_index[entity_id];
        return static_cast<T&>(data[index]);
    }

    T& operator [](unsigned int index) {
        return data[index];
    }
};


// Registry
// The Registry manages the creation and destruction of entities.
// Adds systems and adds components to entities.
class Registry
{
private:
    // Keep track of how many entities were added to the scene.
    int num_entities = 0;

    // Vector of component pools.
    // Each pool contains all the data for a certain component type.
    // vector index = component_id
    // pool index = entity_id
    std::vector<std::shared_ptr<IPool>> component_pools;

    // Vector of component signatures.
    // The signature lets us know which components are turned "on" for an entity.
    // vector index = entity id
    std::vector<Signature> entity_component_signatures;

    // Map of active systems (index = system type_id).
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

    // Set of entities that are flagged to be added or removed in the next registry Update()
    std::set<Entity> entities_to_be_added;
    std::set<Entity> entities_to_be_killed;
    
    // Entity tags (one tag name per entity).
    std::unordered_map<std::string, Entity> entity_per_tag;
    std::unordered_map<int, std::string> tag_per_entity;

    // Entity groups (a set of entities per group name).
    std::unordered_map<std::string, std::set<Entity>> entities_per_group;
    std::unordered_map<int, std::string> group_per_entity;

    // List of free entity IDs that were previously removed.
    std::deque<int> free_ids;

public:
    Registry() {
        Logger::Log("Registry constructor called.");
    }
    ~Registry() {
        Logger::Log("Registry destructor called.");
    }
    void Update();

    // Entity management
    Entity CreateEntity();
    void KillEntity(Entity entity);

    // Tag management
    void TagEntity(Entity entity, const std::string& tag);
    bool EntityHasTag(Entity entity, const std::string& tag) const;
    Entity GetEntityByTag(const std::string& tag) const;
    void RemoveEntityTag(Entity entity);

    // Group management
    void GroupEntity(Entity entity, const std::string& group);
    bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
    std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
    void RemoveEntityGroup(Entity entity);

    // Components management
    template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
    template <typename TComponent> void RemoveComponent(Entity entity);
    template <typename TComponent> bool HasComponent(Entity entity) const;
    template <typename TComponent> TComponent& GetComponent(Entity entity) const;

    // System management
    template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
    template <typename TSystem> void RemoveSystem();
    template <typename TSystem> bool HasSystem() const;
    template <typename TSystem> TSystem& GetSystem() const;

    // Add and remove entities from their systems.
    void AddEntityToSystems(Entity entity);
    void RemoveEntityFromSystems(Entity entity);
};


template <typename TComponent>
void System::RequireComponent() {
    const auto component_id = Component<TComponent>::GetId();
    component_signature.set(component_id);
}


template<typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs && ...args) {
    const auto component_id = Component<TComponent>::GetId();
    const auto entity_id = entity.GetId();

    // If the component_id is greater than the current size of the component_pools, then resize the vector.
    if (component_id >= component_pools.size()) {
        component_pools.resize(component_id + 1, nullptr);
    }

    // If we do not have a Pool for a component type, create a new Pool of type TComponent.
    if (!component_pools[component_id]) {
        //Pool<TComponent>* new_component_pool = new Pool<TComponent>();
        std::shared_ptr<Pool<TComponent>> new_component_pool = std::make_shared<Pool<TComponent>>();
        component_pools[component_id] = new_component_pool;
    }

    // Get the pool of component values for that component type.
    //Pool<TComponent>* component_pool = Pool<TComponent>(component_pools[component_id]);
    std::shared_ptr<Pool<TComponent>> component_pool = std::static_pointer_cast<Pool<TComponent>>(component_pools[component_id]);

    // If the entity_id is greater than the current size of the component pool, then resize the pool.
    //if (entity_id >= component_pool->GetSize()) {
    //    component_pool->Resize(num_entities);
    //}

    // Create a new Component object of the type TComponent, and forwarrd the various parameters to the constructor.
    TComponent new_component(std::forward<TArgs>(args)...);

    // Add the new component to the component_pool list, using the entity_id as index.
    component_pool->Set(entity_id, new_component);

    // Finally, change the component signature of the entity and set the component_id on.
    entity_component_signatures[entity_id].set(component_id);

    //Logger::Log("component_id = " + std::to_string(component_id) + " was added to entity_id " + std::to_string(entity_id));

    //std::cout << "COMPONENT ID = " << component_id << " --> POOL SIZE: " << component_pool->GetSize() << std::endl;
}

template<typename TComponent>
void Registry::RemoveComponent(Entity entity) {
    const auto component_id = Component<TComponent>::GetId();
    const auto entity_id = entity.GetId();
    
    // Remove the component from the component list for that entity.
    std::shared_ptr<Pool<TComponent>> component_pool = std::static_pointer_cast<Pool<TComponent>>(component_pools[component_id]);
    component_pool->Remove(entity_id);

    // Set this component signature for that entity to false.
    entity_component_signatures[entity_id].set(component_id, false);

    //Logger::Log("component_id = " + std::to_string(component_id) + " was removed from entity_id " + std::to_string(entity_id));
}

template<typename TComponent>
inline bool Registry::HasComponent(Entity entity) const {
    // Return whether or not  there is a component for an entity.
    const auto component_id = Component<TComponent>::GetId();
    const auto entity_id = entity.GetId();
    return entity_component_signatures[entity_id].test(component_id);
}

template<typename TComponent>
inline TComponent& Registry::GetComponent(Entity entity) const {
    const auto component_id = Component<TComponent>::GetId();
    const auto entity_id = entity.GetId();
    auto component_pool = std::static_pointer_cast<Pool<TComponent>>(component_pools[component_id]);
    return component_pool->Get(entity_id);
}

template<typename TSystem, typename ...TArgs>
inline void Registry::AddSystem(TArgs && ...args) {
    // TSystem* new_system(new TSystem(std::forward<TArgs>(args)...));
    std::shared_ptr<TSystem> new_system = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    systems.insert(std::make_pair(std::type_index(typeid(TSystem)), new_system));
}

template<typename TSystem>
inline void Registry::RemoveSystem() {
    auto system = systems.find(std::type_index(typeid(TSystem)));
    systems.erase(system);
}

template<typename TSystem>
inline bool Registry::HasSystem() const {
    return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template<typename TSystem>
inline TSystem& Registry::GetSystem() const {
    auto system = systems.find(std::type_index(typeid(TSystem)));
    return *(std::static_pointer_cast<TSystem>(system->second));
}

template<typename TComponent, typename ...TArgs>
inline void Entity::AddComponent(TArgs && ...args) {
    registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template<typename TComponent>
inline void Entity::RemoveComponent() {
    registry->RemoveComponent<TComponent>(*this);
}

template<typename TComponent>
inline bool Entity::HasComponent() const {
    return registry->HasComponent<TComponent>(*this);
}

template<typename TComponent>
inline TComponent& Entity::GetComponent() const {
    return registry->GetComponent<TComponent>(*this);
}
