// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>
#include "Scenario000.h"

#include "Scenario001.h"
#include "Scenario002.h"

namespace Scenario {
    template<typename T>
    std::unique_ptr<Scenario> Generate() {
        return std::make_unique<T>();
    }

    std::vector<uint32_t> GetIndices() {
        return { 1, 2 };
    }

    bool Run(uint32_t index) {
        switch(index) {
        case 1:
            Generate<ScenarioNoChunkECS>();
            break;
        case 2:
            Generate<ScenarioChunkECS>();
            break;
        default:
            return false;
        }
        return true;
    }
}
