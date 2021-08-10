// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

#include "Entity.h"

namespace ECS {
    class System {
    public:
        explicit System(Hashes&& hashes);
        virtual ~System() = default;

        System(const System&)            = default;
        System(System&&)                 = default;
        System& operator=(const System&) = default;
        System& operator=(System&&)      = default;

        void         Run(const Engine& ecsEngine);

    protected:
        template<typename T1>
        static Size  Accept(T1*& t1, const Collector& component) {
            t1 = reinterpret_cast<T1*>(component.bodyRefArray[0]);
            return component.count;
        }

        template<typename T1, typename T2>
        static Size  Accept(T1*& t1, T2*& t2, const Collector& component) {
            t1 = reinterpret_cast<T1*>(component.bodyRefArray[0]);
            t2 = reinterpret_cast<T2*>(component.bodyRefArray[1]);
            return component.count;
        }

        template<typename T1, typename T2, typename T3>
        static Size  Accept(T1*& t1, T2*& t2, T3*& t3, const Collector& component) {
            t1 = reinterpret_cast<T1*>(component.bodyRefArray[0]);
            t2 = reinterpret_cast<T2*>(component.bodyRefArray[1]);
            t3 = reinterpret_cast<T3*>(component.bodyRefArray[2]);
            return component.count;
        }

        template<typename T1, typename T2, typename T3, typename T4>
        static Size  Accept(T1*& t1, T2*& t2, T3*& t3, T4*& t4, const Collector& component) {
            t1 = reinterpret_cast<T1*>(component.bodyRefArray[0]);
            t2 = reinterpret_cast<T2*>(component.bodyRefArray[1]);
            t3 = reinterpret_cast<T3*>(component.bodyRefArray[2]);
            t4 = reinterpret_cast<T4*>(component.bodyRefArray[3]);
            return component.count;
        }

        virtual void ForEach(const Collector& components) = 0;

        Hashes       _hashes;
    };
}
