//
// Created by chen_zi_rui on 2024/4/14.
//

#ifndef PLAYER_PLAYERHELPER_H
#define PLAYER_PLAYERHELPER_H

#include <jni.h>
#include <functional>


#define THREAD_MAIN 1
#define THREAD_CHILD 2
#define FFMPEG_CANNOT_OPEN_URL 3
#define FFMPEG_CANNOT_FIND_STREAM 4
#define FFMPEG_CANNOT_FIND_DECODER 5
#define FFMPEG_CANNOT_ALLOCATE_DECODER 6
#define FFMPEG_CANNOT_COPY_DECODER_INFO 7
#define FFMPEG_CANNOT_OPEN_DECODER 8
#define FFMPEG_NO_MEDIA_DATA 9

class PlayerHelper {

public:
    PlayerHelper(JavaVM *vm, JNIEnv *env, jobject &jobj);

    ~PlayerHelper();

    void onError(int code, int thread = THREAD_MAIN);

    void onPrepare(int thread = THREAD_MAIN);

    void onProgress(int progress, int thread = THREAD_MAIN);

private:
     void bind(int thread,jmethodID fun_id, ...);

private:
    jobject m_obj_ref{};
    jmethodID error_fun_id;
    jmethodID prepare_fun_id;
    jmethodID progress_fun_id;
    JNIEnv *m_env;
    JavaVM* m_java_vm;

};


#endif //PLAYER_PLAYERHELPER_H
