package com.example.pp.ffmpegtest;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.widget.TextView;

import com.example.pp.ffmpegtest.jni.Jni;
import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();
    private TextView sample_text;
    private Jni jni;
    /*-->　ctrl + shift + U 大小写快捷键*/
    private String SDCARDPATH = Environment.getExternalStorageDirectory().getAbsolutePath();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        sample_text = (TextView) findViewById(R.id.sample_text);
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
        jni = new Jni();
        jni.jiefengzhuang(SDCARDPATH + File.separator+"2018_10_08_14_34_41_447_1.264");
//        jni.jiefengzhuang(SDCARDPATH + File.separator+"VID_20180214_164359.mp4");
        sample_text.setText(jni.stringFromJNI());
    }

}
