package com.wanglei.wplayer.audio;

import android.os.Environment;

import java.io.File;
import java.io.IOException;

/**
 * Created by wanglei55 on 2018/10/9.
 */

public class FileUtils {

    public static String filePath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "yin.wav";

    public static String createFilePath() {

        File file = new File(filePath);
        if (!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return filePath;
    }

}
