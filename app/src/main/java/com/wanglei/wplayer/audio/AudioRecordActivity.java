package com.wanglei.wplayer.audio;


import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import com.wanglei.wplayer.R;
import java.io.FileNotFoundException;

public class AudioRecordActivity extends AppCompatActivity {

    private AudioTest audioPlayer;
    private WaveEncoder waveEncoder;
    private WaveDecoder waveDecoder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_record);
        audioPlayer = new AudioTest();
        waveEncoder = new WaveEncoder();
        waveDecoder = new WaveDecoder();
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

    public void wavplaystart(View view) {
        waveDecoder.start();
    }

    public void wavplaystop(View view) {
        waveDecoder.stop();
    }
}
