#include <jni.h>
#include <string>
#include <android/log.h>
//定义宏
#define  LOG_TAG    "libgl2jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

static double r2d(AVRational rational) {
    return rational.num == 0 || rational.den == 0 ? 0 : (double) rational.num / rational.den;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_stringFromJNI(JNIEnv *env, jobject instance) {
    std::string hello = "Hello from C++";
    //ffmpeg信息
    hello += avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_open(JNIEnv *env, jobject instance, jstring url_,
                                            jobject handle) {
    const char *url = env->GetStringUTFChars(url_, 0);
    FILE *file = fopen(url, "rb");
    if (!file) {
        LOGE("打开失败　%s ", url);
    } else {
        LOGE("打开成功　%s ", url);
        fclose(file);
    };
    env->ReleaseStringUTFChars(url_, url);
    return true;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_jiefengzhuang(JNIEnv *env, jobject instance, jstring url_) {
    /***************代码 start****************/
    //初始化解封装
    av_register_all();
    //初始化网络
    avformat_network_init();
    //打开文件-->int avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
    AVFormatContext *avFormatContext = NULL;
    /*文件,地址路径　　http，rpsp，文件*/
    const char *url = env->GetStringUTFChars(url_, 0);
    /*封装格式　一般不用指定，让其内部自行设置*/
    AVInputFormat *avInputFormat = NULL;
    /*字典数组／一组设置　可以直接传NULL　*/
    AVDictionary *avDictionary = NULL;
    /*调用此方法前，确保avformat_network_init已经调用*/
    int ret = avformat_open_input(&avFormatContext, url, avInputFormat, 0);
    if (ret != 0) {
        LOGE("============>> avformat_open_input failed!　%s", av_err2str(ret));
        return;
    }
    LOGE("============>> avformat_open_input %s succeed!", url);
    //%lld　打印long long　类型
    LOGE("============>> duration: %lld nb_stream: %d", avFormatContext->duration,
         avFormatContext->nb_streams);
    //获取流信息　　用于文件没有头部信息时，探测部分文件获取文件信息
    ret = avformat_find_stream_info(avFormatContext, &avDictionary);
    if (ret != 0) {
        LOGE("============>> avformat_find_stream_info failed!　%s", av_err2str(ret));
    }
    int audioStream = 0;
    /*遍历获取文件　视频或者音频　参数*/
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *avStream = avFormatContext->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGE("视频格式！");
            int fps = r2d(avStream->avg_frame_rate);
            int width = avStream->codecpar->width;
            int height = avStream->codecpar->height;
            int codeid = avStream->codecpar->codec_type;
            LOGE("视频格式！");
            LOGE("fps = %d , width = %d , height = %d , codeid = %d ",
                 fps, width, height, codeid);
        } else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGE("音频格式！");
            audioStream = i;
            LOGE("audioStream = %d ", audioStream);
            LOGE("sample_rate = %d , channels = %d ,sample_format = %d",
                 avStream->codecpar->sample_rate, avStream->codecpar->channels,
                 avStream->codecpar->format);
        }
    }

    /*调用方法获取文件　视频或者音频　参数*/
    audioStream = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    LOGE("audioStream = %d ", audioStream);
    //关闭上下文
    avformat_close_input(&avFormatContext);
    /***************代码 end****************/

}