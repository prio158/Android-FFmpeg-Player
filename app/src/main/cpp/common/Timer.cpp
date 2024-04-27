//
// Created by 陈子锐 on 2024/4/26.
//

#include "Timer.h"


Timer::Timer(uint64_t ms, std::function<void()> cb, TimerManager *manager, bool recurring)
        : m_recurring(recurring), m_ms(ms), m_cb(std::move(cb)), m_manager(manager) {

    /* 定期器的启动时间　＋　周期*/
    m_next = GetCurrentMSTime() + m_ms;
}

Timer::Timer(uint64_t ms) : m_ms(ms) {

}


bool Timer::cancel() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if (m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if (!m_cb) return false;
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end()) return false;
    m_manager->m_timers.erase(it);
    m_next = GetCurrentMSTime() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
    if (ms == m_ms && !from_now) return true;
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end()) return false;
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if (from_now) start = GetCurrentMSTime();
    else start = m_next - m_ms;
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this());
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
    ///插入后就立即检查一下:如果插入的定时器排在最前面,代表它的ms(执行周期)处于最小
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if (at_front) {
        m_tickled = true;
        /* 新插入timer的ms(执行周期)最小,通知IOSchedule重新设置epoll_wait的超时周期*/
        onTimerInsertedAtFront();
    }
}

void TimerManager::executeTimerTask(uint64_t now_ms) {
    for (const auto &timer: m_timers) {
        if (timer->m_cb != nullptr && isExecute(timer)) {
            timer->m_cb();
        }
    }
}

bool TimerManager::isExecute(const Timer::ptr &timer) {
    auto currentTime = GetCurrentMSTime();
    auto timerExecuteTime = timer->m_next;
    return timerExecuteTime >= currentTime;
}


bool TimerManager::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

TimerManager::TimerManager() {
    m_preTime = GetCurrentMSTime();
}

TimerManager::~TimerManager() {
}

uint64_t TimerManager::getNextTimer() {
    return 0;
}

