#include <jni.h>
#include <string>
#include <android/log.h>
//定义宏
#define  LOG_TAG    "libgl2jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>     //解码器
#include <libavformat/avformat.h>   //文件参数信息
#include <libavcodec/jni.h>         //硬解码
#include <libswscale/swscale.h>     //像素格式和尺寸的转换
#include <libswresample/swresample.h>   //音频重采样
#include <android/native_window.h>      //显示窗口
#include <android/native_window_jni.h>  //显示窗口
}

//程序入口第一个调用的函数  硬解码必须重写这个方法，传入java环境，才能进行硬解码
extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    JNIEnv *venv;
    if (vm->GetEnv((void **) &venv, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("============>> JNI_OnLoad  failed!");
        return -1;
    }
    av_jni_set_java_vm(vm, NULL);
    LOGE("============>> JNI_OnLoad succeed!");
    return JNI_VERSION_1_6;
}

static double r2d(AVRational rational) {
    return rational.num == 0 || rational.den == 0 ? 0 : (double) rational.num / rational.den;
}

//获取当前时间戳
long long GetCurrentMs() {
    struct timeval time;
    gettimeofday(&time, NULL);
    //秒
    int sec = time.tv_sec;
    //time.tv_usec这个是微秒　然后除以一千转换成秒　　t是毫秒数
    long long t = sec * 1000 + time.tv_usec / 1000;
    return t;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_stringFromJNI(JNIEnv *env, jobject instance) {
    std::string hello = "Hello from C++";
    //ffmpeg信息
    hello += avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
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
    LOGE("====>> videoStream: %d ", videoStream);
    LOGE("====>> audioStream: %d ", audioStream);
    //return 0 if OK, < 0 on error or end of file
    LOGE("avFormatContext====>> duration:  %lld ", avFormatContext->duration);
    LOGE("videoStream====>> duration:  %lld ", avFormatContext->streams[videoStream]->duration);
    LOGE("audioStream====>> duration:  %lld ", avFormatContext->streams[audioStream]->duration);
    int pos = 15 * r2d(avFormatContext->streams[videoStream]->time_base);
    for (;;) {
        int ret = av_read_frame(avFormatContext, avPacket);
        if (ret != 0) {
            LOGE("读取到结尾");

            LOGE("======>> pos: %d ", pos);
            int ret = av_seek_frame(avFormatContext, videoStream, pos,
                                    AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            LOGE("=================>> ret : %d", ret);
            continue;
//            break;
        }
        LOGE(" =========>>stream: %d  size: %d flags: %d pts: %lld ",
             avPacket->stream_index, avPacket->size, avPacket->flags, avPacket->pts);
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
    avformat_open_input(&avFormatContext, url, 0, 0);

    int videoStream = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    //初始化解码器
    avcodec_register_all();

    /******************视频解码器皿 start********************/
    /*//软解码　->根据视频流中codec_id获取解码器
    AVCodec *avCodec_video = avcodec_find_decoder(
            avFormatContext->streams[videoStream]->codecpar->codec_id);*/
    //硬解码h264_mediacodec
    AVCodec *avCodec_video = avcodec_find_decoder_by_name("h264_mediacodec");
    if (!avCodec_video) {
        LOGE("video avcodec_find_encoder_by_name failed!");
//        return;
        //软解码　->根据视频流中codec_id获取解码器
        avCodec_video = avcodec_find_decoder(
                avFormatContext->streams[videoStream]->codecpar->codec_id);
    }

    //创建解码器上下文  初始化
    AVCodecContext *avCodecContext_video = avcodec_alloc_context3(avCodec_video);
    //将 AVStream里面的参数复制到　AVCodecContext
    avcodec_parameters_to_context(avCodecContext_video,
                                  avFormatContext->streams[videoStream]->codecpar);
    avCodecContext_video->thread_count = 1;   //线程数
    //打开解码器
    int ret = avcodec_open2(avCodecContext_video, avCodec_video, 0);
    if (ret != 0) {
        LOGE("========>>video avcodec_open2 failed! ");
        return;
    }
    /***********************视频解码器 end***********************/
    //初始化
    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();
    long long startTime = GetCurrentMs();
    //视频帧数量
    int frameCount = 0;
    for (;;) {
        //超过三秒
        if (GetCurrentMs() - startTime >= 3000) {
            LOGE("=========>>noe decodec fps is %d", frameCount / 3);
            frameCount = 0;
            startTime = GetCurrentMs();
        }
        //读取一帧数据
        int ret = av_read_frame(avFormatContext, avPacket);
        if (ret != 0) {
            LOGE("读取到文件结尾");
            continue;
        }

        //只处理视频数据
        if (avPacket->stream_index != videoStream) {
            continue;
        }
        //发送数据到线程中解码
        ret = avcodec_send_packet(avCodecContext_video, avPacket);
        //清理
        av_packet_unref(avPacket);
//        LOGE("======>>stream_index: %d ",avPacket->stream_index);
        if (ret != 0) {
//            LOGE("avcodec send packet failed!");
            continue;
        }

        //需要接收多次，才能取出全部数据
        for (;;) {
            //接收
            ret = avcodec_receive_frame(avCodecContext_video, avFrame);
            if (ret != 0) {
//                LOGE("=====>> avcodec_recevice_frame failed!");
                break;
            }
//            LOGE("====>>avFrame.pts: %lld", avFrame->pts);

        }

        frameCount++;

    }


    /******************音频解码器 start********************/
    int audioStream = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    //软解码　->根据视频流中codec_id获取解码器
    AVCodec *avCodec_audio = avcodec_find_decoder(
            avFormatContext->streams[audioStream]->codecpar->codec_id);
    //硬解码
//    avCodec_audio = avcodec_find_encoder_by_name("h264_mediacodec");
    if (!avCodec_audio) {
        LOGE("audio avcodec_find_decoder failed!");
        return;
    }

    //创建解码器上下文  初始化
    AVCodecContext *avCodecContext_audio = avcodec_alloc_context3(avCodec_audio);
    //将 AVStream里面的参数复制到　AVCodecContext
    avcodec_parameters_to_context(avCodecContext_audio,
                                  avFormatContext->streams[audioStream]->codecpar);
    avCodecContext_video->thread_count = 1;   //线程数
    //打开解码器
    ret = avcodec_open2(avCodecContext_audio, avCodec_audio, 0);
    if (ret != 0) {
        LOGE("========>>audio avcodec_open2 failed! ");
        return;
    }
    /***********************音频解码器 end***********************/

    //释放内存
    avcodec_free_context(&avCodecContext_video);
    avformat_free_context(avFormatContext);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_pixAndSizeChange(JNIEnv *env, jobject instance,
                                                        jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    //解封装器初始化
    av_register_all();
    //初始化网络
    avformat_network_init();
    //获取文件信息
    AVFormatContext *avFormatContext = NULL;    //上下文
    int ret =avformat_open_input(&avFormatContext, url, NULL, NULL);
    if (ret != 0){
        LOGE("=========>> avFormat_open_input failed!");
        return;
    }

    //找到视频流索引
    int videoStream = av_find_best_stream(avFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    //音频流解码器
    int audioStream = av_find_best_stream(avFormatContext,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    LOGE("====>> videoStream: %d　audioStream: %d  ", videoStream,audioStream);

    //解码器初始化
    avcodec_register_all();
    //视频解码器对象
    AVCodec *avCodec_video = NULL;
    //音频解码器
    AVCodec *avCodec_audio = NULL;
    //视频软解码器
    avCodec_video = avcodec_find_decoder(avFormatContext->streams[videoStream]->codecpar->codec_id);
    avCodec_audio = avcodec_find_decoder(avFormatContext->streams[audioStream]->codecpar->codec_id);
    //硬解码
//    avCodec_video = avcodec_find_decoder_by_name("h264_mediacodec");
//    avCodec_audio = avcodec_find_decoder_by_name("h264_mediacodec");
    if (!avCodec_video || !avCodec_audio){
        LOGE("=================>> avcodec_find_decoder failed!");
        return;
    }

    //视频流解码器上下文
    AVCodecContext *avCodecContext_video = avcodec_alloc_context3(avCodec_video);
    //音频流解码器上下文
    AVCodecContext *avCodecContext_audio = avcodec_alloc_context3(avCodec_audio);
    avCodecContext_video->thread_count = 1; //解码线程数
    avCodecContext_audio->thread_count = 1;
    //AVStream　参数复制到 avCodecContext_video
    avcodec_parameters_to_context(avCodecContext_video,avFormatContext->streams[videoStream]->codecpar);
    avcodec_parameters_to_context(avCodecContext_audio,avFormatContext->streams[audioStream]->codecpar);

    ret = avcodec_open2(avCodecContext_video,avCodec_video,NULL);
    if (ret != 0){
        LOGE("=======>> video avcodec_open2 failed!");
        return;
    }
    ret = avcodec_open2(avCodecContext_audio,avCodec_audio,NULL);
    if (ret != 0){
        LOGE("=======>> audio avcodec_open2 failed!");
        return;
    }

    //读取帧
    AVPacket *avPacket = av_packet_alloc();
    AVFrame  *avFrame = av_frame_alloc();
    //视频：定义像素格式上下文　初始化在成功接收到帧数据之后
    SwsContext *swsContext = NULL;
    int outWidth = 1280;
    int outHeight = 720;
    char *rgb_buf = new char[1920*1080*4];
    char *pcm_buf = new char[48000*4*2];

    //音频重采样　上下文　初始化
    SwrContext *swrContext = swr_alloc();
    //设置参数　
    swrContext = swr_alloc_set_opts(swrContext,
                                    av_get_default_channel_layout(2), // 通道数：　这里固定两声道　也可以使用原始的（avCodecContext_audio->channels）
                                    AV_SAMPLE_FMT_S16,                //　输出格式
                                    avCodecContext_audio->sample_rate,//样本率
                                    //输入参数
                                    av_get_default_channel_layout(avCodecContext_audio->channels),
                                    avCodecContext_audio->sample_fmt,  //输入格式
                                    avCodecContext_audio->sample_rate, //样本采样率
                                    0,0);
    ret =swr_init(swrContext);
    if (ret != 0){
        LOGE("==============>> swr init failed! ");
    }else{
        LOGE("==============>> swr init succeed! ");
    }

    for (;;) {
        int ret= av_read_frame(avFormatContext, avPacket);
        if (ret != 0){
            LOGE("===============>> 读取帧数，文件结尾");
            continue;
        }
        AVCodecContext *cc = NULL;
        if (avPacket->stream_index == videoStream){ //处理视频流  进行解码
            cc = avCodecContext_video;
            //发数据到线程
            int ret = avcodec_send_packet(avCodecContext_video,avPacket);
            LOGE("=====>>video  avcodec_send_packet!");
            //清理
            av_packet_unref(avPacket);
            if(ret != 0){ //发送失败
                LOGE("=====>>video avcodec send packet failed!");
                continue;
            }
        }else if(avPacket->stream_index == audioStream){ //处理音频流
            cc = avCodecContext_audio;
            LOGE("=====>>audio  avcodec_send_packet!");
            //发数据到线程
            int ret = avcodec_send_packet(avCodecContext_audio,avPacket);
            //清理
            av_packet_unref(avPacket);
            if(ret != 0){ //发送失败
                LOGE("=====>>audio avcodec send packet failed!");
                continue;
            }
        }

        //接收上面发过来的数据
        for (;;) {
            if (cc == avCodecContext_video) {    //视频
                int ret = avcodec_receive_frame(avCodecContext_video, avFrame);
                if (ret != 0) { //接收数据失败
                    LOGE("video avcodec_ recrvice＿frame failed!");
                    break;
                }

                //像素格式的转换和像素格式的转换
                swsContext = sws_getCachedContext(swsContext,
                                                  avFrame->width,
                                                  avFrame->height,
                                                  (AVPixelFormat) avFrame->format,
                                                  outWidth,
                                                  outHeight,
                                                  AV_PIX_FMT_RGBA,
                                                  SWS_FAST_BILINEAR, NULL, NULL, NULL);
                if (!swsContext) {
                    LOGE("============>> sws_getCachedContext failed!");
                } else {
                    uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
                    data[0] = (uint8_t *) rgb_buf;
                    int lines[AV_NUM_DATA_POINTERS] = {0};
                    lines[0] = outWidth * 4;
                    //数据高度
                    int h = sws_scale(swsContext,
                                      (const uint8_t *const *) avFrame->data,
                                      avFrame->linesize,
                                      0,
                                      avFrame->height,
                                      data,
                                      lines);
                    LOGE("===============>>sws_scale  h : %d ", h);
                }
            }else if(cc == avCodecContext_audio){  //音频
                int ret = avcodec_receive_frame(avCodecContext_audio, avFrame);
                if (ret != 0) { //接收数据失败
                    LOGE("audio avcodec_ recrvice＿frame failed!");
                    break;
                }
                //音频重采样
                uint8_t *out[2] = {0};
                //pcm_buf 大小与rgb_buf相关
                out[0] = (uint8_t *) pcm_buf;
                int len =swr_convert(swrContext,
                                     out,
                                     avFrame->nb_samples,
                                     (const uint8_t **) avFrame->data,
                                     avFrame->nb_samples);
                LOGE("===============>>swr_convert len = %d  ",len);
            }
        }
    }
    //释放空间
    delete rgb_buf;
    delete pcm_buf;
    //关闭上下文
    avformat_close_input(&avFormatContext);
    env->ReleaseStringUTFChars(url_, url);
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_open(JNIEnv *env, jobject instance, jstring url_,
                                            jobject surface) {
    const char *url = env->GetStringUTFChars(url_, 0);
    //初始化　解封装
    av_register_all();
    //初始化网络
    avformat_network_init();
    //初始化解封装上下文
    AVFormatContext *avFormatContext =NULL;
    int ret = avformat_open_input(&avFormatContext,url,NULL,NULL);
    if (ret != 0){
        LOGE("===========>> avformat open input failed!");
        return false;
    }
    int videoStream = av_find_best_stream(avFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    int audioStream = av_find_best_stream(avFormatContext,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);

    avcodec_register_all();
    //找到视频软解码器
    AVCodec *avCodec_video = avcodec_find_decoder(avFormatContext->streams[videoStream]->codecpar->codec_id);
    //音频软解码器
    AVCodec *avCodec_audio = avcodec_find_decoder(avFormatContext->streams[audioStream]->codecpar->codec_id);

    if (!avCodec_video || !avCodec_audio){
        LOGE("=====>> avcodec find decoder failed!");
        return false;
    }
    LOGE("===========>> 4444444444444");

    //初始化视频流解码器上下文
    AVCodecContext *avCodecContext_video = avcodec_alloc_context3(avCodec_video);
    avCodecContext_video->thread_count = 1;
    ret = avcodec_parameters_to_context(avCodecContext_video,avFormatContext->streams[videoStream]->codecpar);
    if (ret != 0){
        LOGE("=========>>video avcoedv paramets to context failed! ");
        return false;
    }

    //初始化音频流解码器上下文
    AVCodecContext *avCodecContext_audio = avcodec_alloc_context3(avCodec_audio);
    avCodecContext_audio->thread_count = 1;
    ret = avcodec_parameters_to_context(avCodecContext_audio,avFormatContext->streams[audioStream]->codecpar);
    if (ret != 0){
        LOGE("=========>>audio avcoedv paramets to context failed! ");
        return false;
    }

    ret = avcodec_open2(avCodecContext_video,avCodec_video,NULL);
    if (ret != 0){
        LOGE("===============>>video avcodec open2 failed! ");
        return false;
    }
    ret = avcodec_open2(avCodecContext_audio,avCodec_audio,NULL);
    if (ret != 0){
        LOGE("==============>> audio avcodec open2 failed!");
        return false;
    }
    //读取帧数
    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();

    //视频：定义像素格式上下文　初始化在成功接收到帧数据之后
    SwsContext *swsContext = NULL;
    int outWidth = 1280;
    int outHeight = 720;
    char *rgb_buf = new char[1920*1080*4];

    //音频重采样　上下文　初始化
    char *pcm_buf = new char[48000*4*2];
    SwrContext *swrContext = swr_alloc();
    //设置参数　
    swrContext = swr_alloc_set_opts(swrContext,
                                    av_get_default_channel_layout(2), // 通道数：　这里固定两声道　也可以使用原始的（avCodecContext_audio->channels）
                                    AV_SAMPLE_FMT_S16,                //　输出格式
                                    avCodecContext_audio->sample_rate,//样本率
                                    //输入参数
                                    av_get_default_channel_layout(avCodecContext_audio->channels),
                                    avCodecContext_audio->sample_fmt,  //输入格式
                                    avCodecContext_audio->sample_rate, //样本采样率
                                    0,0);
    ret =swr_init(swrContext);
    if (ret != 0){
        LOGE("==============>> swr init failed! ");
    }else{
        LOGE("==============>> swr init succeed! ");
    }

    //显示窗口初始化
    ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(env,surface);
    if (!aNativeWindow){
        LOGE("===============>> anativewindoe fromsurface failed!");
        return false;
    }
    int vWidth= avCodecContext_video->width;
    int vHeight = avCodecContext_video->height;
    //设置 窗口的大小和格式
//    ANativeWindow_setBuffersGeometry(aNativeWindow,vWidth,vHeight,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_setBuffersGeometry(aNativeWindow,outWidth,outHeight,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer aNativeWindow_buffer;

    for (;;) {
        int ret = av_read_frame(avFormatContext, avPacket);
        if (ret != 0){
            LOGE("===>> av read farame failed!");
            continue;
        }
        AVCodecContext *cc = NULL;
        //发送数据包
        if (avPacket->stream_index == videoStream){ //处理视频流
            cc = avCodecContext_video;
            ret = avcodec_send_packet(avCodecContext_video,avPacket);
            //清理
            av_packet_unref(avPacket);
            if (ret != 0){
                LOGE("=====>>video avcodec send packet failed!");
                continue;
            }
        }else if(avPacket->stream_index == audioStream){ //处理音频流
            cc = avCodecContext_audio;
            ret = avcodec_send_packet(avCodecContext_audio,avPacket);
            //清理
            av_packet_unref(avPacket);
            if (ret != 0){
                LOGE("=====>>audio avcodec send packet failed!");
                continue;
            }
        }

        for (;;) {
            //接收数据
            if (cc == avCodecContext_video) { //视频流
                ret = avcodec_receive_frame(avCodecContext_video, avFrame);
                if (ret != 0) {
                    LOGE("===========>>video avcodec receive frame failed!");
                    break;
                }

                //像素格式的转换和像素格式的转换
                swsContext = sws_getCachedContext(swsContext,
                                                  avFrame->width,
                                                  avFrame->height,
                                                  (AVPixelFormat) avFrame->format,
                                                  outWidth,
                                                  outHeight,
                                                  AV_PIX_FMT_RGBA,
                                                  SWS_FAST_BILINEAR, NULL, NULL, NULL);
                if (!swsContext) {
                    LOGE("============>> sws_getCachedContext failed!");
                } else {
                    uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
                    data[0] = (uint8_t *) rgb_buf;
                    int lines[AV_NUM_DATA_POINTERS] = {0};
                    lines[0] = outWidth * 4;
                    //数据高度
                    int h = sws_scale(swsContext,
                                      (const uint8_t *const *) avFrame->data,
                                      avFrame->linesize,
                                      0,
                                      avFrame->height,
                                      data,
                                      lines);
                    LOGE("===============>>sws_scale  h : %d ", h);
                     if (h > 0){
                         //锁住窗口全部区域
                         ANativeWindow_lock(aNativeWindow,&aNativeWindow_buffer,NULL);
                         LOGE("===============>>222222222  aNativeWindow_buffer %p ",aNativeWindow_buffer);
                         uint8_t  *dst = (uint8_t *) aNativeWindow_buffer.bits;
                         //复制到显卡交换缓冲区当中进行交换
//                         memcpy(dst,rgb_buf,vWidth*vHeight*4); //一个像素点四个字节
                         memcpy(dst,rgb_buf,outWidth*outHeight*4); //一个像素点四个字节
                         //
                         ANativeWindow_unlockAndPost(aNativeWindow);
                     }
                }

            } else if (cc == avCodecContext_audio) { //音频流
                ret = avcodec_receive_frame(avCodecContext_audio, avFrame);
                if (ret != 0) {
                    LOGE("========>> audio avcodec recevice frame failed!");
                    break;
                }

                //音频重采样
                uint8_t *out[2] = {0};
                //pcm_buf 大小与rgb_buf相关
                out[0] = (uint8_t *) pcm_buf;
                int len = swr_convert(swrContext,
                                      out,
                                      avFrame->nb_samples,
                                      (const uint8_t **) avFrame->data,
                                      avFrame->nb_samples);
                LOGE("===============>>swr_convert len = %d  ", len);
            }
        }
    }
    avformat_close_input(&avFormatContext);
    env->ReleaseStringUTFChars(url_, url);
    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_pp_ffmpegtest_jni_Jni_openSlTest(JNIEnv *env, jclass type, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    env->ReleaseStringUTFChars(url_, url);
}