//
// Created by 陈子锐 on 2024/4/26.
//

#ifndef PLAYER_TIMER_H
#define PLAYER_TIMER_H


#include "Mutex.h"
#include "Tools.h"
#include <functional>
#include <set>
#include <vector>

class Timer : public std::enable_shared_from_this<Timer> {

public:
    using ptr = std::shared_ptr<Timer>;

    explicit Timer(uint64_t ms, std::function<void()> cb);

    explicit Timer(uint64_t ms);

    uint64_t getTimeout() const {
        return timeout;
    }

    std::function<void()> getTask() const {
        return m_cb;
    }

private:
    //延时时间
    uint64_t timeout = 0;
    //延时后要执行的任务
    std::function<void()> m_cb = nullptr;
};


#endif //PLAYER_TIMER_H
