//
// Created by chen_zi_rui on 2024/4/5.
//

#ifndef JNILEARN_LOG_H
#define JNILEARN_LOG_H

//  控制Android Native日志输出得头文件
// Created by ITplus on 2020/10/21.
//

#include <android/log.h>
/**
* 日志开关
* 1 开 ;其它关
*/
#define LOG_SWITCH 1
#define LOG_TAG "NativePlayer"

#if(LOG_SWITCH == 1)

#define LOGV(format, ...)  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, format, ##__VA_ARGS__)
#define LOGD(format, ...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format, ##__VA_ARGS__)
#define LOGW(format, ...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, format, ##__VA_ARGS__)
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, ##__VA_ARGS__)

#else

#define LOGV(...) NULL
	#define LOGD(...) NULL
	#define LOGI(...) NULL
	#define LOGW(...) NULL
	#define LOGE(...) NULL

#endif








#endif //JNILEARN_LOG_H
