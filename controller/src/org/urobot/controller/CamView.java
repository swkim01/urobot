package org.urobot.controller;



import java.io.IOException;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
 

public class CamView extends SurfaceView implements SurfaceHolder.Callback {
    public final static int POSITION_UPPER_LEFT  = 9;
    public final static int POSITION_UPPER_RIGHT = 3;
    public final static int POSITION_LOWER_LEFT  = 12;
    public final static int POSITION_LOWER_RIGHT = 6;
 
    public final static int SIZE_STANDARD   = 1;
    public final static int SIZE_BEST_FIT   = 4;
    public final static int SIZE_FULLSCREEN = 8;
   
    private MultipartJpegViewThread thread;
    private MultipartJpegInputStream mIn = null;    
    private boolean showFps = false;
    private boolean mRun = false;
    private boolean surfaceDone = false;    
    private Paint overlayPaint;
    private int overlayTextColor;
    private int overlayBackgroundColor;
    private int ovlPos;
    private int dispWidth;
    private int dispHeight;
    private int displayMode;
 
    public class MultipartJpegViewThread extends Thread {
        private SurfaceHolder mSurfaceHolder;
        private int frameCounter = 0;
        private long start;
        private String fps = "";
        private Bitmap ovl;
         
        public MultipartJpegViewThread(SurfaceHolder surfaceHolder, Context context) {
            mSurfaceHolder = surfaceHolder;
        }
 
        private Rect destRect(int bmw, int bmh) {
            int tempx;
            int tempy;
            if (displayMode == CamView.SIZE_STANDARD) {
                tempx = (dispWidth / 2) - (bmw / 2);
                tempy = (dispHeight / 2) - (bmh / 2);
                return new Rect(tempx, tempy, bmw + tempx, bmh + tempy);
            }
            if (displayMode == CamView.SIZE_BEST_FIT) {
                float bmasp = (float) bmw / (float) bmh;
                bmw = dispWidth;
                bmh = (int) (dispWidth / bmasp);
                if (bmh > dispHeight) {
                    bmh = dispHeight;
                    bmw = (int) (dispHeight * bmasp);
                }
                tempx = (dispWidth / 2) - (bmw / 2);
                tempy = (dispHeight / 2) - (bmh / 2);
                return new Rect(tempx, tempy, bmw + tempx, bmh + tempy);
            }
            if (displayMode == CamView.SIZE_FULLSCREEN)
                return new Rect(0, 0, dispWidth, dispHeight);
            return null;
        }
         
        public void setSurfaceSize(int width, int height) {
            synchronized(mSurfaceHolder) {
                dispWidth = width;
                dispHeight = height;
            }
        }
         
        private Bitmap makeFpsOverlay(Paint p) {
            Rect b = new Rect();
            p.getTextBounds(fps, 0, fps.length(), b);
            
            // false indentation to fix forum layout
            Bitmap bm = Bitmap.createBitmap(b.width(), b.height(), Bitmap.Config.ARGB_8888);
 
            Canvas c = new Canvas(bm);
            p.setColor(overlayBackgroundColor);
            c.drawRect(0, 0, b.width(), b.height(), p);
            p.setColor(overlayTextColor);
            c.drawText(fps, -b.left, b.bottom-b.top-p.descent(), p);
            return bm;           
        }
 
        public void run() {
            start = System.currentTimeMillis();
            PorterDuffXfermode mode = new PorterDuffXfermode(PorterDuff.Mode.DST_OVER);
            Bitmap bm;
            int width;
            int height;
            Rect destRect;
            Canvas c = null;
            Paint p = new Paint();
            while (mRun) {
                if(surfaceDone) {
                    try {
                        c = mSurfaceHolder.lockCanvas();
                        synchronized (mSurfaceHolder) {
                            try {
                                bm = mIn.readMultipartJpegFrame();
                                destRect = destRect(bm.getWidth(),bm.getHeight());
                                c.drawColor(Color.BLACK);
                                c.drawBitmap(bm, null, destRect, p);
                                if(showFps) {
                                    p.setXfermode(mode);
                                    if(ovl != null) {
                                    	
                                    	// false indentation to fix forum layout
                                    	height = ((ovlPos & 1) == 1) ? destRect.top : destRect.bottom-ovl.getHeight();
                                    	width  = ((ovlPos & 8) == 8) ? destRect.left : destRect.right -ovl.getWidth();
                                        c.drawBitmap(ovl, width, height, null);
                                    }
                                    p.setXfermode(null);
                                    frameCounter++;
                                    if((System.currentTimeMillis() - start) >= 1000) {
                                        fps = String.valueOf(frameCounter)+"fps";
                                        frameCounter = 0;
                                        start = System.currentTimeMillis();
                                        ovl = makeFpsOverlay(overlayPaint);
                                    }
                                }
                            } catch (IOException e) {}
                        }
                    } finally { if (c != null) mSurfaceHolder.unlockCanvasAndPost(c); }
                }
            }
        }
    }
 
    private void init(Context context) {
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
        thread = new MultipartJpegViewThread(holder, context);
        setFocusable(true);
        overlayPaint = new Paint();
        overlayPaint.setTextAlign(Paint.Align.LEFT);
        overlayPaint.setTextSize(12);
        overlayPaint.setTypeface(Typeface.DEFAULT);
        overlayTextColor = Color.WHITE;
        overlayBackgroundColor = Color.BLACK;
        ovlPos = CamView.POSITION_LOWER_RIGHT;
        displayMode = CamView.SIZE_STANDARD;
        dispWidth = getWidth();
        dispHeight = getHeight();
    }
   
    public void startPlayback() {
        if(mIn != null) {
            mRun = true;
            thread.start();            
        }
    }
   
    public void stopPlayback() {
        mRun = false;
        boolean retry = true;
        while(retry) {
            try {
                thread.join();
                retry = false;
            } catch (InterruptedException e) {}
        }
    }
 
    public CamView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }
 
    public void surfaceChanged(SurfaceHolder holder, int f, int w, int h) {
        thread.setSurfaceSize(w, h);
    }
 
    public void surfaceDestroyed(SurfaceHolder holder) {
        surfaceDone = false;
        stopPlayback();
    }
    public CamView(Context context) {
    	super(context);
    	init(context);
    }    
    public void surfaceCreated(SurfaceHolder holder) {
    	surfaceDone = true;
    }
    public void showFps(boolean b) {
    	showFps = b;
    }
    public void setSource(MultipartJpegInputStream source) {
    	mIn = source;
    	startPlayback();
    }
    public void setOverlayPaint(Paint p) { overlayPaint = p; }
    public void setOverlayTextColor(int c) { overlayTextColor = c; }
    public void setOverlayBackgroundColor(int c) { overlayBackgroundColor = c; }
    public void setOverlayPosition(int p) { ovlPos = p; }
    public void setDisplayMode(int s) { displayMode = s; }

}
