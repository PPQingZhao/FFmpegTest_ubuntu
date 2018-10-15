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

void avformat_find_stream_info(AVFormatContext *avFormatContext, AVDictionary *avDictionary) {
    LOGE("============>> avformat_find_stream_info");
    int ret = avformat_find_stream_info(avFormatContext, &avDictionary);
    if (ret != 0) {
        LOGE("============>> avformat_find_stream_info failed!　%s", av_err2str(ret));
    }
    int audioStream = 0;
    int videoStream = 0;
    LOGE("========>>avFprmatContext.nb_stream: %d  ", avFormatContext->nb_streams);
    /*遍历获取文件　视频或者音频　参数*/
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *avStream = avFormatContext->streams[i];
        LOGE("===>> avStream.duration: %d", avStream->duration);
        LOGE("===>> avFormatContext.duration: %d", avFormatContext->duration);
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGE("视频格式！");
            int fps = r2d(avStream->avg_frame_rate);
            int width = avStream->codecpar->width;
            int height = avStream->codecpar->height;
            int codeid = avStream->codecpar->codec_id;
            int pixformat = avStream->codecpar->format;//像素格式
            LOGE("fps = %d , width = %d , height = %d , codeid = %d pix_Format　＝　%d",
                 fps, width, height, codeid, pixformat);
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
    videoStream = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    LOGE("videoStream = %d ", videoStream);
}

void m_av_read_frame(AVFormatContext *avFormatContext, AVPacket *avPacket) {
    int videoStream = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int audioStream = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    LOGE("====>> videoStream: %d ",videoStream);
    LOGE("====>> audioStream: %d ",audioStream);
    //return 0 if OK, < 0 on error or end of file
    LOGE("avFormatContext====>> duration:  %lld ",avFormatContext->duration);
    LOGE("videoStream====>> duration:  %lld ",avFormatContext->streams[videoStream]->duration);
    LOGE("audioStream====>> duration:  %lld ",avFormatContext->streams[audioStream]->duration);
    int pos = 15 * r2d(avFormatContext->streams[videoStream]->time_base);
    for (;;) {
        int ret = av_read_frame(avFormatContext, avPacket);
        if (ret != 0) {
            LOGE("读取到结尾");

            LOGE("======>> pos: %d ",pos);
            int ret = av_seek_frame(avFormatContext, videoStream, pos,
                          AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            LOGE("=================>> ret : %d", ret);
            continue;
//            break;
        }
        LOGE(" =========>>stream: %d  size: %d flags: %d pts: %lld ",
             avPacket->stream_index,avPacket->size, avPacket->flags, avPacket->pts);
    }
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
    int ret = avformat_open_input(&avFormatContext, url, 0, 0);
    if (ret != 0) {
        LOGE("============>> avformat_open_input failed!　%s", av_err2str(ret));
        return;
    }
    LOGE("============>> avformat_open_input %s succeed!", url);
    //%lld　打印long long　类型　如果这里获取不到长度,就使用　AvStream 里面的
    LOGE("============>> duration: %lld nb_stream: %d", avFormatContext->duration,
         avFormatContext->nb_streams);


    //①　获取流信息　　用于文件没有头部信息时，探测部分文件获取文件信息
//    avformat_find_stream_info(avFormatContext, avDictionary);

    //②读音视频帧数据
    /***************AVPacket 相关函数********************/
    //创建并初始化　AVPacket 　会申请堆内存空间，需要主动释放内存
    AVPacket *avPacket = av_packet_alloc();
    //空间复制　，创建并并用计数
//    AVPacket *avPacket_clone = av_packet_clone(avPacket);
//    int av_packet_ref(AVPacket *dst, const AVPacket *src);　//增加一个引用
//    void av_packet_unref(AVPacket *pkt);  //减少一个引用
//    void av_packet_free(AVPacket **pkt);  //清空对象并减少引用计数
    //手动创建
//    void av_init_packet(AVPacket *pkt);   //初始化　默认值
//    int av_packet_from_data(AVPacket *pkt, uint8_t *data, int size);
    m_av_read_frame(avFormatContext, avPacket);


//seek　操作  进度条
//    stream_index: 音频或视频
//    timestamp：　//时间AVStream.time_base
//    flags:　视频关键帧标示位：AVSEEK_FLAG_BACKWARD
//    　                 AVSEEK_FLAG_BYTE
//    　                 AVSEEK_FLAG_ANY
//    　                 AVSEEK_FLAG_FRAME
//    int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,
//                      int flags);

//关闭上下文
    avformat_close_input(&avFormatContext);
/***************代码 end****************/
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_avdeCode(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    //初始化
    av_register_all();
    //初始化网络
    avformat_network_init();
    //初始化AVFormatContext
    AVFormatContext *avFormatContext = NULL;
    avformat_open_input(&avFormatContext,url,0,0);

    //初始化解码器
    avcodec_register_all();

    int videoStream = av_find_best_stream(avFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
//    avcodec_parameters_to_context(avCodecContext,avFormatContext->streams[videoStream].)

    //软解码　->根据视频流中codec_id获取解码器
    AVCodec *avCodec = avcodec_find_decoder(avFormatContext->streams[videoStream]->codecpar->codec_id);
    //硬解码
    avCodec = avcodec_find_encoder_by_name("h264_mediacodec");
    if (!avCodec){
        LOGE("avcodec_find_decoder failed!");
        return;
    }

    //创建解码器上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(0);
    avCodecContext->thread_count = 1;   //线程数


    //释放内存
    avcodec_free_context(&avCodecContext);
    avformat_free_context(avFormatContext);
}