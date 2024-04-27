//
// Created by chen_zi_rui on 2024/4/14.
//

#ifndef PLAYER_SAFEQUEUE_H
#define PLAYER_SAFEQUEUE_H

#include <queue>
#include "Mutex.h"
#include <optional>

using namespace std;

template<class T>
class SafeQueue {
public:
    void push(const T &v) {
        Spinlock::Lock lock(spinlock);
        queue.push(v);
    }

    std::optional<T> pop() {
        Spinlock::Lock lock(spinlock);
        if (queue.empty()) {
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> res(std::move(queue.front()));
            return res;
        }
    }


private:
    std::queue<T> queue{};
    /**自旋锁性能更佳*/
    Spinlock spinlock;


};


#endif //PLAYER_SAFEQUEUE_H
