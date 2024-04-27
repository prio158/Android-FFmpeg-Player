//
// Created by 陈子锐 on 2024/4/27.
//

#ifndef PLAYER_IOSCHEDULE_H
#define PLAYER_IOSCHEDULE_H

#include "Timer.h"
#include <sys/epoll.h>
#include <unistd.h>

class IOSchedule : public TimerManager {

public:
    using ptr = std::shared_ptr<IOSchedule>;

    explicit IOSchedule();

    ~IOSchedule();

    void onTimerInsertedAtFront() override;

private:
    void loopEvent();

    bool stopping(uint64_t &timeout);

private:
    int m_epfd = 0;
    int m_tickleFds[2]{};
    bool isLooping = false;
};


#endif //PLAYER_IOSCHEDULE_H
