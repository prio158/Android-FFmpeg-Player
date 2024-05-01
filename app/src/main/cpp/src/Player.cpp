//
// Created by chen_zi_rui on 2024/4/13.
//

#include "Player.h"

void *prepareCallback(void *args) {
    auto player = static_cast<Player *>(args);
    player->_prepareTaskCallback();
    return nullptr;
}

void *startCallback(void *args) {
    auto player = static_cast<Player *>(args);
    player->_startTaskCallback();
    return nullptr;
}

Player::Player(JavaVM *vm, JNIEnv *env, jobject *jobj) {
    avformat_network_init();
    playerHelper = new PlayerHelper(vm, env, *jobj);
}

void Player::setDataSource(const char *path_) {
    /*字符串指针需要深拷贝*/
    auto dst = new char[strlen(path_) + 1];
    stpcpy(dst, path_);
    m_path = dst;
}

void Player::prepare() {
    pthread_create(&prepareTask, 0, prepareCallback, this);
}

void Player::_prepareTaskCallback() {
    avFormatContext = avformat_alloc_context();
    AVDictionary *opts = nullptr;
    int ret;
    /* 指定打开网络的超时时间 */
    av_dict_set(&opts, "timeout", "3000000", 0);
    ret = avformat_open_input(&avFormatContext, m_path, 0, 0);
    if (ret != 0) {
        LOGE("[avformat_open_input] error:%s,[path]:%s", av_err2str(ret), m_path);
        /* 这里打开媒体文件或网络出了问题，需要通知到Java层，这样方便Java层做一些UI的变更*/
        playerHelper->onError(FFMPEG_CANNOT_OPEN_URL, THREAD_CHILD);
        return;
    }
    LOGI("[avformat_open_input] success");
    /*查找媒体信息流*/
    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        LOGE("[avformat_find_stream_info] error:%s, [path]:%s", av_err2str(ret), m_path);
        playerHelper->onError(FFMPEG_CANNOT_FIND_STREAM, THREAD_CHILD);
        return;
    }
    duration = avFormatContext->duration / AV_TIME_BASE;
    nb_streams = avFormatContext->nb_streams;
    /* 提取音频、视频流*/
    for (int i = 0; i < nb_streams; i++) {
        auto stream = avFormatContext->streams[i];
        auto parma = stream->codecpar;
        auto decoder = avcodec_find_decoder(parma->codec_id);
        if (decoder == nullptr) {
            LOGE("[avcodec_find_decoder] fail, stream index:%d, codec_id:%d", i, parma->codec_id);
            playerHelper->onError(FFMPEG_CANNOT_FIND_DECODER, THREAD_CHILD);
            return;
        }
        /* 创建AvcodecContext，此时Context里面是空的，没有信息填充 */
        auto codec_context = avcodec_alloc_context3(decoder);
        if (codec_context == nullptr) {
            LOGE("[avcodec_alloc_context3] fail");
            playerHelper->onError(FFMPEG_CANNOT_ALLOCATE_DECODER, THREAD_CHILD);
            return;
        }
        ret = avcodec_parameters_to_context(codec_context, parma);
        if (ret < 0) {
            LOGE("[avcodec_parameters_to_context] error:%s", av_err2str(ret));
            playerHelper->onError(FFMPEG_CANNOT_COPY_DECODER_INFO, THREAD_CHILD);
            return;
        }
        ret = avcodec_open2(codec_context, decoder, nullptr);
        if (ret < 0) {
            LOGE("[avcodec_open2] error:%s", av_err2str(ret));
            playerHelper->onError(FFMPEG_CANNOT_OPEN_DECODER, THREAD_CHILD);
            return;
        }
        auto type = parma->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, playerHelper, codec_context, stream->time_base);
        } else if (type == AVMEDIA_TYPE_VIDEO) {
            auto fps = av_q2d(stream->avg_frame_rate);
            if (isnan(fps) || fps == 0) {
                fps = av_q2d(stream->r_frame_rate);
            }
            if (isnan(fps) || fps == 0) {
                fps = av_q2d(av_guess_frame_rate(avFormatContext, stream, nullptr));
            }
            videoChannel = new VideoChannel(i, playerHelper, codec_context, stream->time_base, fps);
        }
    }

    /* 没有视频流 */
    if (videoChannel == nullptr && audioChannel == nullptr) {
        LOGE("[path:%s] have no media data", m_path);
        playerHelper->onError(FFMPEG_NO_MEDIA_DATA, THREAD_CHILD);
        return;
    }

    LOGI("Native Player Prepare Finish");
    /* 告诉Java层，媒体信息准备完毕 */
    playerHelper->onPrepare(THREAD_CHILD);
}

void Player::_startTaskCallback() {
    int ret;
    while (isPlaying) {
        auto packet = av_packet_alloc();
        ret = av_read_frame(avFormatContext, packet);
        if (ret == 0) {
            if (packet->stream_index == videoChannel->channelId) {
                videoChannel->pkt_queue.enQueue(packet, true, true);
            } else if (packet->stream_index == audioChannel->channelId) {
                audioChannel->pkt_queue.enQueue(packet, false, true);
            } else {
                LOGE("UNKOWN, packet->stream_index:%d", packet->stream_index);
                av_packet_free(&packet);
            }
        } else if (ret == AVERROR_EOF) {
            // end of file, no data
            // 读取完毕，不一定播放完毕，因为不是说读一个packet，马上送到解码器里面去解码,而是送入缓冲队列中
            // 所以读取和播放不是同步的,读完了，数据都在队列里面，还没有播放完
            av_packet_free(&packet);
            if (!videoChannel->hasPacketData() && !videoChannel->hasFrameData()) {
                //播放完毕,才能break
                LOGD("Player READ, AVERROR_EOF");
                break;
            }
            LOGE("FILE READ EOF");
        } else {
            LOGD("Player READ,ERROR");
            av_packet_free(&packet);
            break;
        }
    }
    LOGD("Player READ,END");
    isPlaying = false;
    videoChannel->stop();
    audioChannel->stop();
}

void Player::start() {
    isPlaying = true;
    videoChannel->play();
    audioChannel->play();
    pthread_create(&startTask, nullptr, startCallback, this);
}

void Player::setNativeWindow(ANativeWindow *window_) {
    window = window_;
    if (videoChannel) {
        LOGD("VideoChannel::setWindow,videoChannel:%p", videoChannel);
        videoChannel->setWindow(window);
    }
}

Player::~Player() {
    release();
    avformat_network_deinit();
    if (m_path) {
        delete[] m_path;
        m_path = nullptr;
    }
    delete playerHelper;
}

void Player::stop() {
    LOGI("Player::stop()");
    isPlaying = false;
    pthread_join(prepareTask, nullptr);
    pthread_join(startTask, nullptr);
    /** 确定线程退出后，才会释放VideoChannel、AudioChannel*/
    release();
}

void Player::enable() {
    LOGI("Player::enable()");
    isPlaying = true;
}

void Player::release() {
    if (videoChannel) {
        videoChannel->stop();
        videoChannel = nullptr;
    }
    if (audioChannel) {
        audioChannel->stop();
        audioChannel = nullptr;
    }
    if (avFormatContext) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }
}




