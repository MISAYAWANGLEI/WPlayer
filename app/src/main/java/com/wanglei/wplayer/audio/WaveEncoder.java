package com.wanglei.wplayer.audio;


import com.wanglei.wplayer.audio.wav.WaveWriter;

import java.io.FileNotFoundException;

/**
 * 录制音频并保存为wav文件
 */
public class WaveEncoder implements AudioCapture.OnAudioCaptureListener {

    private WaveWriter mWaveWriter;
    private AudioCapture mAudioCapture;

    public WaveEncoder() {
        mWaveWriter = new WaveWriter();
        mAudioCapture = new AudioCapture();
        mAudioCapture.setOnAudioCaptureListener(this);
    }

    //打开文件并写入头信息
    public boolean prepare(String filePath, int sampleRate, short numChannels, short bitsPerSample) throws FileNotFoundException {
        return mWaveWriter.open(filePath, sampleRate, numChannels, bitsPerSample);
    }

    public boolean prepare(String filePath) throws FileNotFoundException {
        return prepare(filePath, AudioCapture.DEFAULT_SAMPLE_RATE, (short)1, (short)16);
    }

    //开始录制音频，在回调中将获取的数据写入文件
    public boolean start() {
        return mAudioCapture.start();
    }

    private boolean writeData(byte[] data, int off, int len) {
        return mWaveWriter.writeData(data, off, len);
    }

    //停止录制
    public boolean stop() {
        boolean audioStop = mAudioCapture.stop();
        boolean waveWriterStop = mWaveWriter.closeFile();
        return waveWriterStop && audioStop;
    }

    @Override
    public void onAudioFrameCaptured(byte[] bytes) {
        writeData(bytes, 0, bytes.length);
    }
}
