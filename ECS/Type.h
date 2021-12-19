// Copyright 2013-2021 AFI, Inc. All Rights Reserved.

#pragma once

namespace ECS {
    using Hash      = uint64_t;
    using Size      = uint16_t;
    struct Type {
        Hash hash   = 0;
        Size size   = 0;
        Size offset = 0;
    };
    using Types     = std::vector<Type>;

    using Hashes              = std::vector<Hash>;
    using Sizes               = std::vector<Size>;
    using HashSizePair        = std::pair<Hash, Size>;
    using HashSizePairs       = std::vector<HashSizePair>;
    using HashSizePairsKeys   = std::ranges::keys_view<std::ranges::ref_view<HashSizePairs>>;
    using HashSizePairsValues = std::ranges::values_view<std::ranges::ref_view<HashSizePairs>>;
}
