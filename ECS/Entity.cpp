// Copyright 2013-2021 AFI, Inc. All Rights Reserved.

#include <pch.h>
#include "Entity.h"

namespace ECS {
    BodyRefs Entity::Get(const Hashes& hashes) const {
        return std::move(_handler.Get(_index, hashes));
    }

    void Entity::ChangeIndex(BodyIndex index) {
        _index = index;
    }

    Instance::Instance(TypeInfo&& typeInfo)
        : _typeInfo(std::move(typeInfo))
        , _packCount(static_cast<Size>(ChunkSizeToByte / _typeInfo.GetTotalSize())) {
        _bodyHandlers.emplace_back(new BodyHandler{ _packCount, _typeInfo.GetTypes() });
        _currentHandler = _bodyHandlers.front();
    }

    Instance::~Instance() {
        for (const auto* eachHandler : _bodyHandlers) {
            delete eachHandler;
        }
    }

    bool Instance::IsType(const HashSizePairsKeys& hashes) const {
        const auto isAnyFailed = std::ranges::any_of(hashes, [this](const auto hash)-> bool {
            return false == _typeInfo.IsHas(hash);
        });
        return isAnyFailed ? false : true;
    }

    bool Instance::IsType(const Hashes& hashes) const {
        const auto isAnyFailed = std::ranges::any_of(hashes, [this](const auto hash)-> bool {
            return false == _typeInfo.IsHas(hash);
        });
        return isAnyFailed ? false : true;
    }

    Collectors Instance::GenerateCollector(const Hashes& hashes) const {
        Collectors result;
        for (const auto* handler : _bodyHandlers) {
            result.emplace_back(handler, handler->GetAllocCount());

            for (auto& collector = result.back();
                const auto hash : hashes) {
                collector.refs.emplace_back(handler->Get(hash));
            }
        }
        return result;
    }

    const BodyHandler* Instance::FindHandler(const Hashes& hashes) {
        for (const auto hash : hashes) {
            if (false == _typeInfo.IsHas(hash)) {
                return nullptr;
            }
        }

        RefreshCurrentHandler();
        return _currentHandler;
    }

    void Instance::RemoveEmptyHandler() {
        const auto removeRanges = std::ranges::remove_if(_bodyHandlers, [this](const auto* eachHandler)->bool {
            return _currentHandler != eachHandler && eachHandler->IsEmpty();
        });
        for (const auto* eachHandler : removeRanges) {
            delete eachHandler;
        }
        _bodyHandlers.erase(removeRanges.begin(), removeRanges.end());
    }

    void Instance::RefreshCurrentHandler() {
        if (false == _currentHandler->IsFull()) {
            return;
        }

        for (const auto* eachHandler : _bodyHandlers) {
            if (false == eachHandler->IsFull()) {
                _currentHandler = eachHandler;
                return;
            }
        }

        _bodyHandlers.emplace_back(new BodyHandler{ _packCount, _typeInfo.GetTypes() });
        _currentHandler = _bodyHandlers.back();
    }

    Entity::Entity(const BodyHandler& handler) : _handler(handler), _index(handler.Allocate()) {
    }

    Entity::~Entity() {
        _handler.Free(_index);
    }

    Engine::~Engine() {
        for(const auto& entities : std::views::values(_entityPool)) {
            for(const auto* entity : entities) {
                if(nullptr == entity) {
                    break;
                }
                delete entity;
            }
        }
    }

    void Engine::RegistryTypeInformation(HashSizePairs&& types) {
        const auto hashes = std::views::keys(types);
        for (auto& instance : _instances) {
            if (instance.IsType(hashes)) {
                return;
            }
        }

        _instances.emplace_back(TypeInfo{ std::move(types) });
    }

    ConstInstanceRefs Engine::CollectInstances(const Hashes& hashes) const {
        ConstInstanceRefs result;
        for (const auto& instance : _instances) {
            if (instance.IsType(hashes)) {
                result.emplace_back(&instance);
            }
        }
        return result;
    }

    Entity* Engine::CreateEntity(const Hashes& hashes) {
        for (auto& instance : _instances) {
            if (const auto* handler = instance.FindHandler(hashes);
                nullptr != handler) {
                auto& entities = _entityPool[handler];
                if(entities.size() < handler->GetPackCount()) {
                    entities.resize(handler->GetPackCount());
                }

                ++_numEntities;
                auto* entity = new Entity{ *handler };
                entities[handler->GetAllocCount() - 1] = entity;
                return entity;
            }
        }

        return nullptr;
    }

    void Engine::DestroyEntity(gsl::not_null<Entity*>&& entity) {
        DestroyEntity(&entity->_handler, entity->_index);
    }

    void Engine::DestroyEntity(gsl::not_null<const BodyHandler*>&& handler, BodyIndex index) {
        --_numEntities;

        auto& entities = _entityPool[handler.get()];
        delete entities[index];
        entities[index] = nullptr;

        auto* lastEntity = entities[handler->GetAllocCount()];
        if(nullptr == lastEntity) {
            return;
        }

        lastEntity->ChangeIndex(index);
        entities[index] = lastEntity;
        entities[handler->GetAllocCount()] = nullptr;
    }

    const OwnerEntities& Engine::CollectEntities(const Collector& collector) {
        return _entityPool[collector.handler];
    }
}
