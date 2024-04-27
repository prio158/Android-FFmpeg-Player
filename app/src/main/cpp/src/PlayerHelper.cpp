//
// Created by chen_zi_rui on 2024/4/14.
//

#include "PlayerHelper.h"

PlayerHelper::PlayerHelper(JavaVM *vm, JNIEnv *env, jobject &jobj) : m_env(env) {
    m_obj_ref = env->NewGlobalRef(jobj);
    m_java_vm = vm;
    jclass jclazz = env->GetObjectClass(jobj);
    error_fun_id = env->GetMethodID(jclazz, "onError", "(I)V");
    prepare_fun_id = env->GetMethodID(jclazz, "onPrepare", "()V");
    progress_fun_id = env->GetMethodID(jclazz, "onProgress", "(I)V");
}

PlayerHelper::~PlayerHelper() {
    m_env->DeleteGlobalRef(m_obj_ref);
    m_obj_ref = nullptr;
}

void PlayerHelper::onError(int code, int thread) {
    bind(thread, error_fun_id, code);
}

void PlayerHelper::onPrepare(int thread) {
    bind(thread, prepare_fun_id);
}

void PlayerHelper::onProgress(int progress, int thread) {
    bind(thread, progress_fun_id);
}

void PlayerHelper::bind(int thread, jmethodID fun_id, ...) {
    va_list args;
    va_start(args, fun_id);
    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        /**JNIEnv是与线程绑定的，m_env是与主线程绑定的JNIEnv，所以在子线程中没法使用，所以要获取与子线程绑定  的JNIEnv
           我们没有办法像上面那样直接获得 JNIEnv，获取它的实例需要把你的子线程 Attach到JavaVM上去，
           调用的方法是 JavaVM::AttachCurrentThread，获取与当前线程绑定的JNIEnv
         */
        if (m_java_vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(m_obj_ref, fun_id, args);
        m_java_vm->DetachCurrentThread();
    } else {
        m_env->CallVoidMethod(m_obj_ref, fun_id, args);
    }
}