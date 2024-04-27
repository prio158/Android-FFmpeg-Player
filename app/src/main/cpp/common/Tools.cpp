//
// Created by 陈子锐 on 2024/4/26.
//
#include "Tools.h"

uint64_t GetCurrentMSTime() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}


