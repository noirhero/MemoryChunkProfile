// Copyright 2013-2021 AFI, Inc. All Rights Reserved.

#include <pch.h>
#include "Chunk.h"

namespace Chunk {
    TypeInfo::TypeInfo(HashSizePairs&& types) {
        for (const auto& [hash, size] : types) {
            _types.emplace_back(hash, size, _totalSize);
            _totalSize += size;
        }
    }

    bool TypeInfo::IsHas(Hash hash) const noexcept {
        return std::ranges::any_of(_types, [hash](const auto& eachType)->bool {
            return eachType.hash == hash;
        });
    }

    BodyHandler::BodyHandler(Size packCount, const Types& types) : _packCount(packCount) {
        for (const auto& [hash, size, offset] : types) {
            _types.try_emplace(hash, std::pair{ size, offset });
        }
    }

    BodyIndex BodyHandler::Allocate() const {
        if (IsFull()) {
            return InvalidBodyIndex;
        }

        return _allocCount++;
    }

    void BodyHandler::Free(BodyIndex index) const {
        if (index >= _allocCount || InvalidBodyIndex == index) {
            return;
        }

        --_allocCount;
        if (index == _allocCount || IsEmpty()) {
            return;
        }

        for (const auto& [size, offset] : std::views::values(_types)) {
            const auto start = offset * _packCount;
            const auto& src = _body.memory[start + size * _allocCount];
            auto& dest = _body.memory[start + size * index];
            memcpy_s(&dest, size, &src, size);
        }
    }

    BodyRefs BodyHandler::Get(BodyIndex index, const Hashes& hashes) const {
        BodyRefs result;
        for (const auto hash : hashes) {
            const auto& [size, offset] = _types[hash];
            result.emplace_back(&_body.memory[offset * _packCount + size * index]);
        }
        return result;
    }

    BodyRef BodyHandler::Get(Hash hash) const {
        const auto& [size, offset] = _types[hash];
        return &_body.memory[offset * _packCount];
    }
}
