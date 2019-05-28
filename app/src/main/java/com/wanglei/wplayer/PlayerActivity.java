package com.wanglei.wplayer;

import android.content.res.Configuration;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

public class PlayerActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    private WPlayer player;
    private SeekBar seekBar;
    private int progress;
    private boolean isTouch;//是否正在拖动seekBar
    private boolean isSeek;
    private TextView progress_time;
    private TextView total_time;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        player = new WPlayer();
        player.setSurfaceView(surfaceView);
        //player.setDataSource("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8");
        player.setDataSource("/sdcard/3.mp4");
        seekBar = findViewById(R.id.seekBar);
        progress_time = findViewById(R.id.progress_time);
        total_time = findViewById(R.id.total_time);
        seekBar.setOnSeekBarChangeListener(this);
        player.setOnPrepareListener(new WPlayer.OnPrepareListener() {
            @Override
            public void onPrepare() {
                //获得视频时间
                final int duration = player.getDuration();//单位秒
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(PlayerActivity.this, "准备好了", Toast.LENGTH_SHORT).show();
                        //直播获取的时间为0，直播的时候进度条隐藏，直播拖拽也没用，玩穿越？
                        if (duration != 0) {
                            seekBar.setVisibility(View.VISIBLE);
                            progress_time.setVisibility(View.VISIBLE);
                            total_time.setVisibility(View.VISIBLE);
                            int hh  = duration / 3600;
                            int mm  = (duration % 3600) / 60;
                            int ss  = (duration % 60);
                            total_time.setText(hh+":"+mm+":"+ss);
                        }
                    }
                });
                player.start();
            }
        });

        player.setOnErrorListener(new WPlayer.OnErrorListener() {
            @Override
            public void onError(int errorCode) {
                Log.i("WFFMPEG", "Java接到回调:" + errorCode);
            }
        });

        player.setOnProgressListener(new WPlayer.OnProgressListener() {
            @Override
            public void onProgress(final int progress) {
                if (!isTouch) {//拖动时不更新
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            int duration = player.getDuration();
                            if (duration != 0) {
                                if (isSeek) {//更新的时候刚刚拖动过，这里就先不更新了，拖动完已经更新过
                                    isSeek = false;
                                    return;
                                }
                                //更新进度 计算比例
                                seekBar.setProgress(progress * 100 / duration);
                                int hh  = progress / 3600;
                                int mm  = (progress % 3600) / 60;
                                int ss  = (progress % 60);
                                progress_time.setText(hh+":"+mm+":"+ss);
                            }
                        }
                    });
                }
            }
        });
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager
                    .LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        setContentView(R.layout.activity_player);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        player.setSurfaceView(surfaceView);
        player.setDataSource("/sdcard/3.mp4");
        seekBar = findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        seekBar.setProgress(progress);
    }

    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }


    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {//开始拖动
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {//停止拖动
        isSeek = true;
        isTouch = false;
        progress = player.getDuration() * seekBar.getProgress() / 100;
        //进度调整
        player.seek(progress);
    }
}
