
#ifndef PLAYER_SAFE_QUEUE_H
#define PLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>
#include "Log.h"
#include "IOSchedule.h"

#define MAX_QUEUE_SIZE (5 * 1024 * 1024)

template<typename T>
class SafeQueue {
    typedef void (*ReleaseHandle)(T &);

    typedef void (*SyncHandle)(std::queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&cond, 0);
        ioSchedule = IOSchedule::ptr(new IOSchedule);
    }

    ~SafeQueue() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }


    void enQueue(T new_value, bool video = false) {
        if (checkQueueHasEnoughData()) {
            Mutex::Lock lock(size_mutex);
            LOGI("TAG 等待 5s 前的，pkt_queue size:%d", size());
            ioSchedule->addTimer(5000, []() {
                LOGD("TAG 定时任务");
            }, false);
            LOGI("TAG 等待 5s 后的，pkt_queue size:%d", size());
            LOGI("TAG------------------------------------------------");
            lock.unlock();
        }
        pthread_mutex_lock(&mutex);
        if (mEnable) {
            if (video) {
                videoQueSize += sizeof(*new_value);
            } else {
                audioQueSize += sizeof(*new_value);
            }
            q.push(new_value);
            pthread_cond_signal(&cond);
        } else {
            releaseHandle(new_value);
        }
        pthread_mutex_unlock(&mutex);
    }


    int deQueue(T &value, bool video = false) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while (mEnable && q.empty()) {
            LOGD("没有数据开始休眠");
            /* 如果q一直没有数据，那么这里就会一直阻塞，总不可能一直阻塞吧，调用setEnable进行强制唤醒，解除阻塞 */
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
            if (video) {
                videoQueSize -= sizeof(*value);
            } else {
                audioQueSize -= sizeof(*value);
            }
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    void setEnable(bool enable) {
        pthread_mutex_lock(&mutex);
        this->mEnable = enable;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            releaseHandle(value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void sync() {
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseHandle(ReleaseHandle r) {
        releaseHandle = r;
    }

    void setSyncHandle(SyncHandle s) {
        syncHandle = s;
    }

    bool checkQueueHasEnoughData() {
        return q.size() >= 2000;
    }

private:
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    std::queue<T> q;
    size_t videoQueSize = 0;
    size_t audioQueSize = 0;
    bool mEnable;
    ReleaseHandle releaseHandle;
    SyncHandle syncHandle;
    IOSchedule::ptr ioSchedule;
    Mutex size_mutex;

};


#endif //PLAYER_SAFE_QUEUE_H
