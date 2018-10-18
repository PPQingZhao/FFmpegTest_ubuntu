#include <jni.h>
#include <string>
#include <android/log.h>
#include <SLES/OpenSLES.h>          //音频播放器
#include <SLES/OpenSLES_Android.h>　//音频播放器
//定义宏
#define  LOG_TAG    "libgl2jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


