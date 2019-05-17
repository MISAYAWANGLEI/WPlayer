package com.wanglei.wplayer.audio;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

/**
 * 实时录制并播放
 */
public class AudioTest implements AudioCapture.OnAudioCaptureListener {

    private static final String TAG = "AudioTest";

    private static final int SIMPLE_RATE = 44100; //采样率
    private static final int FORMAT = AudioFormat.ENCODING_PCM_16BIT; //量化位宽
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_OUT_MONO; //单通道
    private static final int PLAY_TYPE = AudioManager.STREAM_MUSIC; //播放模式

    //AudioTrack 提供了两种播放模式，一种是 static 方式，一种是 streaming 方式，
    // 前者需要一次性将所有的数据都写入播放缓冲区，简单高效，
    // 通常用于播放铃声、系统提醒的音频片段;
    // 后者则是按照一定的时间间隔不间断地写入音频数据，
    // 理论上它可用于任何音频播放的场景
    private static final int PLAY_MODE = AudioTrack.MODE_STREAM;
    //使用相对底层的AudioTrack来播放
    private AudioTrack audioTrack;
    private AudioCapture mAudioCapture;
    private volatile boolean isStart = false;

    public AudioTest() {
        mAudioCapture = new AudioCapture();
        mAudioCapture.setOnAudioCaptureListener(this);
    }

    public boolean start() {
        boolean audioStart = mAudioCapture.start();
        boolean audioPlayerStart = start(PLAY_TYPE, SIMPLE_RATE, CHANNEL_CONFIG, FORMAT);
        return audioPlayerStart && audioStart;
    }

    public boolean start(int streamType, int sampleRate, int channelConfig, int format) {
        if (isStart) {
            return false;
        }
        audioTrack = createAudioTrack(streamType, sampleRate, channelConfig, format);
        if (audioTrack == null) {
            return false;
        }
        isStart = true;
        audioTrack.play();
        Log.d(TAG, "play: start");
        return true;
    }

    public boolean play(byte[] audioData, int offset, int size) {
        if (!isStart) {
            return false;
        }
        if (audioTrack.write(audioData, offset, size) != size) {
            Log.d(TAG, "play: size != write size");
        }
        return true;
    }

    public void stop() {
        if (!isStart) {
            return;
        }
        if (audioTrack.getState() == AudioTrack.PLAYSTATE_PLAYING) {
            audioTrack.stop();
        }
        isStart = false;
        mAudioCapture.stop();
        mAudioCapture = null;
        audioTrack.release();
        audioTrack = null;
    }

    @Override
    public void onAudioFrameCaptured(byte[] bytes) {
        play(bytes, 0, bytes.length);
    }

    private AudioTrack createAudioTrack(int streamType, int sampleRate,
                                        int channelConfig, int format) {
        int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelConfig, format);
        if (minBufferSize == AudioTrack.ERROR_BAD_VALUE) {
            return null;
        }
        int audioTrackBufferSize = minBufferSize * 4; //4帧音频帧的大小
        AudioTrack audioTrack = new AudioTrack(streamType, sampleRate, channelConfig, format,
                audioTrackBufferSize, PLAY_MODE);
        if (audioTrack.getState() == AudioTrack.STATE_UNINITIALIZED) {
            return null;
        }
        return audioTrack;
    }
}
