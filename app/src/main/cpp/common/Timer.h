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


class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {

    friend class TimerManager;

public:
    using ptr = std::shared_ptr<Timer>;

    explicit Timer(uint64_t ms, std::function<void()> cb, TimerManager *manager,
                   bool recurring = false);

    explicit Timer(uint64_t ms);

    bool cancel();

private:
    //是否循环定时器
    bool m_recurring = false;
    //执行周期　　　
    uint64_t m_ms = 0;
    //定时器要执行的任务
    std::function<void()> m_cb = nullptr;
    //TimerManager
    TimerManager *m_manager = nullptr;

private:
    struct Comparator {
        bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const {
            if (!lhs && !rhs) return false;
            if (!lhs) return true;
            if (!rhs) return false;
            return lhs.get() < rhs.get();
        }
    };
};


class TimerManager {
    friend class Timer;

public:
    using ptr = std::shared_ptr<TimerManager>;

    using RWMutexType = RWMutex;

    ~TimerManager() = default;

public:
    bool hasTimer();

    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);


protected:
    /// notify
    virtual void onTimerInsertedAtFront() = 0;

    void addTimer(const Timer::ptr &timer);

    void executeTimerTask();

    uint64_t getTimeOut() const;


private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
};


#endif //PLAYER_TIMER_H
