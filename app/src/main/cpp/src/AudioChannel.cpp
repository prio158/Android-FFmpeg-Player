//
// Created by chen_zi_rui on 2024/4/21.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int channelId, PlayerHelper *helper, AVCodecContext *avCodecContext,
                           const AVRational &base) : BaseChannel(channelId, helper, avCodecContext,
                                                                 base) {
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    per_samples_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    sample_rate = 44100;
    sample_size = sample_rate * out_channels * per_samples_size;
    /*一次采样所需要的空间*/
    buffer = new uint8_t[sample_size];
}

AudioChannel::~AudioChannel() {


}

void *play_audio(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->_play();
    return nullptr;
}

void *decode_audio(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return nullptr;
}


void AudioChannel::play() {
    setEnable(true);
    isPlaying = true;
    _initAudioResample();
    pthread_create(&audioDecodeTask, nullptr, play_audio, this);
    pthread_create(&audioPlayTask, nullptr, decode_audio, this);
}

void AudioChannel::stop() {

}

void AudioChannel::decode() {
    AVPacket *pkt = nullptr;
    int ret;
    while (isPlaying) {
        ret = pkt_queue.deQueue(pkt, false,true);

        if (!isPlaying)
            break;

        if (!ret)
            continue;

        ret = avcodec_send_packet(avCodecContext, pkt);
        releaseAvPacket(pkt);
        if (ret < 0) {
            LOGE("avcodec_send_audio_packet error:%s", av_err2str(ret));
            break;
        }

        auto frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == 0) {
            frame_queue.enQueue(frame,false);
        } else if (ret == AVERROR(EAGAIN)) {
            releaseAvPacket(pkt);
            continue;
        } else {
            LOGE("avcodec_receive_audio_frame error:%s", av_err2str(ret));
            releaseAvPacket(pkt);
            break;
        }
    }
    releaseAvPacket(pkt);
}

void audioPlayCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    LOGI("Audio Play Callback Execute");
    auto audioChannel = static_cast<AudioChannel *>(pContext);
    int size = audioChannel->_getData();
    if (size > 0) {
        (*caller)->Enqueue(caller, audioChannel->buffer, size);
    }
}


void AudioChannel::_play() {
    //1、创建引擎接口对象
    SLObjectItf engineObject = nullptr;
    SLresult result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("SlCreateEngine fail");
        return;
    }
    //2、调用 Realize 方法来实现引擎接口对象
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Realize fail");
        return;
    }
    //3、获取引擎接口
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("GetInterface fail");
        return;
    }

    //4、创建混音器
    SLObjectItf outputMixObject = NULL;
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //5、创建播放器
    //创建buffer缓冲类型的队列作为数据定位器（获取播放数据） 2个缓冲区
    //它允许你将音频数据存储在一个简单的缓冲队列中，以便播放器可以按需获取数据并播放。
    //为什么是 2 个缓冲区？
    //使用两个缓冲区的好处在于，你可以在一个缓冲区正在播放时，填充另一个缓 冲区的新数据。这样可以实现数据的连续播放，避免中断。
    //此外，直到最近，如果你想使用 Android 的低延迟音频路径，至少需要设置两个缓冲区。
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2};

    //pcm数据格式: pcm、声道数、采样率、采样位、容器大小、通道掩码(双声道)、字节序(小端)
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};

    //数据源 （数据获取器+格式）
    SLDataSource slDataSource = {&android_queue, &pcm};
    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    //播放器对象
    SLObjectItf bqPlayerObject = NULL;

    //Object 可能会存在一个或者多个 Interface，官方为每一种 Object 都定义了一系列的 Interface；
    //Object 对象提供了各种操作，如果希望使用该对象支持的功能函数，则必须通过其 GetInterface 函数拿到
    //Interface 接口，然后通过 Interface 来访问功能函数 。
    //用来存放播放器对象的接口,这里只需要获取播放器对象的一个接口，所以数组的长度给1
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    //创建播放器
    //audioSnk里面包装混音器传到播放器里面
    //而播放器相当于对混音器做了一层装饰，对外提供额外的方法，例如：开始、停止等方法，真正用来播放的是混音器
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource,
                                          &audioSnk, 1,
                                          ids, req);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    //获取队列的操作接口
    SLAndroidSimpleBufferQueueItf bufferQueueItf = nullptr;
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bufferQueueItf);
    //设置回调（启动播放器后执行回调来获取数据并播放）
    result =  (*bufferQueueItf)->RegisterCallback(bufferQueueItf, audioPlayCallback, this);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //获取播放器状态接口
    SLPlayItf bqPlayerItf = nullptr;
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerItf);

    //设置播放器状态
    (*bqPlayerItf)->SetPlayState(bqPlayerItf, SL_PLAYSTATE_PLAYING);

    //还要手动调用一次，回调方法，才能开始播放
    audioPlayCallback(bufferQueueItf, this);
}


int AudioChannel::_getData() {
    AVFrame *avFrame = nullptr;
    int ret;
    int data_size = 0;
    while (isPlaying) {
        ret = frame_queue.deQueue(avFrame, false,false);
        if (!isPlaying)
            break;

        if (!ret)
            continue;

        int nbs = swr_convert(swrContext, &buffer, sample_size,
                              (const uint8_t **) avFrame->data,
                              avFrame->nb_samples);

        data_size = nbs * out_channels * per_samples_size;
        releaseAvFrame(avFrame);
        break;
    }
    return data_size;
}

void AudioChannel::_initAudioResample() {
    swrContext = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                    avCodecContext->channel_layout,
                                    avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate,
                                    0, nullptr);

    swr_init(swrContext);
}



