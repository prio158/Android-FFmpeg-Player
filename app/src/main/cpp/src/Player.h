//
// Created by chen_zi_rui on 2024/4/13.
//

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include <memory>
#include <string.h>
#include <pthread.h>
#include "PlayerHelper.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

extern "C" {
#include "libavformat/avformat.h"
}

class Player {
    friend void *prepareCallback(void *args);

    friend void *startCallback(void *args);

public:
    using ptr = std::shared_ptr<Player>;

    Player(JavaVM *vm, JNIEnv *env, jobject *jobj);

    ~Player();

    void setNativeWindow(ANativeWindow *window);

    void setDataSource(const char *path_);

    /**
     * @brief 准备阶段会解析媒体文件或直播地址的媒体信息，耗时
     *
     * */
    void prepare();

    /**
     * @brief 读取媒体数据，根据数据类将音频、视频放入Audio/Video Channel的队列中
     *
     * */
    void start();

    void stop();

    void enable();

private:
    void _prepareTaskCallback();

    void _startTaskCallback();

private:
    /* 媒体文件路径或直播地址*/
    const char *m_path = nullptr;
    /* 在子线程中进行prepare，prepareTask是prepare子线程句柄*/
    pthread_t prepareTask {};
    /* 在子线程中进行start，prepareTask是start子线程句柄*/
    pthread_t startTask {};
    /* 媒体信息上下文 */
    AVFormatContext *avFormatContext = nullptr;
    /* 回调Helper*/
    PlayerHelper *playerHelper = nullptr;
    /* 播放时长，单位s*/
    int64_t duration = 0;
    /* 流数量 */
    uint8_t nb_streams = 0;
    /* 视频流解码 */
    VideoChannel *videoChannel = nullptr;
    /* 音频流解码 */
    AudioChannel *audioChannel = nullptr;
    /* 记录播放状态 */
    bool isPlaying = false;
    /* Window */
    ANativeWindow *window{};
};


#endif //PLAYER_PLAYER_H
