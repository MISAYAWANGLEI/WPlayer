package com.wanglei.wplayer.audioplayer;


import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import com.wanglei.wplayer.R;
import java.io.FileNotFoundException;

public class AudioRecordActivity extends AppCompatActivity {

    private AudioPlayer audioPlayer;
    private WaveEncoder waveEncoder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_record);
        audioPlayer = new AudioPlayer();
        waveEncoder = new WaveEncoder();
        try {
            waveEncoder.prepare(FileUtils.createFilePath());
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    public void start(View view){
        if (audioPlayer!=null){
            audioPlayer.start();
        }
    }

    public void stop(View view){
        if (audioPlayer!=null){
            audioPlayer.stop();
        }
    }

    public void wavstart(View view){
        if (waveEncoder!=null){
            waveEncoder.start();
        }
    }

    public void wavstop(View view){
        if (waveEncoder!=null){
            waveEncoder.stop();
        }
    }

}
