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
    while (isLooping) {
        uint64_t timeout = 0;
        if (stopping(timeout)) {
            LOGD("timeout invalid");
            break;
        }

        do {
            static const int MAX_TIME_OUT = 3000;
            if (timeout != ~0ull) {
                timeout = timeout > MAX_TIME_OUT ? MAX_TIME_OUT : timeout;
            } else timeout = MAX_TIME_OUT;
            ret = epoll_wait(m_epfd, shared_events.get(), 64, timeout);
            if (ret < 0 && errno == EINTR) {} else break;
        } while (true);

        ///上面利用epoll_wait进行睡眠，当睡眠timeout结束后，执行定时任务
        std::vector<std::function<void(void)>> cbs;

        for (const auto &cb: cbs) {
            cb();
        }
        cbs.clear();
        break;
    }
}

bool IOSchedule::stopping(uint64_t &timeout) {
    timeout = getTimeOut();
    return timeout == ~0ull;
}


void IOSchedule::onTimerInsertedAtFront() {
    LOGI("通知触发定时任务");
    isLooping = true;
    loopEvent();
}

IOSchedule::~IOSchedule() {
    isLooping = false;
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);
}



