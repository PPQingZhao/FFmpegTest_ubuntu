package com.example.pp.ffmpegtest.jni;

/**
 * Created by qing on 18-10-8.
 */

public class Jni {
    static {
        System.loadLibrary("native-lib");
    }


    public static native String stringFromJNI();

    /**
     * @param url    路径
     * @param handle 　窗口句柄
     * @return
     */
    public native static boolean open(String url, Object handle);

    //解封装
    public native static void jiefengzhuang(String url);

    //解码器
    public native static void avdeCode(String url);

    //视频像素和尺寸转换
    public native static void pixAndSizeChange(String url);
}
