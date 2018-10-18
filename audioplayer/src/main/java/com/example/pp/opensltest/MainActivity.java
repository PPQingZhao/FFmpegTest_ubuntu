package com.example.pp.opensltest;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;

import com.example.pp.opensltest.jni.Jni;
import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {
    private String SDCARDPATH = Environment.getExternalStorageDirectory().getAbsolutePath();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestPermissionList();
    }

    private void requestPermissionList() {
        new RxPermissions(this).
                requestEachCombined(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .subscribe(new Consumer<Permission>() {
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
        Jni.openSlTest(SDCARDPATH + File.separator+"ffmpeg/test.pcm");
    }

}
