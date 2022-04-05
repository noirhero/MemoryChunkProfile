// Copyright 2013-2022 AFI, Inc. All Rights Reserved.

#pragma once

#include <ECS/chunk.h>

namespace ECS {
    using namespace Chunk;

    struct Collector {
        const BodyHandler* handler = nullptr;
        const Size         count   = 0;
        BodyRefs           refs;
    };

    using Collectors        = std::vector<Collector>;
    using BodyHandlerOwner  = gsl::owner<BodyHandler*>;
    using BodyHandlerOwners = std::vector<BodyHandlerOwner>;

    //=================================================================================================================
    // Instance
    //=================================================================================================================
    class Instance {
    public:
        explicit Instance(TypeInfo&& typeInfo);
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) = default;
        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) = delete;

        [[nodiscard]] bool               IsType(const HashSizePairsKeys& hashes) const;
        [[nodiscard]] bool               IsType(const Hashes& hashes) const;

        [[nodiscard]] Collectors         GenerateCollector(const Hashes& hashes) const;

        [[nodiscard]] const BodyHandler* FindHandler(const Hashes& hashes);

        void                             RemoveEmptyHandler();

    private:
        void                             RefreshCurrentHandler();

        const TypeInfo                   _typeInfo;
        const Size                       _packCount = 0;
        const BodyHandler*               _currentHandler = nullptr;
        BodyHandlerOwners                _bodyHandlers;
    };

    //=================================================================================================================
    // Entity
    //=================================================================================================================
    using EntityPoolIndex = uint16_t;

    class Entity {
    private:
        friend class Engine;
        friend class EntityPool;

        explicit Entity(const BodyHandler& handler, EntityPoolIndex poolIndex);
        ~Entity();

    public:
        Entity(const Entity&)            = delete;
        Entity(Entity&&)                 = delete;
        Entity& operator=(const Entity&) = delete;
        Entity& operator=(Entity&&)      = delete;

        [[nodiscard]] BodyRef  Get(const Hash hash) const;
        [[nodiscard]] BodyRefs Get(const Hashes& hashes) const;

        [[nodiscard]] EntityPoolIndex GetPoolIndex() const noexcept {
            return _poolIndex;
        }

        [[nodiscard]] BodyIndex GetIndex() const noexcept {
            return _index;
        }

        template<typename T>
        [[nodiscard]] T* Accept(const Hash hash) const {
            return reinterpret_cast<T*>(Get(hash));
        }

    private:
        void                         ChangeIndex(BodyIndex index);

        const BodyHandler&           _handler;
        const EntityPoolIndex        _poolIndex;
        BodyIndex                    _index = InvalidBodyIndex;
    };

    //=================================================================================================================
    // EntityPool
    //=================================================================================================================
    using Entities = std::vector<Entity*>;

    class EntityPool {
        friend class Engine;

    public:
        explicit EntityPool(const BodyHandler& handler);

        [[nodiscard]] const Entities& GetEntities() const noexcept {
            return _entities;
        }

        Entity*                     Allocate();
        void                        Deallocate(gsl::not_null<Entity*> entity);
        void                        Deallocate(BodyIndex index);

    private:
        void                        ReservePool(EntityPoolIndex count);
        void                        Clear();

        const BodyHandler&          _handler;

        std::vector<char>           _buffer;
        std::deque<EntityPoolIndex> _reserveIndices;
        Entities                    _entities;
    };

    //=================================================================================================================
    // Engine
    //=================================================================================================================
    using Instances               = std::vector<Instance>;
    using ConstInstanceRefs       = std::vector<const Instance*>;
    using BodyHandlerAtEntityPool = std::unordered_map<const BodyHandler*, EntityPool>;

    class Engine {
    public:
        Engine()                         = default;
        ~Engine()                        = default;
        Engine(const Engine&)            = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&)      = delete;

        void                               RegistryTypeInformation(HashSizePairs&& types);
        [[nodiscard]] ConstInstanceRefs    CollectInstances(const Hashes& hashes) const;
        void                               ClearCollector(const Collector& collector) const;

        Entity*                            CreateEntity(const Hashes& hashes);
        void                               DestroyEntity(gsl::not_null<Entity*>&& entity);
        void                               DestroyEntity(gsl::not_null<const BodyHandler*>&& handler, BodyIndex index);

        [[nodiscard]] Entities             CollectEntities(const Collector& collector) const;
        [[nodiscard]] constexpr size_t     GetNumTotalEntity() const noexcept { return _numEntities; }

    private:
        Instances                          _instances;
        mutable BodyHandlerAtEntityPool    _entityPool;
        size_t                             _numEntities = 0;
    };
}
