// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>

#include "Scenario/Scenario000.h"

namespace {
    uint32_t ScenarioSelectProcess() {
        const auto indices = Scenario::GetIndices();

        while (true) {
            fmt::print("\nSelect scenario mode.\n1. No chunk.\n2. Chunk.\n:");

            std::string buffer;
            std::getline(std::cin, buffer);

            char* stop = nullptr;
            const auto selectIndex = static_cast<uint32_t>(strtol(buffer.c_str(), &stop, 10));
            if ('\0' != *stop) {
                continue;
            }

            if (std::ranges::any_of(indices, [selectIndex](const auto eachIndex)->bool {
                return eachIndex == selectIndex;
            })) {
                return selectIndex;
            }
        }
    }
}

int main() {
    Scenario::Run(ScenarioSelectProcess());
}
