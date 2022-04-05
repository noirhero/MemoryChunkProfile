// Copyright 2013-2022 AFI, Inc. All Rights Reserved.

#include <pch.h>
#include "entity.h"

namespace ECS {
    //=================================================================================================================
    // Instance
    //=================================================================================================================
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
        if(false == IsType(hashes)) {
            return nullptr;
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

    //=================================================================================================================
    // Entity
    //=================================================================================================================
    Entity::Entity(const BodyHandler& handler, EntityPoolIndex poolIndex) : _handler(handler), _poolIndex(poolIndex), _index(handler.Allocate()) {
    }

    Entity::~Entity() {
        _handler.Free(_index);
    }

    BodyRef Entity::Get(const Hash hash) const {
        return _handler.Get(_index, hash);
    }

    BodyRefs Entity::Get(const Hashes& hashes) const {
        return _handler.Get(_index, hashes);
    }

    void Entity::ChangeIndex(BodyIndex index) {
        _index = index;
    }

    //=================================================================================================================
    // EntityPool
    //=================================================================================================================
    EntityPool::EntityPool(const BodyHandler& handler) : _handler(handler) {
        ReservePool(static_cast<EntityPoolIndex>(handler.GetPackCount()));
    }

    Entity* EntityPool::Allocate() {
        assert(false == _reserveIndices.empty());

        const auto index = _reserveIndices.front();
        _reserveIndices.pop_front();

        auto* entity = new(&_buffer[index * sizeof(Entity)]) Entity(_handler, index);

        const auto lastIndex = 0 == _handler.GetAllocCount() ? 0 : _handler.GetAllocCount() - 1;
        _entities[lastIndex] = entity;
        return entity;
    }

    void EntityPool::Deallocate(gsl::not_null<Entity*> entity) {
        const auto index = entity->GetIndex();
        const auto poolIndex = entity->GetPoolIndex();
        _reserveIndices.emplace_back(poolIndex);

        entity->~Entity();
        _entities[index] = nullptr;

        auto* lastEntity = _entities[_handler.GetAllocCount()];
        if(nullptr == lastEntity) {
            return;
        }

        lastEntity->ChangeIndex(index);
        _entities[index] = lastEntity;
        _entities[_handler.GetAllocCount()] = nullptr;
    }

    void EntityPool::Deallocate(BodyIndex bodyIndex) {
        if(_entities.size() > bodyIndex && _entities[bodyIndex]) {
            Deallocate(_entities[bodyIndex]);
        }
    }

    void EntityPool::ReservePool(EntityPoolIndex count) {
        assert(_buffer.empty());
        _buffer.resize(count * sizeof(Entity));

        for (decltype(count) i = 0; i < count; ++i) {
            _reserveIndices.emplace_back(i);
        }

        _entities.resize(_handler.GetPackCount());
    }

    void EntityPool::Clear() {
        _handler.Clear();

        _reserveIndices.clear();
        const auto count = static_cast<EntityPoolIndex>(_buffer.size() / sizeof(Entity));
        for(std::remove_const_t<decltype(count)> i = 0; i < count; ++i) {
            _reserveIndices.emplace_back(i);
        }
        _entities.resize(_handler.GetPackCount(), nullptr);
    }

    //=================================================================================================================
    // Engine
    //=================================================================================================================
    void Engine::RegistryTypeInformation(HashSizePairs&& types) {
        const auto hashes = std::views::keys(types);
        for (auto& instance : _instances) {
            if (instance.IsType(hashes)) {
                return;
            }
        }

        _instances.emplace_back(TypeInfo{ types });
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

    void Engine::ClearCollector(const Collector& collector) const {
        const auto findIterator = _entityPool.find(collector.handler);
        if(_entityPool.end() == findIterator) {
            return;
        }

        findIterator->second.Clear();
    }

    Entity* Engine::CreateEntity(const Hashes& hashes) {
        if(hashes.empty()) {
            return nullptr;
        }

        for (auto& instance : _instances) {
            if (const auto* handler = instance.FindHandler(hashes);
                nullptr != handler) {
                ++_numEntities;

                if(const auto findIterator = _entityPool.find(handler);
                    _entityPool.end() == findIterator) {
                    if(const auto&[resultPair, isSuccess] = _entityPool.try_emplace(handler, *handler); 
                        isSuccess) {
                        return resultPair->second.Allocate();
                    }
                }
                else {
                    return findIterator->second.Allocate();
                }
                break;
            }
        }

        return nullptr;
    }

    void Engine::DestroyEntity(gsl::not_null<Entity*>&& entity) {
        DestroyEntity(&entity->_handler, entity->_index);
    }

    void Engine::DestroyEntity(gsl::not_null<const BodyHandler*>&& handler, BodyIndex index) {
        --_numEntities;

        const auto findIterator = _entityPool.find(handler.get());
        if (_entityPool.end() == findIterator) {
            return;
        }

        findIterator->second.Deallocate(index);
    }

    Entities Engine::CollectEntities(const Collector& collector) const {
        const auto findIterator = _entityPool.find(collector.handler);
        if (_entityPool.end() == findIterator) {
            return {};
        }

        return findIterator->second.GetEntities();
    }
}
