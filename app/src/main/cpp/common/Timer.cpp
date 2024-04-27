//
// Created by 陈子锐 on 2024/4/26.
//

#include "Timer.h"


Timer::Timer(uint64_t ms, std::function<void()> cb, TimerManager *manager, bool recurring)
        : m_recurring(recurring), m_ms(ms), m_cb(std::move(cb)), m_manager(manager) {}

Timer::Timer(uint64_t ms) : m_ms(ms) {}

bool Timer::cancel() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if (m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it != m_manager->m_timers.end())
            m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}


Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
    Timer::ptr timer(new Timer(ms, std::move(cb), this, recurring));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer);
    ///虽然addTimer的作用是添加定时器，但是返回timer的目的是给调用方能够控制定时器
    return timer;
}

///　执行定时任务
static void onTimer(const std::weak_ptr<void> &weak_cond, const std::function<void()> &cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if (tmp) {
        cb();
    }
}


uint64_t TimerManager::getTimeOut() const {
    auto next_timer = *m_timers.begin();
    return next_timer->m_ms;
}


void TimerManager::addTimer(const Timer::ptr &timer) {
    auto it = m_timers.insert(timer).first;
    bool at_front = (it == m_timers.begin());
    if (at_front) {
        onTimerInsertedAtFront();
    }
}

void TimerManager::executeTimerTask() {
    for (const auto &timer: m_timers) {
        if (timer->m_cb != nullptr) {
            timer->m_cb();
        }
    }
    m_timers.clear();
}

bool TimerManager::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

