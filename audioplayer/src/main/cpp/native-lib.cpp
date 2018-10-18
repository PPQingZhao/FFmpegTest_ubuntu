#include <jni.h>
#include "include/native-lib.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_example_pp_opensltest_jni_Jni_openSlTest(JNIEnv *env, jclass type, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    LOGE("============>> url: %s" , url);
    
    //①创建引擎
    SLEngineItf  slEngineItf = CreateSL();
    if (!slEngineItf){
        LOGE("===========>> CreatedSL failed! ");
        return;
    }
    
    //②创建混音器
    SLObjectItf mix = NULL;

    SLresult slResult =0;
    slResult = (*slEngineItf)->CreateOutputMix(slEngineItf, &mix,0,0,0);
    if (slResult != SL_RESULT_SUCCESS){
        LOGE("==============>> CreateOutputMix failed!");
        return;
    }

    //实例化 第二个参数：表示阻塞式等待创建
    slResult = (*mix)->Realize(mix,SL_BOOLEAN_FALSE);
    if (slResult != SL_RESULT_SUCCESS){
        LOGE("==============>> Realize failed!");
        return;
    }

    //参数１: 输出类型  参数2: 混音器
    SLDataLocator_OutputMix slDataLocator_outputMix = {SL_DATALOCATOR_OUTPUTMIX,mix};
    //
    SLDataSink slDataSink = {&slDataLocator_outputMix,NULL};

    //③ 配置音频信息
    //缓冲队列 10个长度
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,10};
    //具体的音频格式
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, //pcm格式
                            2,                 //声道数
                            SL_SAMPLINGRATE_44_1,   //采样率
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,    //单个容器的大小
                            SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT, //前左声道　　前右声道
                            SL_BYTEORDER_LITTLEENDIAN //字节序,小端
                            };

    SLDataSource slDataSource = {&simpleBufferQueue,&pcm};
    
    //④ 创建播放器
    //创建对象
    SLObjectItf player = NULL;
    SLPlayItf slPlayItf = NULL;
    const  SLInterfaceID  slInterfaceID[] = {SL_IID_BUFFERQUEUE};
    const  SLboolean sLboolean[] = {SL_BOOLEAN_TRUE};
    SLAndroidSimpleBufferQueueItf  simpleBufferQueueItf_player = NULL;
    slResult = (*slEngineItf)->CreateAudioPlayer(slEngineItf,
                                      &player,
                                      &slDataSource,
                                      &slDataSink,
                                      sizeof(slInterfaceID)/ sizeof(sLboolean),
                                      slInterfaceID,
                                       sLboolean);

    if (slResult != SL_RESULT_SUCCESS){
        LOGE("==================>> CreateAudioPlayer failed!");
        return;
    }
    //　实例化播放器对象
    slResult = (*player)->Realize(player,SL_BOOLEAN_FALSE);
    if (slResult != SL_RESULT_SUCCESS){
        LOGE("==========>> (*player)->Realize failed!");
        return;
    }
    //获取player　接口
    slResult = (*player)->GetInterface(player,SL_IID_PLAY,&slPlayItf);
    if (slResult != SL_RESULT_SUCCESS){
        LOGE("================>> (*player)->GetInterface  SL_IID_PLAY failed!");
        return;
    }
    //获取　队列
    slResult = (*player)->GetInterface(player,SL_IID_BUFFERQUEUE,&simpleBufferQueueItf_player);
    if (slResult != SL_RESULT_SUCCESS){
        LOGE("================>> (*player)->GetInterface SL_IID_BUFFERQUEUE failed!");
        return;
    }

    //设置回调函数，播放队列空调用
    slResult = (*simpleBufferQueueItf_player)->RegisterCallback(simpleBufferQueueItf_player,PcmCallBack,NULL);
    if (slResult != SL_RESULT_SUCCESS){
        LOGE("================>>  (*simpleBufferQueueItf_player)->RegisterCallback failed!");
        return;
    }

    //设置播放器为播放状态
    (*slPlayItf)->SetPlayState(slPlayItf,SL_PLAYSTATE_PLAYING);
    //启动队列回调  插入一个字节
    (*simpleBufferQueueItf_player)->Enqueue(simpleBufferQueueItf_player,"",1);

    env->ReleaseStringUTFChars(url_, url);
}
