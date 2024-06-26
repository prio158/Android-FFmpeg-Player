//
// Created by Administrator on 2019/11/21.
//

#ifndef ENJOYPLAYER_VIDEOCHANNEL_H
#define ENJOYPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "PlayerHelper.h"
#include "Mutex.h"
#include "IOSchedule.h"
#include <threads.h>
#include <android/native_window_jni.h>
#include "AudioChannel.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/rational.h"
#include <sys/epoll.h>
#include "libavutil/time.h"
}

#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1

class VideoChannel : public BaseChannel {

    friend void *play_video(void *args);

public:
    explicit VideoChannel(int channelId, PlayerHelper *helper, AVCodecContext *avCodecContext,
                          const AVRational &base, int fps);

    void setWindow(ANativeWindow *window);

    ~VideoChannel();

public:
    /**
     * @brief 解码+播放
     * */
    virtual void play() override;

    virtual void stop() override;

    virtual void decode() override;

    virtual void enable() override;

private:

    void _release();

    void _play();

    void _render(uint8_t **data, int *linesize, int w, int h);

private:
    int fps = 0;
    pthread_t videoDecodeTask{}, videoPlayTask{};
    bool isPlaying = false;
    ANativeWindow *window = nullptr;
    Mutex window_mutex{};
    int width = 0;
    int height = 0;
    AVPixelFormat pixelFormat;
    AVPixelFormat outputPixelFormat = AV_PIX_FMT_RGBA;

};


#endif //ENJOYPLAYER_VIDEOCHANNEL_H
