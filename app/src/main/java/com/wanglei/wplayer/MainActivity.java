package com.wanglei.wplayer;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.wanglei.wplayer.audioplayer.AudioRecordActivity;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);
    }

    public void toPlayer(View view) {
        Intent i = new Intent(this,PlayerActivity.class);
        startActivity(i);
    }

    public void toAudio(View view) {
        Intent i = new Intent(this, AudioRecordActivity.class);
        startActivity(i);
    }
}
