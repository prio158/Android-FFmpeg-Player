//
// Created by chen_zi_rui on 2024/4/15.
//
#include "VideoChannel.h"
#include "Log.h"


void *decode_video(void *args) {
    auto videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();
    return nullptr;
}

void *play_video(void *args) {
    auto videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->_play();
    return nullptr;
}

VideoChannel::VideoChannel(int channelId, PlayerHelper *helper, AVCodecContext *avCodecContext,
                           const AVRational &base, int fps) : BaseChannel(channelId, helper,
                                                                          avCodecContext, base),
                                                              fps(fps) {
    width = avCodecContext->width;
    height = avCodecContext->height;
    pixelFormat = avCodecContext->pix_fmt;
}


void VideoChannel::_play() {

    LOGD("VideoChannel::play");

    AVFrame *av_frame = nullptr;

    int ret;

    uint8_t *outputData[4];

    int lineSize[4];

    /*提前申请转换后数据的内存*/
    av_image_alloc(outputData, lineSize, width, height, outputPixelFormat, 1);

    auto sws_context = sws_getContext(width, height, pixelFormat,
                                      width, height, outputPixelFormat,
                                      SWS_FAST_BILINEAR,
                                      nullptr, nullptr, nullptr);

    double frame_rate = 1.0 / fps;

    while (isPlaying) {

        /** 阻塞方法，当队列中没有数据的时候，会阻塞在这里await，只有在队列有数据的时候，会解除阻塞*/
        ret = frame_queue.deQueue(av_frame, true, false);

        /** 停止播放时候，直接退出 */
        if (!isPlaying)
            break;

        /**ret==0代表没有数据*/
        if (!ret)
            continue;

        ///控制视频播放速度跟随 FPS
        double extra_delay = av_frame->repeat_pict / (2 * fps);
        double delay = extra_delay + frame_rate;
        auto audio_clock = AudioChannel::GetClock();
        auto video_clock = av_frame->best_effort_timestamp * av_q2d(time_base);
        double diff = video_clock - audio_clock;
        /**
         * 根据每秒视频播放的帧数（fps），确定一个延迟范围
         * delay < 0.04 ===> 0.04
         * delay > 0.1 ===> 0.1
         * 0.04 < delay < 0.1 ===> delay
         * */
        double sync = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (diff <= -sync) {
            /// diff<0 代表视频落后，diff <= -0.05代表落后比较多，就需要进行同步
            /// 通过减少 delay ，让视频追赶上音频
            /// diff是负数，delay + diff 就是减少 delay 时间，但是最小为 0
            delay = FFMAX(0, delay + diff);
        } else if (diff > sync) {
            /// diff > sync 代表视频快了很多，就需要 delay 久一点，等待音频
            delay = delay + diff;
        }

        av_usleep(delay * 1000000);

        LOGI("SYNC Video Clock=%lf,Audio Clock=%lf,A-V=%lf", audio_clock, video_clock, -diff);

        /**
         * 因为ANativeWindow不支持显示YUV格式的数据，所以需要将FFmpeg解码出来YUV数据进行转换
         * av_frame->data: 是一个指针数组：uint8_t *data[AV_NUM_DATA_POINTERS], 数组里面存的是uint8_t*
         *
         * linesize:表示图像每一行的字节数，而不是图像的大小
         * 对于平面数据（例如 YUV420），linesize[i] 包含第 i 个平面的跨度（stride）。例如，对于一个 640x480 的图像，
         * data[0] 包含指向 Y 分量的指针，data[1] 和 data[2] 分别包含指向 U 和 V 分量的指针。在这种情况下，linesize[0]
         * 等于 640，linesize[1] 和 linesize[2] 都等于 320（因为 U 和 V 分量的宽度是 Y 分量的一半）。
         *
         * */
        sws_scale(sws_context, av_frame->data, av_frame->linesize,
                  0, av_frame->height, outputData, lineSize);
        //注意经过sws_scale转换后，av_frame->data转换后的RGBA数据都保存到outputData[0]中。
        //sws_scale输出的rgba数据的lineSize：每一行像素数据所占用的字节数。（这个值是经过FFmpeg内部的字节对齐逻辑而来的）
        //比如: HAVE_SIMD_ALIGN_64（64字节对齐）,但默认FFmpeg是STRIDE_ALIGN, 进行8字节对齐。所以lineSize是经过8字节对齐后的值，
        //代表转换后的RGBA数据每一行像素数据所占用的字节数。
        _render(outputData, lineSize, av_frame->width, av_frame->height);
        releaseAvFrame(av_frame);
    }
    /**
     * 处理的一点经验：在栈中开辟了堆内存，不会它在栈里面怎么玩花活
     * 出栈时必须要释放掉。
     * */
    av_freep(&outputData[0]);
    releaseAvFrame(av_frame);
    sws_freeContext(sws_context);
    isPlaying = false;
}


void VideoChannel::play() {
    isPlaying = true;
    /* 开启队列工作 */
    setEnable(true);
    /* 解码 */
    pthread_create(&videoDecodeTask, nullptr, decode_video, this);
    /* 播放 */
    pthread_create(&videoPlayTask, nullptr, play_video, this);
}

void VideoChannel::stop() {
    isPlaying = false;
    setEnable(false);
    pthread_join(videoPlayTask, nullptr);
    pthread_join(videoDecodeTask, nullptr);
    _release();
}

void VideoChannel::enable() {
    isPlaying = true;
    setEnable(true);
}

void VideoChannel::decode() {
    LOGD("VideoChannel::decode");
    AVPacket *packet = nullptr;
    while (isPlaying) {
        /* deQueue 是一个阻塞方法，当pkt_queue空的时候，会阻塞，直到pkt_queue存入新的数据。*/
        int ret = pkt_queue.deQueue(packet, true, true);
        /* 停止播放时候，直接退出 */
        if (!isPlaying) {
            break;
        }
        /* ret = 0 时，代表没有取到数据（不会阻塞-队列为空）*/
        if (!ret) {
            continue;
        }

        /* 向解码器发送编码数据 */
        /* avcodec_send_packet 内部也有一个循环，不断地解码，然后把数据放入一个队列中
         * 当返回ret = AVERROR(EAGAIN),代表解码器内部队列已经满了，需要读取,但这里没有溢出
         * 的可能了，因为下面我马上avcodec_receive_frame，不需要等到满了，再读取。
         * */
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret < 0) {
            LOGE("avcodec_send_video_packet error:%s", av_err2str(ret));
            break;
        }

        /* 从解码器中的队列取出解码好的数据 */
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        /* ret = AVERROR(EAGAIN) 代表解码器中的解码队列已经取空了，没有更多的数据了 */
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            LOGE("avcodec_receive_video_frame error:%s", av_err2str(ret));
            break;
        }
        frame_queue.enQueue(frame, true);
    }
    releaseAvPacket(packet);
}

void VideoChannel::setWindow(ANativeWindow *window_) {
    Mutex::Lock lock(window_mutex);
    if (nullptr != window)
        ANativeWindow_release(window);
    window = window_;
}

void VideoChannel::_render(uint8_t **data, int *linesize, int w, int h) {
    Mutex::Lock lock(window_mutex);
    if (nullptr == window) {
        lock.unlock();
        return;
    }
    /*设置缓冲区格式和大小,缓冲区里面是像素数据,只影响缓冲区的大小和格式，不会改变窗口在屏幕上的实际尺寸*/
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    /*这个buffer里面存放着要显示在NativeWindow上的数据*/
    ANativeWindow_Buffer buffer;
    /*加锁*/
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        //data 中存放的就是RGBA格式的数据，然后向buffer中填充，但是在数据填充时，需要根据
        //window_buffer.stride来一行行拷贝,stride是指图像缓冲区中每一行像素数据所占用的字节数。
        //它不仅仅等于图像的实际宽度，而可能会更大，因为需要字节对齐。比如852x480的图像，852个像素点，占852*4个字节=3408
        //但是在内存中存储时，为了内存对齐（字节对齐），实际分配不止3408个字节。

        //bits 是一个指向图像缓冲区像素数据的指针,可以通过访问 bits 来读取或修改像素数据
        auto *dstData = static_cast<uint8_t *>(buffer.bits);
        //buffer.stride代表dstData每一行占据的字节数大小，这个数值也是经过字节数对齐后的值。
        size_t dstSize = buffer.stride * 4;
        //rgba数据
        uint8_t *srcData = data[0];
        //sws_scale转换rgba数据后，每一行的字节数
        int srcSize = linesize[0];
        for (int i = 0; i < buffer.height; i++) {
            memcpy(dstData + i * dstSize, srcData + i * srcSize, srcSize);
        }
        ANativeWindow_unlockAndPost(window);
        return;
    } else {
        ANativeWindow_release(window);
        window = nullptr;
        LOGE("ANativeWindow_lock fail");
        return;
    }
}

VideoChannel::~VideoChannel() {
    _release();
}

void VideoChannel::_release() {
    isPlaying = false;
    setEnable(false);
    helper = nullptr;
    pkt_queue.clear();
    frame_queue.clear();
}


