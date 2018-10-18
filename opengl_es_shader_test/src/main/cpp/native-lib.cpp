#include <jni.h>
#include <android/native_window_jni.h>
#include "include/native-lib.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>

//顶点着色器glsl
#define GET_STR(x) #x
static const char *vertexShader = GET_STR(
        attribute vec4 aPosition;  //顶点坐标
        attribute vec2 aTexCoord;　//材质顶点坐标
        varying vec2 vTexCoord;　　//输出的材质坐标
        void main(){
        vTexCoord = vec2(aTexCoord.x, 1.0-aTexCoord.y);
        gl_Position = aPosition;
}
);

//片元着色器  软件码和部分x86硬解码　使用下面这种格式
static const char *fragYUV_420P = GET_STR(
        precision mediump float; //精度
        varying vec2 vTexCoord;　//顶点着色器传递的坐标
        uniform sampler2D yTexture;　　//输入的材质(不透明灰度，单像素)
        uniform sampler2D uTexture;　　//　
        uniform sampler2D vTexture;
        void main(){
        vec3 yuv;
        vec3 rgb;
        yuv.r = texture2D(yTexture,vTexCoord).r;
        yuv.g = texture2D(yTexture,vTexCoord).r - 0.5;
        yuv.b = texture2D(yTexture, vTexCoord).r - 0.5;
        //yuv 转换　rgb   一个矩阵 mat3
        rgb = mat3(1.0    , 1.0     , 1.0    ,
                   0.0    , -0.39465, 2.03211,
                   1.13983, -0.5806 , 0.0)*yuv;
        //输出像素颜色
        gl_FragColor = vec4(rgb,1.0);
}
);


//shader　初始化
GLint InitShader(const char *code, GLint type){
    GLint sh = glCreateShader(type);
    if (sh == 0){
        LOGE("====>> InitShader failed! %d",type);
        return 0;
    }
    //加载shader
    glShaderSource(sh,
                   1, //shader　数量
                   &code,//shader代码
                   //代码长度
                   0 );
    //编译shader
    glCompileShader(sh);

    //获取编译情况
    GLint status;
    glGetShaderiv(sh,GL_COMPILE_STATUS,&status);
    if (status == 0){
        LOGE("glGetShaderiv　failed!");
        return 0;
    }
    LOGE("glGetShaderiv　succee!");
    return sh;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_pp_opengl_1es_1shader_1test_jni_Jni_open(JNIEnv *env, jclass type, jstring url_,
                                                          jobject surface) {
    const char *url = env->GetStringUTFChars(url_, 0);
    //读取文件
    FILE *file = fopen(url,"rb");
    if (!file){
        LOGE("===========>>fopen failed!");
        return;
    }

    //①　获取原始窗口
    ANativeWindow *awin = ANativeWindow_fromSurface(env, surface);

    //EGL
    //① 创建初始化display
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOGE("===========>>eglGetDisplay failed! ");
        return;
    }

    if (EGL_TRUE != eglInitialize(eglDisplay, NULL, NULL)) {
        LOGE("==================>>eglInitialize failed! ");
        return;
    }

    //②
    //2-1　　surface窗口配置
    // eglChooseConfig (EGLDisplay dpy, const EGLint *attrib_list,
    // EGLConfig *config, EGLint config_size, EGLint *num_config);
    //输出配置项
    EGLConfig config = NULL;
    EGLint num_config = NULL;
    //输入配置项
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };
    if (EGL_TRUE != eglChooseConfig(eglDisplay, configSpec, &config, 1, &num_config)) {
        LOGE("=============>> eglChooseConfig failed!");
        return;
    }

    //2-2 创建 surface
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config, awin, NULL);
    if (eglSurface == EGL_NO_SURFACE) {
        LOGE("==============>>eglCreateWindowSurface failed! ");
        return;
    }

    //③ 创建上下文

    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttr);
    if (eglContext == EGL_NO_CONTEXT) {
        LOGE("==============>> eglCreateContext failed!");
        return;
    }

    if (EGL_TRUE != eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOGE("===============>>eglMakeCurrent failed! ");
        return;
    }

    //shader　初始化
    //顶点坐标初始化
    GLint vsh = InitShader(vertexShader,GL_VERTEX_SHADER);
    //片元初始化
    GLint fsh = InitShader(fragYUV_420P,GL_FRAGMENT_SHADER);

    //创建渲染程序
    GLint program = glCreateProgram();
    if(program == 0){
        LOGE("===========>>glCreateProgram failed! ");
        return;
    }
    //渲染程序中加入着色器代码
    glAttachShader(program,vsh);
    glAttachShader(program,fsh);
    //连接程序
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program,GL_LINK_STATUS,&status);
    if (status != GL_TRUE){
        LOGE("=========>> glLinkProgram failed!");
        return;
    }

    //激活渲染程序
    glUseProgram(program);
    LOGE("=======>>glUseProgram  success!");

    /////////////////////////////////////////////////////
    //加入三维顶点数据　两个三角形组成正方形
    static float ver[] = {
            1.0f,-1.f,0.0f,
            -1.0f,-1.0f,0.0f,
            1.0f,1.0f,0.0f,
            -1.0f,1.0f,0.0f
    };

    GLuint apos = glGetAttribLocation(program,"aPosition");
    glEnableVertexAttribArray(apos);
    //传递值
    glVertexAttribPointer(apos,3,GL_FLOAT,GL_FALSE,12,ver);

    //加入材质坐标
    static float txts[] = {
            1.0f,0.0f,
            0.0f,0.0f,
            1.0f,1.0f,
            0.0f,1.0f
    };

    GLuint  atex = glGetAttribLocation(program,"aTexCoord");
    glEnableVertexAttribArray(atex);
    glVertexAttribPointer(atex,2,GL_FLOAT,GL_FALSE,8,txts);

    //材质纹理初始化
    //设置纹理层
    glUniform1i(glGetUniformLocation(program,"yTexture"),0); //纹理层第一层
    glUniform1i(glGetUniformLocation(program,"uTexture"),1); //纹理层第二层
    glUniform1i(glGetUniformLocation(program,"vTexture"),2); //纹理层第三层

    //创建opengl纹理
    GLuint  texts[3]={0};
    glGenTextures(3,texts);//创建三个纹理

    int width = 424;
    int height = 240;
    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D,texts[0]);
    //缩小过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //放大过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //设置纹理格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本　0默认
                 GL_LUMINANCE,//gpu内部格式，灰度图
                 width, height,//尺寸是2的次方
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式　亮度，灰度图　要与上面一致
                 GL_UNSIGNED_BYTE,//像素的数据类型
                 NULL//纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D,texts[1]);
    //缩小过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //放大过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //设置纹理格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本　0默认
                 GL_LUMINANCE,//gpu内部格式，灰度图
                 width/2, height/2,//尺寸是2的次方
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式　亮度，灰度图　要与上面一致
                 GL_UNSIGNED_BYTE,//像素的数据类型
                 NULL//纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D,texts[2]);
    //缩小过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //放大过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //设置纹理格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本　0默认
                 GL_LUMINANCE,//gpu内部格式，灰度图
                 width/2, height/2,//尺寸是2的次方
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式　亮度，灰度图　要与上面一致
                 GL_UNSIGNED_BYTE,//像素的数据类型
                 NULL//纹理的数据
    );

    //////////////////////////////////////////////////////////////////////
    //纹理放修改和显示
    unsigned  char *buf[3] = {0};
    buf[0]=new unsigned char[width*height];
    buf[1]=new unsigned char[width*height/4];
    buf[2]=new unsigned char[width*height/4];

    for (int i = 0; i < 10000; i++) {
        /*memset(buf[0], i, width * height);
        memset(buf[1], i, width * height/4);
        memset(buf[2], i, width * height/4);*/

        //是否读到结尾  420P格式  yyyyyyyy uu vv
        if (feof(file) == 0){
            fread(buf[0],1,width*height,file);
            fread(buf[1],1,width*height/4,file);
            fread(buf[2],1,width*height/4,file);
        }

        glActiveTexture(GL_TEXTURE0);//激活第一层纹理
        glBindTexture(GL_TEXTURE_2D, texts[0]);//绑定到创建的opengl纹理
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);//替换纹理内容

        glActiveTexture(GL_TEXTURE1);//激活第二层纹理
        glBindTexture(GL_TEXTURE_2D, texts[1]);//绑定到创建的opengl纹理
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width/2, height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[1]);//替换纹理内容

        glActiveTexture(GL_TEXTURE2);//激活第三层纹理
        glBindTexture(GL_TEXTURE_2D, texts[2]);//绑定到创建的opengl纹理
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width/2, height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[2]);//替换纹理内容

        //三角形绘制
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
        //窗口显示
        eglSwapBuffers(eglDisplay,eglSurface);
    }



    env->ReleaseStringUTFChars(url_, url);
}