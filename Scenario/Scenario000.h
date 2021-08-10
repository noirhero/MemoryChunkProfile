// Copyright 2011-2021 GameParaidos, Inc. All Rights Reserved.

#pragma once

namespace Scenario {
    class Scenario {
    public:
        Scenario() = default;
        virtual ~Scenario() = default;
    };

    std::vector<uint32_t> GetIndices();
    bool                  Run(uint32_t index);
}
