
#ifndef PLAYER_SAFE_QUEUE_H
#define PLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>
#include "Log.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)

template<typename T>
class SafeQueue {
    typedef void (*ReleaseHandle)(T &);

    typedef void (*SyncHandle)(std::queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }


    void enQueue(T new_value, bool video = false) {
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
        LOGD("all size:%d", videoQueSize + audioQueSize);
        return q.size() >=1000;
        //return (videoQueSize + audioQueSize) > MAX_QUEUE_SIZE;
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

};


#endif //PLAYER_SAFE_QUEUE_H
