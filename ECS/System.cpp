// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>
#include "System.h"

namespace ECS {
    System::System(Hashes&& hashes) : _hashes(std::move(hashes)) {
    }

    void System::Run(Engine& ecsEngine, float delta) {
        for (const auto* instance : ecsEngine.CollectInstances(_hashes)) {
            for (const auto& collector : instance->GenerateCollector(_hashes)) {
                ForEach(collector, delta);
            }
        }
    }
}
