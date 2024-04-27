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

    ptr GetThis() {
        return shared_from_this();
    }

    ///取消定时任务 Timer
    bool cancel();

    /// 更新该定时器的执行时间为now + m_ms
    bool refresh();

    /// 更新 Timer 的定时周期
    bool reset(uint64_t ms, bool from_now);

private:
    //是否循环定时器
    bool m_recurring = false;
    //执行周期　　　
    uint64_t m_ms = 0;
    //定期器的启动时间
    uint64_t m_next = 0;
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
            if (lhs->m_next < rhs->m_next) return true;
            if (lhs->m_next > lhs->m_next) return false;
            return lhs.get() < rhs.get();
        }
    };
};


class TimerManager {
    friend class Timer;

public:
    using ptr = std::shared_ptr<TimerManager>;

    using RWMutexType = RWMutex;

    explicit TimerManager();

    ~TimerManager();

public:
    bool hasTimer();

    ///添加定时器任务
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);


protected:
    /// notify
    virtual void onTimerInsertedAtFront() = 0;

    void addTimer(const Timer::ptr &timer);

    void executeTimerTask(uint64_t now_ms);

    uint64_t getTimeOut() const;

    static bool isExecute(const Timer::ptr& timer);

    uint64_t getNextTimer();

private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    bool m_tickled = false;
    /// 上次执行时间
    uint64_t m_preTime{};


};


#endif //PLAYER_TIMER_H
