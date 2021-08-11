// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

#include "Chunk.h"

namespace ECS {
    using namespace Chunk;

    class Entity {
    public:
        explicit Entity(const BodyHandler& handler);
        ~Entity();

        Entity(const Entity&)            = delete;
        Entity(Entity&&)                 = delete;
        Entity& operator=(const Entity&) = delete;
        Entity& operator=(Entity&&)      = delete;

        [[nodiscard]] constexpr bool Is(const BodyHandler* handler) const noexcept {
            return &_handler == handler;
        }
        [[nodiscard]] constexpr bool Is(const BodyHandler* handler, BodyIndex index) const noexcept {
            return &_handler == handler && _index == index;
        }

        [[nodiscard]] BodyRefs       Get(const Hashes& hashes) const;

    private:
        const BodyHandler&           _handler;
        const BodyIndex              _index = InvalidBodyIndex;
    };

    struct Collector {
        const BodyHandler* handler = nullptr;
        const Size         count = 0;
        BodyRefs           refs;
    };

    using BodyHandlerOwner  = gsl::owner<BodyHandler*>;
    using BodyHandlerOwners = std::vector<BodyHandlerOwner>;
    using Collectors        = std::vector<Collector>;

    class Instance {
    public:
        explicit Instance(TypeInfo&& typeInfo);
        ~Instance();

        Instance(const Instance&)            = default;
        Instance(Instance&&)                 = default;
        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&)      = delete;

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

    using Instances         = std::vector<Instance>;
    using ConstInstanceRefs = std::vector<const Instance*>;
    using OwnerEntity       = gsl::owner<Entity*>;
    using OwnerEntities     = std::vector<OwnerEntity>;

    class Engine {
    public:
        Engine()                         = default;
        ~Engine()                        = default;
        Engine(const Engine&)            = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&)      = delete;

        void                            RegistryTypeInformation(HashSizePairs&& types);
        [[nodiscard]] ConstInstanceRefs CollectInstances(const Hashes& hashes) const;

        Entity*                         CreateEntity(const Hashes& hashes);
        void                            DestroyEntity(gsl::not_null<const Entity*>&& entity);
        void                            DestroyEntity(gsl::not_null<const BodyHandler*>&& handler, BodyIndex index);

        [[nodiscard]] constexpr size_t  GetNumTotalEntity() const noexcept { return _entities.size(); }

    private:
        Instances                       _instances;
        OwnerEntities                   _entities;
    };
}
