//
// Created by Administrator on 2019/11/14.
//

#ifndef PLAYER_BASECHANNEL_H
#define PLAYER_BASECHANNEL_H

extern "C" {
#include "libavcodec/avcodec.h"
}


#include "PlayerHelper.h"
#include "SafeQueueMutex.h"

class BaseChannel {
public:
    BaseChannel(int channelId, PlayerHelper *helper, AVCodecContext *avCodecContext, AVRational
    base) : channelId(channelId),
            helper(helper),
            avCodecContext(avCodecContext),
            time_base(base) {
        pkt_queue.setReleaseHandle(releaseAvPacket);
        frame_queue.setReleaseHandle(releaseAvFrame);
    }

    virtual ~BaseChannel() {
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        pkt_queue.clear();
        frame_queue.clear();
    };

    virtual void play() = 0;

    virtual void stop() = 0;

    virtual void decode() = 0;

    virtual void enable() = 0;

    void setEnable(bool enable) {
        pkt_queue.setEnable(enable);
        frame_queue.setEnable(enable);
    }

    static void releaseAvFrame(AVFrame *&frame) {
        av_frame_free(&frame);
    }

    static void releaseAvPacket(AVPacket *&packet) {
        av_packet_free(&packet);
    }

    bool hasPacketData() {
        return !pkt_queue.empty();
    }

    bool hasFrameData() {
        return !frame_queue.empty();
    }

public:
    int channelId;
    PlayerHelper *helper;
    AVCodecContext *avCodecContext;
    AVRational time_base;
    SafeQueue<AVPacket *> pkt_queue;
    /*解码后数据，AVFrame图像数据是YUV的格式*/
    SafeQueue<AVFrame *> frame_queue;
    bool isPlaying = false;
    double clock = 0;

};

#endif //PLAYER_BASECHANNEL_H
