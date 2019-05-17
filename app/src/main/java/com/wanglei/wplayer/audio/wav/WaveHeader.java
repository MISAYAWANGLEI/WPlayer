package com.wanglei.wplayer.audio.wav;

import android.util.Log;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

public class WaveHeader {

    private static final String TAG = WaveHeader.class.getSimpleName();

    public static final long RIFF_CHUNK_SIZE_OFFSET = 4;
    public static final long DATA_CHUNK_SIZE_OFFSET = 40;
    public static final int CHUNKSIZE_EXCLUDE_DATA = 36;

    /**
     * RIFF块字段
     */
    private String riffChunkId = "RIFF";
    private int riffChunkSize;
    private String format = "WAVE";

    /**
     * fmt块字段
     */
    private String fmtChunkId = "fmt ";//后面跟一个空格，使其占4字节
    private int fmtChunkSize;
    private short audioFormat = 1;
    private short numberChannels;
    private int sampleRate;
    private int byteRate;
    private short blockAlign;
    private short bitsPerSample;

    /**
     * data块字段
     */
    private String dataChunkId = "data";
    private int dataChunkSize;


    public WaveHeader() {
    }

    public WaveHeader(int sampleRate, short numChannels, short bitsPerSample) {
        this.sampleRate = sampleRate;
        this.numberChannels = numChannels;
        this.bitsPerSample = bitsPerSample;
        //计算1S的音频数据占用的字节数 计算公式：SampleRate * NumChannels * BitsPerSample/8
        byteRate = numberChannels * this.sampleRate * this.bitsPerSample / 8;
        //计算一个音频采样点占用的字节数 计算公式：NumChannels * BitsPerSample/8
        blockAlign = (short) (this.numberChannels * this.bitsPerSample / 8);
        fmtChunkSize = 16;
    }

    public boolean writeHeader(DataOutputStream dataOutputStream) throws IOException {
        if (dataOutputStream != null) {
            //riff块写入
            dataOutputStream.writeBytes(riffChunkId);
            dataOutputStream.write(WaveHelper.int2Bytes(riffChunkSize), 0, 4);
            dataOutputStream.writeBytes(format);

            //fmt块写入
            dataOutputStream.writeBytes(fmtChunkId);
            dataOutputStream.write(WaveHelper.int2Bytes(fmtChunkSize), 0, 4);
            dataOutputStream.write(WaveHelper.short2Bytes(audioFormat), 0, 2);
            dataOutputStream.write(WaveHelper.short2Bytes(numberChannels), 0, 2);
            dataOutputStream.write(WaveHelper.int2Bytes(sampleRate), 0, 4);
            dataOutputStream.write(WaveHelper.int2Bytes(byteRate), 0, 4);
            dataOutputStream.write(WaveHelper.short2Bytes(blockAlign), 0, 2);
            dataOutputStream.write(WaveHelper.short2Bytes(bitsPerSample), 0, 2);

            //data数据块写入
            dataOutputStream.writeBytes(dataChunkId);
            dataOutputStream.write(WaveHelper.int2Bytes(dataChunkSize), 0, 4);
            return true;
        }
        return false;
    }

    public boolean readHeader(DataInputStream dataInputStream) throws IOException {
        byte[] byte4Buffer = new byte[4];
        byte[] byte2Buffer = new byte[2];

        int readBytes = 0;

        if (dataInputStream != null) {
            //riff块的读取
            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            riffChunkId = WaveHelper.bytes2String(byte4Buffer);
            Log.d(TAG, "readHeader: riffChunkId: " + riffChunkId);

            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            riffChunkSize = WaveHelper.bytes2Int(byte4Buffer);
            Log.d(TAG, "readHeader: riffChunkSize: " + riffChunkSize);

            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            format = WaveHelper.bytes2String(byte4Buffer);
            Log.d(TAG, "readHeader: format: " + format);

            //fmt块的读取
            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            fmtChunkId = WaveHelper.bytes2String(byte4Buffer);
            Log.d(TAG, "readHeader: fmtChunkId: " + fmtChunkId);

            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            fmtChunkSize = WaveHelper.bytes2Int(byte4Buffer);
            Log.d(TAG, "readHeader: fmtChunkSize: " + fmtChunkSize);

            readBytes = dataInputStream.read(byte2Buffer, 0, 2);
            if (!checkReadBytes(readBytes, 2)) {
                return false;
            }
            audioFormat = WaveHelper.bytes2Short(byte2Buffer);
            Log.d(TAG, "readHeader: audioFormat: " + audioFormat);

            readBytes = dataInputStream.read(byte2Buffer, 0, 2);
            if (!checkReadBytes(readBytes, 2)) {
                return false;
            }
            numberChannels = WaveHelper.bytes2Short(byte2Buffer);
            Log.d(TAG, "readHeader: numberChannels: " + numberChannels);

            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            sampleRate = WaveHelper.bytes2Int(byte4Buffer);
            Log.d(TAG, "readHeader: sampleRate: " + sampleRate);

            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            byteRate = WaveHelper.bytes2Int(byte4Buffer);
            Log.d(TAG, "readHeader: byteRate: " + byteRate);

            readBytes = dataInputStream.read(byte2Buffer, 0, 2);
            if (!checkReadBytes(readBytes, 2)) {
                return false;
            }
            blockAlign = WaveHelper.bytes2Short(byte2Buffer);
            Log.d(TAG, "readHeader: block align: " + blockAlign);

            readBytes = dataInputStream.read(byte2Buffer, 0, 2);
            if (!checkReadBytes(readBytes, 2)) {
                return false;
            }
            bitsPerSample = WaveHelper.bytes2Short(byte2Buffer);
            Log.d(TAG, "readHeader: bitsPerSample: " + bitsPerSample);

            //data块的读取
            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            dataChunkId = WaveHelper.bytes2String(byte4Buffer);
            Log.d(TAG, "readHeader: dataChunkId: " + dataChunkId);

            readBytes = dataInputStream.read(byte4Buffer, 0, 4);
            if (!checkReadBytes(readBytes, 4)) {
                return false;
            }
            dataChunkSize = WaveHelper.bytes2Int(byte4Buffer);
            Log.d(TAG, "readHeader: dataChunkSize: " + dataChunkSize);
            Log.d(TAG, "readHeader: finish");
            return true;
        }
        return false;
    }

    private boolean checkReadBytes(int nByte, int desNByte) {
        return nByte == desNByte;
    }


}
