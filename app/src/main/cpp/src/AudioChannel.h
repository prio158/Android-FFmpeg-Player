//
// Created by chen_zi_rui on 2024/4/21.
//

#ifndef PLAYER_AUDIOCHANNEL_H
#define PLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include "PlayerHelper.h"
#include "Mutex.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Platform.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}

class AudioChannel : public BaseChannel {

    friend void *play_audio(void *args);

    friend void audioPlayCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext);

public:
    explicit AudioChannel(int channelId, PlayerHelper *helper,
                          AVCodecContext *avCodecContext, const AVRational &base);

    ~AudioChannel();

public:
    void play() override;

    void stop() override;

    void decode() override;

    void enable() override;

    static double GetClock() {
        return audioClock;
    }


private:
    void _play();

    int _getData();

    void _initAudioResample();

private:
    bool isPlaying = false;
    Mutex mutex{};
    pthread_t audioDecodeTask{}, audioPlayTask{};
    SLEngineItf engineInterface = nullptr;
    SwrContext *swrContext = nullptr;
    uint8_t *buffer;
    int sample_rate;
    int out_channels;
    int per_samples_size;
    int sample_size;
    static double audioClock;
};


#endif //PLAYER_AUDIOCHANNEL_H
