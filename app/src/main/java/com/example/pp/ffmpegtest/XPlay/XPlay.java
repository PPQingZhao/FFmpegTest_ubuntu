package com.example.pp.ffmpegtest.XPlay;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.util.AttributeSet;
import android.util.Log;

import com.example.pp.ffmpegtest.jni.Jni;

import java.io.File;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by qing on 18-10-16.
 */

public class XPlay extends GLSurfaceView implements Runnable{

    private static final String TAG = XPlay.class.getSimpleName();
    private GLRenderer mGLRender;

    public XPlay(Context context) {
        this(context, null);
    }

    public XPlay(Context context, AttributeSet attrs) {
        super(context, attrs);
        mGLRender = new GLRenderer();
        setRenderer(mGLRender);
    }

    @Override
    public void run() {
        Log.e(TAG,"===============>> run");
        Jni.open(SDCARDPATH + File.separator+"ffmpeg/VID_20181015_164136.mp4",getHolder().getSurface());
    }

        private String SDCARDPATH = Environment.getExternalStorageDirectory().getAbsolutePath();
    class GLRenderer implements GLSurfaceView.Renderer{
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            new Thread(XPlay.this).start();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {

        }

        @Override
        public void onDrawFrame(GL10 gl) {

        }
    }
}
