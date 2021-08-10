// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

namespace Util {
    class Timer {
        using Clock                      = std::chrono::system_clock;
        using TimePoint                  = Clock::time_point;

    public:
        void                             Reset();
        void                             Update();

        [[nodiscard]] constexpr float    Total() const noexcept { return _total; }
        [[nodiscard]] constexpr float    Delta() const noexcept { return _delta; }
        [[nodiscard]] constexpr uint32_t Frame() const noexcept { return _framePerSeconds; }

    private:
        TimePoint                        _prev = Clock::now();

        float                            _total = 0.0f;
        float                            _delta = 0.0f;
        float                            _oneSeconds = 0.0f;
        uint32_t                         _frame = 0;
        uint32_t                         _framePerSeconds = 0;
    };

    namespace Random {
        uint32_t Get(uint32_t minValue, uint32_t maxValue);
    }
}
