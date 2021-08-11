// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

#include "Entity.h"

namespace ECS {
    class System {
    public:
        System(Hashes&& hashes);
        virtual ~System() = default;

        System(const System&)            = default;
        System(System&&)                 = default;
        System& operator=(const System&) = default;
        System& operator=(System&&)      = default;

        virtual void Run(Engine& ecsEngine, float delta);

    protected:
        virtual void ForEach(const Collector& collector, float delta) = 0;

        Hashes       _hashes;

        template<typename T1, typename T2, typename T3, typename T4>
        static std::tuple<T1*, T2*, T3*, T4*> Accept(const Collector& collector) {
            return {
                reinterpret_cast<T1*>(collector.refs[0]),
                reinterpret_cast<T2*>(collector.refs[1]),
                reinterpret_cast<T3*>(collector.refs[2]),
                reinterpret_cast<T4*>(collector.refs[3]),
            };
        }
    };
}
