package com.example.pp.ffmpegtest.jni;

/**
 * Created by qing on 18-10-8.
 */

public class Jni {
    static{
        System.loadLibrary("native-lib");
    }

    public native String stringFromJNI();

    /**
     * @param url 路径
     * @param handle　窗口句柄
     * @return
     */
    public native boolean open(String url,Object handle);

    public native void jiefengzhuang(String url);

}
