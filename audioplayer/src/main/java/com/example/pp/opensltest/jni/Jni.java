package com.example.pp.opensltest.jni;

/**
 * Created by qing on 18-10-8.
 */

public class Jni {
    static {
        System.loadLibrary("native-lib");
    }

    //使用opensl播放音频
    public native static void openSlTest(String url);
}
