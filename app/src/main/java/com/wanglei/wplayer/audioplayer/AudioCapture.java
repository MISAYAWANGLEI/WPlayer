package com.wanglei.wplayer.audioplayer;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

public class AudioCapture {
    private static final String TAG = AudioCapture.class.getSimpleName();

    private AudioRecord mAudioRecord;

    public static final int DEFAULT_SAMPLE_RATE = 44100; //默认采样率
    public static final int DEFAULT_CHANNEL = AudioFormat.CHANNEL_IN_MONO; //默认单通道
    public static final int SIMPLE_FORMAT = AudioFormat.ENCODING_PCM_16BIT; //16位量化
    private static final int DEFAULT_SOURCE_MIC = MediaRecorder.AudioSource.MIC; //声音从麦克风采集而来

    private volatile boolean isExit = true;
    private volatile boolean isStart = false;
    //外部使用回调接口接收数据。为false的话, 外部需要显示调用readData来获得音频原始数据
    private volatile boolean isUsingCallBack = true;//默认true，通过回调返回

    private Thread captureThread;

    private OnAudioCaptureListener listener;

    public AudioCapture() {
    }

    public void setOnAudioCaptureListener(OnAudioCaptureListener listener) {
        this.listener = listener;
    }

    public void setUsingCallBack(boolean usingCallBack) {
        isUsingCallBack = usingCallBack;
    }

    public boolean start() {
        return start(DEFAULT_SOURCE_MIC, DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL, SIMPLE_FORMAT);
    }

    public boolean start(int audioSource, int simpleRate, int channels, int audioFormat) {
        if (isStart) {
            Log.d(TAG, "start: it is already start capturing");
            return false;
        }
        mAudioRecord = createAudioRecord(audioSource, simpleRate, channels, audioFormat); //初始化AudioRecord
        if (mAudioRecord == null) {
            Log.d(TAG, "createAudioRecord failed");
            return false;
        }
        mAudioRecord.startRecording(); //开始录制
        isExit = false;
        //开辟线程, 从 AudioRecord 中的缓冲区将音频数据读出来
        captureThread = new Thread(new AudioCaptureRunnable());
        captureThread.start();
        isStart = true;
        Log.d(TAG, "start: start successfully");
        return true;
    }

    public boolean stop() {
        if (!isStart) {
            return false;
        }
        isExit = true;
        try {
            captureThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        if (mAudioRecord.getState() == AudioRecord.RECORDSTATE_RECORDING) {
            mAudioRecord.stop(); // 不是必须调用的, 因为release内部也会调用
        }
        mAudioRecord.release(); //
        mAudioRecord = null;
        isStart = false;
        Log.d(TAG, "stop: stop successfully");
        return true;
    }

    private class AudioCaptureRunnable implements Runnable {

        @Override
        public void run() {
            while (!isExit && isUsingCallBack) {
                byte[] buffer = new byte[1024 * 2]; //每次拿2k
                int result = mAudioRecord.read(buffer, 0, buffer.length);
                if (result == AudioRecord.ERROR_BAD_VALUE) {
                    Log.d(TAG, "run: ERROR_BAD_VALUE");
                } else if (result == AudioRecord.ERROR_INVALID_OPERATION) {
                    Log.d(TAG, "run: ERROR_INVALID_OPERATION");
                } else {
                    if (listener != null) {
                        Log.d(TAG, "run: capture buffer length is " + result);
                        listener.onAudioFrameCaptured(buffer);
                    }
                }
            }
        }
    }

    //采集到的数据通过回调接口传到外部
    public interface OnAudioCaptureListener {
        void onAudioFrameCaptured(byte[] bytes);
    }

    private AudioRecord createAudioRecord(int audioSource, int simpleRate, int channels, int audioFormat) {
        //获取一帧音频帧的大小：调用系统提供的方法即可
        int minBufferSize = AudioRecord.getMinBufferSize(simpleRate, channels, audioFormat);
        if (minBufferSize == AudioRecord.ERROR_BAD_VALUE) {
            Log.d(TAG, "获取音频帧大小失败!");
            return null;
        }
        int audioRecordBufferSize = minBufferSize * 4; //AudioRecord内部缓冲设置为4帧音频帧的大小句
        AudioRecord audioRecord = new AudioRecord(audioSource, simpleRate, channels, audioFormat, audioRecordBufferSize);
        if (audioRecord.getState() == AudioRecord.STATE_UNINITIALIZED) {
            Log.d(TAG, "初始化AudioRecord失败!");
            return null;
        }
        return audioRecord;
    }

    public int readData(byte[] buffer, int off, int len) {
        if (isUsingCallBack) {
            throw new IllegalStateException("in callback mode, please fetch pcm data via call back!");
        }
        if (buffer != null) {
            int ret = mAudioRecord.read(buffer, off, len);
            if (ret == AudioRecord.ERROR_BAD_VALUE) {
                Log.d(TAG, "run: ERROR_BAD_VALUE");
            } else if (ret == AudioRecord.ERROR_INVALID_OPERATION) {
                Log.d(TAG, "run: ERROR_INVALID_OPERATION");
            }
            return ret;
        }
        return -1;
    }
}
