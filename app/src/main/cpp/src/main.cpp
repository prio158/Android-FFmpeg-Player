#include <jni.h>
#include <string>
#include "Player.h"
#include "Log.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

JavaVM *javaVm = nullptr;

/**
 * @brief Java层调用System.load时调用，获取Java的Jvm指针，让Native层全局持有
 *
 * */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT jlong JNICALL

Java_com_example_player_common_Player_nativePlayerInit(JNIEnv *env, jobject thiz) {
    //auto player = Player::ptr(new Player);
    //return reinterpret_cast<jlong>(player.get());‘
    LOGI("nativePlayerInit");
    auto ptr = new Player(javaVm, env, &thiz);
    return reinterpret_cast<jlong>(ptr);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_common_Player_setDataSource(JNIEnv *env, jobject thiz, jlong native_ptr,
                                                    jstring path) {

    auto path_ = env->GetStringUTFChars(path, 0);

    auto nativePlayer = reinterpret_cast<Player *>(native_ptr);

    nativePlayer->setDataSource(path_);

    env->ReleaseStringUTFChars(path, path_);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_common_Player_nativeStart(JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto nativePlayer = reinterpret_cast<Player *>(native_ptr);
    nativePlayer->start();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_common_Player_setSurface(JNIEnv *env, jobject thiz, jlong native_ptr,
                                                 jobject surface) {
    auto nativePlayer = reinterpret_cast<Player *>(native_ptr);
    auto window = ANativeWindow_fromSurface(env, surface);
    LOGD("NativePlayer::setWindow XXX window:%p", window);
    nativePlayer->setNativeWindow(window);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_common_Player_nativePrepare(JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto nativePlayer = reinterpret_cast<Player *>(native_ptr);
    LOGD("JNI nativePlayer->prepare() Call");
    nativePlayer->prepare();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_common_Player_nativeStop(JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto nativePlayer = reinterpret_cast<Player *>(native_ptr);
    LOGD("JNI nativePlayer->stop() Call");
    nativePlayer->stop();
    delete nativePlayer;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_common_Player_nativeEnable(JNIEnv *env, jobject thiz,
                                                   jlong native_ptr) {
    auto nativePlayer = reinterpret_cast<Player *>(native_ptr);
    LOGD("JNI nativePlayer->enable() Call");
    nativePlayer->enable();
}