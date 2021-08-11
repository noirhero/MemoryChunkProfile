// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>
#include "Entity.h"

namespace ECS {
    Entity::Entity(const BodyHandler& handler) : _handler(handler), _index(handler.Allocate()) {
    }

    Entity::~Entity() {
        _handler.Free(_index);
    }

    BodyRefs Entity::Get() const {
        return std::move(_handler.Get(_index));
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
            result.emplace_back(handler->GetAllocCount());

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
                _entities.emplace_back(new Entity{ *handler });
                return _entities.back();
            }
        }

        return nullptr;
    }

    void Engine::DestroyEntity(gsl::not_null<Entity*>&& entity) {
        const auto findIterator = std::ranges::find(_entities, entity.get());
        if (_entities.empty()) {
            return;
        }

        delete entity.get();
        _entities.erase(findIterator);
    }
}
