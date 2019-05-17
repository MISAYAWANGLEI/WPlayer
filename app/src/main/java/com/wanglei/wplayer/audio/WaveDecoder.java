package com.wanglei.wplayer.audio;

import com.wanglei.wplayer.audio.wav.WaveReader;

import java.io.IOException;

/**
 * 播放wav格式音頻
 */
public class WaveDecoder{

    private static final int SAMPLES_PER_FRAME = 1024;

    private AudioPlayer mAudioPlayer;
    private WaveReader mWavFileReader;
    private volatile boolean mIsTestingExit = false;


    public boolean start() {
        mWavFileReader = new WaveReader();
        mAudioPlayer = new AudioPlayer();
        try {
            mWavFileReader.open(FileUtils.filePath);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        mAudioPlayer.startPlayer();
        new Thread(AudioPlayRunnable).start();
        return true;
    }

    public boolean stop() {
        mIsTestingExit = true;
        if (mAudioPlayer!=null
                &&mWavFileReader!=null){
            mAudioPlayer.stopPlayer();
            mWavFileReader.closeFile();
        }
        return true;
    }

    private Runnable AudioPlayRunnable = new Runnable() {
        @Override
        public void run() {
            byte[] buffer = new byte[SAMPLES_PER_FRAME * 2];
            while (!mIsTestingExit && mWavFileReader.read(buffer, 0, buffer.length) > 0) {
                mAudioPlayer.play(buffer, 0, buffer.length);
            }
            mAudioPlayer.stopPlayer();
            try {
                mWavFileReader.closeFile();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };
}
