package com.example.pp.ffmpegtest;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.example.pp.ffmpegtest.XPlay.XPlay;
import com.example.pp.ffmpegtest.jni.Jni;
import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();
    private TextView sample_text;
    /*-->　ctrl + shift + U 大小写快捷键*/
    private String SDCARDPATH = Environment.getExternalStorageDirectory().getAbsolutePath();
    private FrameLayout videoViewer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        sample_text = (TextView) findViewById(R.id.sample_text);
        videoViewer = findViewById(R.id.videoViewer);
        requestPermissionList();
    }

    private void requestPermissionList() {
        new RxPermissions(this).
                requestEachCombined(Manifest.permission.WRITE_EXTERNAL_STORAGE).subscribe(new Consumer<Permission>() {
            @Override
            public void accept(Permission permission) throws Exception {

                if (permission.granted) {  //全部权限授权成功
                    init();
                } else if (permission.shouldShowRequestPermissionRationale) { //至少有一个权限未授权 并且该权限未勾选不再询问
//                    showDialogWithoutNeverAgain();
                } else {    //至少有一个权限未授权 并且已勾选不再询问
//                    showDialogWithSetting();
                }
            }
        });
    }

    private void init() {
        sample_text.setText(Jni.stringFromJNI());
//        jni.jiefengzhuang(SDCARDPATH + File.separator+"2018_10_08_14_34_41_447_1.264");
//        jni.jiefengzhuang(SDCARDPATH + File.separator+"ffmpeg/VID_20181015_164136.mp4");
//        jni.jiefengzhuang(SDCARDPATH + File.separator+"VID_20180214_164359.mp4");
//        jni.avdeCode(SDCARDPATH + File.separator+"ffmpeg/VID_20181015_164136.mp4");
//        Jni.pixAndSizeChange(SDCARDPATH + File.separator+"ffmpeg/VID_20181015_164136.mp4");
//        XPlay xPlay = ThreadPool.obtain(getApplicationContext());
        XPlay xPlay = new XPlay(this);
        videoViewer.addView(xPlay);
    }

}
