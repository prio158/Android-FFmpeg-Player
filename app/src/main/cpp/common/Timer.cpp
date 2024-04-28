//
// Created by 陈子锐 on 2024/4/26.
//

#include "Timer.h"


Timer::Timer(uint64_t ms, std::function<void()> cb)
        : timeout(ms), m_cb(std::move(cb)) {}

Timer::Timer(uint64_t ms) : timeout(ms) {}


