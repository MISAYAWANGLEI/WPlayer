package com.wanglei.wplayer.audioplayer.wav;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;


public class WaveHelper {

    public static byte[] int2Bytes(int intValue) {
        return ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(intValue).array();
    }

    public static byte[] short2Bytes(short shortValue) {
        return ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN).putShort(shortValue).array();
    }

    public static int bytes2Int(byte[] bytes) {
        return ByteBuffer.wrap(bytes).getInt();
    }

    public static short bytes2Short(byte[] bytes) {
        return ByteBuffer.wrap(bytes).getShort();
    }

    public static String bytes2String(byte[] bytes, Charset charset) {
        return new String(bytes, charset);
    }

    public static String bytes2String(byte[] bytes) {
        return bytes2String(bytes, Charset.forName("UTF-8"));
    }
}
