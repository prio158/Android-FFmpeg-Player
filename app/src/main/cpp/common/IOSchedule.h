//
// Created by 陈子锐 on 2024/4/27.
//

#ifndef PLAYER_IOSCHEDULE_H
#define PLAYER_IOSCHEDULE_H

#include <sys/epoll.h>
#include <unistd.h>
#include <memory>
#include "Timer.h"

class IOSchedule {

public:
    using ptr = std::shared_ptr<IOSchedule>;

    explicit IOSchedule();

    ~IOSchedule();

public:
    void addTimerTask(const Timer::ptr &timer);

    void loopEvent();

private:
    std::vector<Timer::ptr> timers {};
    int m_epfd = 0;
};


#endif //PLAYER_IOSCHEDULE_H
