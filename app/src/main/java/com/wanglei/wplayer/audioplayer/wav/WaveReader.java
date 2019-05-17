package com.wanglei.wplayer.audioplayer.wav;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;


public class WaveReader {

    private DataInputStream dataInputStream;

    private WaveHeader waveHeader;

    public boolean open(String filePath) throws IOException {
        if (filePath == null) {
            return false;
        }
        if (dataInputStream != null) {
            closeFile();
        }
        dataInputStream = new DataInputStream(new FileInputStream(filePath));
        return readWaveHeader();
    }

    private boolean readWaveHeader() {
        waveHeader = new WaveHeader();
        try {
            return waveHeader.readHeader(dataInputStream);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return false;
    }

    public void closeFile() throws IOException {
        if (dataInputStream != null) {
            dataInputStream.close();
            dataInputStream = null;
        }
    }

    public int read(byte[] buffer, int off, int len) {
        if (dataInputStream == null || waveHeader == null) {
            return -1;
        }
        try {
            return dataInputStream.read(buffer, off, len);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return -1;
    }
}
