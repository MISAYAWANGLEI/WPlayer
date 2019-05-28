package com.wanglei.wplayer;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

public class WPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }
    private String dataSource;
    private SurfaceHolder holder;
    private OnPrepareListener listener;
    private OnErrorListener onErrorListener;
    /**
     * 设置播放的文件或者直播地址
     */
    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 设置播放显示的画布
     * @param surfaceView
     */
    public void setSurfaceView(SurfaceView surfaceView) {
        if (null != holder){
            holder.removeCallback(this);
        }
        holder = surfaceView.getHolder();
        holder.addCallback(this);
    }

    public void onError(int errorCode){
        stop();
        if (onErrorListener!=null){
            onErrorListener.onError(errorCode);
        }
    }

    public int getDuration() {
        return native_getDuration();
    }

    public void seek(final int progress) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                native_seek(progress);
            }
        }).start();
    }

    public interface OnErrorListener {
        void onError(int error);
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    private OnProgressListener onProgressListener;

    /**
     * native回调java层回传播放进度
     * @param progress
     */
    public void onProgress(int progress) {
        if (null != onProgressListener) {
            onProgressListener.onProgress(progress);
        }
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public void onPrepare(){
        if (null != listener){
            listener.onPrepare();
        }
    }

    public void setOnPrepareListener(OnPrepareListener listener){
        this.listener = listener;
    }
    public interface OnPrepareListener{
        void onPrepare();
    }

    /**
     * 准备
     */
    public void prepare() {
        native_prepare(dataSource);
    }

    /**
     * 播放
     */
    public void start() {
        native_start();
    }

    /**
     * 停止播放
     */
    public void stop() {
        native_stop();
    }

    public void release() {
        holder.removeCallback(this);
        native_release();
    }

    /**
     * 画布创建好了
     *
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布发生了变化（横竖屏切换、按了home都会回调这个函数）
     *
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i("WFFMPEG","surfaceChanged");
        native_setSurface(holder.getSurface());
    }

    /**
     * 销毁画布 (按了home/退出应用/)
     *
     * @param holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }


    native void native_prepare(String dataSource);
    native void native_start();
    native void native_stop();
    native void native_release();
    native void native_setSurface(Surface surface);
    native int native_getDuration();
    native void native_seek(int progress);
}
