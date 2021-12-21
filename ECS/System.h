// Copyright 2013-2021 AFI, Inc. All Rights Reserved.

#pragma once

#include "Entity.h"

namespace ECS {
    class System {
    public:
        System(Hashes&& hashes);

        System(const System&)            = default;
        System(System&&)                 = default;
        virtual ~System()                = default;
        System& operator=(const System&) = default;
        System& operator=(System&&)      = default;

        virtual void Run(Engine& engine, float delta);

    protected:
        virtual void ForEach(Engine& /*engine*/, const Collector& /*collector*/, float /*delta*/) {};

        Hashes       _hashes;

        template<typename T1>
        __inline T1* Accept(const Collector& collector) const noexcept {
            return reinterpret_cast<T1*>(collector.refs[0]);
        }

        template<typename T1, typename T2, typename T3, typename T4>
        __inline std::tuple<T1*, T2*, T3*, T4*> Accept(const Collector& collector) const noexcept {
            return {
                reinterpret_cast<T1*>(collector.refs[0]),
                reinterpret_cast<T2*>(collector.refs[1]),
                reinterpret_cast<T3*>(collector.refs[2]),
                reinterpret_cast<T4*>(collector.refs[3]),
            };
        }
    };
}
