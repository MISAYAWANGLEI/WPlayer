package com.wanglei.wplayer.audioplayer;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.support.annotation.NonNull;

import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;


public class AACEncoder implements AudioCapture.OnAudioCaptureListener {

    public static final int DEFAULT_BIT_RATE = 128 * 1024; //128kb

    public static final int DEFAULT_SIMPLE_RATE = 44100; //44100Hz

    public static final int DEFAULT_CHANNEL_COUNTS = 1;

    public static final int DEFAULT_MAX_INPUT_SIZE = 16384; //16k

    private MediaCodec mediaCodec;

    private MediaFormat mediaFormat;

    private OnAACEncodeListener onAACEncodeListener;

    private boolean isAsyMode = true;

    private volatile boolean isStart = false;

    private Thread encoderThread;

    private DataOutputStream dataOutputStream;

    private AudioCapture mAudioCapture;

    public AACEncoder() {
        mAudioCapture = new AudioCapture();
        mAudioCapture.setOnAudioCaptureListener(this);
    }

    public void setDataOutputStream(DataOutputStream dataOutputStream) {
        this.dataOutputStream = dataOutputStream;
    }

    public void setOnAACEncodeListener(OnAACEncodeListener onAACEncodeListener) {
        this.onAACEncodeListener = onAACEncodeListener;
    }

    public void setAsyMode(boolean asyMode) {
        isAsyMode = asyMode;
        if (isAsyMode) {
            mAudioCapture.setUsingCallBack(false);
        }
    }

    private MediaFormat createMediaFormat() {
        MediaFormat mediaFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC,
                DEFAULT_SIMPLE_RATE, DEFAULT_CHANNEL_COUNTS);
        mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, DEFAULT_BIT_RATE);
        mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, DEFAULT_MAX_INPUT_SIZE);
        return mediaFormat;
    }

    private void configure() {
        mediaCodec = createMediaCodec();
        if (mediaCodec == null) {
            throw new IllegalStateException("该设备不支持AAC编码器");
        }
        if (isAsyMode) {
            mediaCodec.setCallback(new AsyEncodeCallback());
            mAudioCapture.setUsingCallBack(false);
        } else {
            encoderThread = new Thread(new SynchronousEncodeRunnable());
        }
        mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
    }

    private MediaCodec createMediaCodec() {
        mediaFormat = createMediaFormat();
        MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.ALL_CODECS);
        String name = mediaCodecList.findEncoderForFormat(mediaFormat);
        if (name != null) {
            try {
                return MediaCodec.createByCodecName(name);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    public void start() {
        configure();
        mediaCodec.start();
        mAudioCapture.start();
        isStart = true;
        if (!isAsyMode) {
            encoderThread.start();
        }
    }

    @Override
    public void onAudioFrameCaptured(byte[] bytes) {
        encode(mediaCodec, bytes);
    }

    private class AsyEncodeCallback extends MediaCodec.Callback {

        @Override
        public void onInputBufferAvailable(@NonNull MediaCodec codec, int index) {
            if (!isStart) {
                try {
                    innerStop();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                return;
            }
            byte[] buf = new byte[2 * 1024];
            int ret = mAudioCapture.readData(buf, 0, buf.length);
            if (ret > 0) {
                ByteBuffer byteBuffer = codec.getInputBuffer(index);
                byteBuffer.clear();
                byteBuffer.put(buf);
                mediaCodec.queueInputBuffer(index, 0, buf.length, System.nanoTime() / 1000,
                        0);
            }
        }

        @Override
        public void onOutputBufferAvailable(@NonNull MediaCodec codec, int index, @NonNull MediaCodec.BufferInfo info) {
            if (!isStart) {
                try {
                    innerStop();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                return;
            }
            ByteBuffer byteBuffer = mediaCodec.getOutputBuffer(index);
            byteBuffer.position(info.offset);
            byteBuffer.limit(info.offset + info.size);
            byte[] frame = new byte[info.size];
            byteBuffer.get(frame, 0, info.size);
            if (onAACEncodeListener != null) {
                onAACEncodeListener.onEncodedFrame(frame);
            }
            writeToFile(frame);
            mediaCodec.releaseOutputBuffer(index, false);
        }

        @Override
        public void onError(@NonNull MediaCodec codec, @NonNull MediaCodec.CodecException e) {

        }

        @Override
        public void onOutputFormatChanged(@NonNull MediaCodec codec, @NonNull MediaFormat format) {

        }
    }

    private class SynchronousEncodeRunnable implements Runnable {

        @Override
        public void run() {
            while (isStart) {
                queryEncodedData();
            }
            try {
                innerStop();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public synchronized void encode(MediaCodec mediaCodec, byte[] data) {
        if (!isStart) {
            return;
        }
        if (data != null) {
            int bufferIndexId = mediaCodec.dequeueInputBuffer(10000);
            if (bufferIndexId >= 0) {
                ByteBuffer inputBuffer = mediaCodec.getInputBuffer(bufferIndexId);
                inputBuffer.clear();
                inputBuffer.put(data);
                mediaCodec.queueInputBuffer(bufferIndexId, 0, data.length, System.nanoTime() / 1000,
                        0);
            }
        }
    }

    private synchronized void queryEncodedData() {
        if (mediaCodec == null) {
            return;
        }
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndexId = mediaCodec.dequeueOutputBuffer(bufferInfo, 10000);
        if (outputBufferIndexId >= 0) {
            ByteBuffer byteBuffer = mediaCodec.getOutputBuffer(outputBufferIndexId);
            byteBuffer.position(bufferInfo.offset);
            byteBuffer.limit(bufferInfo.offset + bufferInfo.size);
            byte[] frame = new byte[bufferInfo.size];
            byteBuffer.get(frame, 0, bufferInfo.size);
            if (onAACEncodeListener != null) {
                onAACEncodeListener.onEncodedFrame(frame);
            }
            writeToFile(frame);
            mediaCodec.releaseOutputBuffer(outputBufferIndexId, false);
        }
    }

    public void stop() {
        if (!isStart) {
            return;
        }
        isStart = false;
    }

    public interface OnAACEncodeListener {
        void onEncodedFrame(byte[] data);
    }

    private void writeToFile(byte[] frame) {
        byte[] packetWithADTS = new byte[frame.length + 7];
        System.arraycopy(frame, 0, packetWithADTS, 7, frame.length);
        addADTStoPacket(packetWithADTS, packetWithADTS.length);
        if (dataOutputStream != null) {
            try {
                dataOutputStream.write(packetWithADTS, 0, packetWithADTS.length);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void addADTStoPacket(byte[] packet, int packetLen) {
        int profile = 2;  //AAC LC，MediaCodecInfo.CodecProfileLevel.AACObjectLC;
        int freqIdx = 4;  //44100, 见后面注释avpriv_mpeg4audio_sample_rates中32000对应的数组下标，来自ffmpeg源码
        int chanCfg = DEFAULT_CHANNEL_COUNTS;  //见后面注释channel_configuration，Stero双声道立体声

        /*int avpriv_mpeg4audio_sample_rates[] = {
            96000, 88200, 64000, 48000, 44100, 32000,
                    24000, 22050, 16000, 12000, 11025, 8000, 7350
        };
        channel_configuration: 表示声道数chanCfg
        0: Defined in AOT Specifc Config
        1: 1 channel: front-center
        2: 2 channels: front-left, front-right
        3: 3 channels: front-center, front-left, front-right
        4: 4 channels: front-center, front-left, front-right, back-center
        5: 5 channels: front-center, front-left, front-right, back-left, back-right
        6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
        7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
        8-15: Reserved
        */
        // fill in ADTS data
        packet[0] = (byte) 0xFF;
        packet[1] = (byte) 0xF9;
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private void innerStop() throws IOException {
        if (mediaCodec != null) {
            mediaCodec.stop();
            mediaCodec.release();
            mediaCodec = null;
        }
        if (dataOutputStream != null) {
            dataOutputStream.close();
            dataOutputStream = null;
        }
        if (mAudioCapture != null) {
            mAudioCapture.stop();
            mAudioCapture = null;
        }
    }
}
