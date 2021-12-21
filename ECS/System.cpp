// Copyright 2013-2021 AFI, Inc. All Rights Reserved.

#include <pch.h>
#include "System.h"

namespace ECS {
    System::System(Hashes&& hashes) : _hashes(std::move(hashes)) {
    }

    void System::Run(Engine& engine, float delta) {
        for (const auto* instance : engine.CollectInstances(_hashes)) {
            for (const auto& collector : instance->GenerateCollector(_hashes)) {
                ForEach(engine, collector, delta);
            }
        }
    }
}
