#pragma once

#include "Event.h"
#include "..\Logger\Logger.h"

#include <map>
#include <typeindex>
#include <list>

class IEventCallback
{
private:
    virtual void Call(Event& e) = 0;
public:
    virtual ~IEventCallback() = default;
    void Execute(Event& e) {
        Call(e);
    }
};

template <typename TOwner, typename TEvent>
class EventCallback : public IEventCallback
{
private:
    typedef void (TOwner::* CallbackFunction)(TEvent&);

    TOwner* owner_instance;
    CallbackFunction callback_function;

    virtual void Call(Event& e) override {
        std::invoke(callback_function, owner_instance, static_cast<TEvent&>(e));
    }
public:
    EventCallback(TOwner* owner_instance, CallbackFunction callback_function) {
        this->owner_instance = owner_instance;
        this->callback_function = callback_function;
    }

    virtual ~EventCallback() override = default;
};

typedef std::list<std::unique_ptr<IEventCallback>> HandlerList;

class EventBus
{
private:
    std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;

public:
    EventBus() {
        Logger::Log("EventBus constructor called!");
    }

    ~EventBus() {
        Logger::Log("EventBus destructor called!");
    }

    // Clear the subscriber list.
    void Reset() {
        subscribers.clear();
    }

    // Subscribe to an event type <T>
    // A listener subscribes to an event.
    // Example: event_bus->SubscribeToEvent<CollisionEvent>(this, &Game::OnCollision);)
    template <typename TEvent, typename TOwner>
    void SubscribeToEvent(TOwner* owner_instance, void (TOwner::*callback_function)(TEvent&)) {
        if (!subscribers[typeid(TEvent)].get()) {
            subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
        }
        auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(owner_instance, callback_function);
        subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
    }

    // Emit an event of type <T>
    // As soon as something emits an event, we execute all listener callbacks.
    // Example: event_bus->EmitEvent<CollisionEvent>(player, enemy);
    template <typename TEvent, typename ...TArgs>
    void EmitEvent(TArgs&& ...args) {
        auto handlers = subscribers[typeid(TEvent)].get();
        if (handlers) {
            for (auto it = handlers->begin(); it != handlers->end(); it++) {
                auto handler = it->get();
                TEvent event(std::forward<TArgs>(args)...);
                handler->Execute(event);
            }
        }
    }
};
