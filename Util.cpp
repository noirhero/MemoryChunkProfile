// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>
#include "Util.h"

namespace Util {
    void Timer::Reset() {
        _prev = Clock::now();
        _delta = _total = _oneSeconds = 0.0f;
        _frame = _framePerSeconds = 0;
    }

    void Timer::Update() {
        const auto now = Clock::now();
        _delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - _prev).count() * 0.001f;
        _prev = now;

        _total += _delta;
        _oneSeconds += _delta;
        if (1.0f <= _oneSeconds) {
            _oneSeconds -= 1.0f;
            _framePerSeconds = _frame + 1;
            _frame = 0;
        }
        else {
            ++_frame;
        }
    }

    namespace Random {
        std::random_device                                                               g_device;
        std::mt19937                                                                     g_generator(g_device());
        std::map<std::pair<uint32_t, uint32_t>, std::uniform_int_distribution<uint32_t>> g_distributionCache;

        uint32_t Get(uint32_t minValue, uint32_t maxValue) {
            const auto findIterator = std::ranges::find_if(g_distributionCache, [minValue, maxValue](const auto& eachPair)->bool {
                const auto& key = eachPair.first;
                return key.first == minValue && key.second == maxValue;
            });
            if (g_distributionCache.end() == findIterator) {
                const auto insertIterator = g_distributionCache.try_emplace(std::pair{ minValue, maxValue }, minValue, maxValue).first;
                return insertIterator->second(g_generator);
            }

            return findIterator->second(g_generator);
        }
    }
}
