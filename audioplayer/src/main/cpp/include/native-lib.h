#include <jni.h>
#include <string>
#include <android/log.h>
#include <SLES/OpenSLES.h>          //音频播放器
#include <SLES/OpenSLES_Android.h>　//音频播放器
//定义宏
#define  LOG_TAG    "libgl2jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)



static SLObjectItf slObjectItf = NULL;

//音频播放器引擎初始化
SLEngineItf CreateSL() {
    SLresult  sLresult;
    SLEngineItf slEngineItf = NULL;

    //创建引擎对象
    sLresult = slCreateEngine(&slObjectItf,0,0,0,0,0);
    if (sLresult != SL_RESULT_SUCCESS)//创建失败
        return NULL;

    //实例化
    sLresult = (*slObjectItf)->Realize(slObjectItf,SL_BOOLEAN_TRUE);
    if (sLresult != SL_RESULT_SUCCESS){
        return NULL;
    }

    //获取接口
    sLresult = (*slObjectItf)->GetInterface(slObjectItf, SL_IID_ENGINE, &slEngineItf);
    if (sLresult != SL_RESULT_SUCCESS){
        return NULL;
    }
    return slEngineItf;
}

void PcmCallBack(SLAndroidSimpleBufferQueueItf slAndroidSimpleBufferQueueItf,void* context){
    LOGE("===========>> call function PcmCallBack!");
    //打开文件
    static FILE *file = NULL;
    if (!file){
        file= fopen("/sdcard/ffmpeg/test.pcm","rb");
    }

    if (!file){
        LOGE("===========>>file fopen failed!");
        return;
    }

    static char *buf = NULL;
    if (!buf){
        buf = new char[1024*1024];
    }

    //是否结尾  0表示没有到结尾
    if (feof(file) == 0){
        //读取内容  读取一个单位为1024个字节的数据
        int len = fread(buf,1,1024,file);
        if (len > 0){
            //发送音频数据 递归调用
            (*slAndroidSimpleBufferQueueItf)->Enqueue(slAndroidSimpleBufferQueueItf,buf,len);
        }
    }
}
