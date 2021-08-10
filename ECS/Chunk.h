// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

#include "Type.h"

namespace Chunk {
    using namespace ECS;

    constexpr uint16_t ChunkSizeToByte = 16384; // 16KB
    struct Body {
        uint8_t        memory[ChunkSizeToByte]{};
    };

    class TypeInfo {
    public:
        explicit TypeInfo(HashSizePairs&& types);

        [[nodiscard]] bool         IsHas(Hash hash) const noexcept;
        [[nodiscard]] const Types& GetTypes() const noexcept { return _types; }
        [[nodiscard]] Size         GetTotalSize() const noexcept { return _totalSize; }

    private:
        Size                       _totalSize = 0;
        Types                      _types;
    };

    using     HashBySizeOffsetMap        = std::map<Hash, std::pair<Size, Size>>;
    using     BodyRef                    = uint8_t*;
    using     BodyRefs                   = std::vector<BodyRef>;
    using     BodyIndex                  = Size;
    constexpr BodyIndex InvalidBodyIndex = -1;

    class BodyHandler {
    public:
        explicit BodyHandler(Size packCount, const Types& types);

        [[nodiscard]] constexpr bool IsFull() const noexcept { return _packCount == _allocCount; }
        [[nodiscard]] constexpr bool IsEmpty() const noexcept { return 0 == _allocCount; }
        [[nodiscard]] constexpr Size GetAllocCount() const noexcept { return _allocCount; }

        BodyIndex                    Allocate() const;
        void                         Free(BodyIndex index) const;

        BodyRefs                     Get(BodyIndex index) const;
        BodyRef                      Get(Hash findHash) const;

    private:
        const Size                  _packCount = 0;
        mutable HashBySizeOffsetMap _types;

        mutable Body                _body;
        mutable Size                _allocCount = 0;
    };
}
