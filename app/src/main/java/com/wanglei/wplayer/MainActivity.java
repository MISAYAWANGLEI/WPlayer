package com.wanglei.wplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private WPlayer dnPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        dnPlayer = new WPlayer();
        dnPlayer.setSurfaceView(surfaceView);
        dnPlayer.setDataSource("rtmp://live.hkstv.hk.lxdns.com/live/hks");
        dnPlayer.setOnPrepareListener(new WPlayer.OnPrepareListener() {
            @Override
            public void onPrepare() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "准备好了", Toast.LENGTH_SHORT).show();
                    }
                });
            }
        });
    }

    public void start(View view) {

    }

}
