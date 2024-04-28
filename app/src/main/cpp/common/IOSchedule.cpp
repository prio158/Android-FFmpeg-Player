//
// Created by 陈子锐 on 2024/4/27.
//

#include "IOSchedule.h"
#include <cassert>

IOSchedule::IOSchedule() {
    m_epfd = epoll_create(1);
    assert(m_epfd > 0);
    int ret = pipe(m_tickleFds);
    assert(ret == 0);
    epoll_event event{};
    memset(&event, 0, sizeof(event));
    event.events = EPOLLET | EPOLLIN;
    event.data.fd = m_tickleFds[0];
    ret = fcntl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    assert(ret >= 0);
    ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    assert(ret == 0);
}

void IOSchedule::loopEvent() {
    const uint64_t MAX_EVENTS = 256;
    auto *events = new epoll_event[MAX_EVENTS]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event *ptr) {
        delete[] ptr;
    });
    int ret;
    while (true) {
        auto timer = timers.back();
        timers.pop_back();
        auto task = timer->getTask();
        do {
            ret = epoll_wait(m_epfd, shared_events.get(), 64, (int) timer->getTimeout());
            if (ret < 0 && errno == EINTR) {} else break;
        } while (true);

        ///上面利用epoll_wait进行阻塞，当阻塞timeout结束后，执行延迟任务
        if (task)
            task();
        break;
    }
}

void IOSchedule::stopLoop() {
    isLooping = false;
}

IOSchedule::~IOSchedule() {
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);
}

void IOSchedule::addTimerTask(const Timer::ptr &timer) {
    timers.emplace_back(timer);
    loopEvent();
}




