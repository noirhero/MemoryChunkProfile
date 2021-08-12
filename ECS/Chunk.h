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

        [[nodiscard]] bool                   IsHas(Hash hash) const noexcept;
        [[nodiscard]] constexpr const Types& GetTypes() const noexcept { return _types; }
        [[nodiscard]] constexpr Size         GetTotalSize() const noexcept { return _totalSize; }

    private:
        Size                                 _totalSize = 0;
        Types                                _types;
    };

    using     HashBySizeOffsetMap = std::map<Hash, std::pair<Size, Size>>;
    using     BodyRef = uint8_t*;
    using     BodyRefs = std::vector<BodyRef>;
    using     BodyIndex = Size;
    constexpr BodyIndex InvalidBodyIndex = -1;

    class BodyHandler {
    public:
        explicit BodyHandler(Size packCount, const Types& types);

        [[nodiscard]] constexpr bool IsFull() const noexcept { return _packCount == _allocCount; }
        [[nodiscard]] constexpr bool IsEmpty() const noexcept { return 0 == _allocCount; }
        [[nodiscard]] constexpr Size GetAllocCount() const noexcept { return _allocCount; }
        [[nodiscard]] constexpr Size GetPackCount() const noexcept { return _packCount; }

        BodyIndex                    Allocate() const;
        void                         Free(BodyIndex index) const;

        BodyRefs                     Get(BodyIndex index, const Hashes& hashes) const;
        BodyRef                      Get(Hash hash) const;

    private:
        const Size                  _packCount = 0;
        mutable HashBySizeOffsetMap _types;

        mutable Body                _body;
        mutable Size                _allocCount = 0;
    };

    template<typename T1>
    T1* Accept(const BodyRefs& refs) {
        return reinterpret_cast<T1*>(refs[0]);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    std::tuple<T1*, T2*, T3*, T4*, T5*> Accept(const BodyRefs& refs) {
        return {
            reinterpret_cast<T1*>(refs[0]),
            reinterpret_cast<T2*>(refs[1]),
            reinterpret_cast<T3*>(refs[2]),
            reinterpret_cast<T4*>(refs[3]),
            reinterpret_cast<T5*>(refs[4]),
        };
    }
}
