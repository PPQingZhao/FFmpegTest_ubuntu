package com.example.pp.opengl_es_shader_test;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.widget.FrameLayout;

import com.example.pp.opengl_es_shader_test.XPlay.ThreadPool;
import com.example.pp.opengl_es_shader_test.XPlay.XPlay;
import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = MainActivity.class.getSimpleName();
    private String SDCARDPATH = Environment.getExternalStorageDirectory().getAbsolutePath();
    private FrameLayout xplay;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        xplay = findViewById(R.id.xplay);
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
        XPlay xPlay = ThreadPool.obtain(getApplicationContext());
        this.xplay.addView(xPlay);
    }

}
