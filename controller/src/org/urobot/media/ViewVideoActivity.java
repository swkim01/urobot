package org.urobot.media;

import java.util.ArrayList;

import org.urobot.R;

import android.app.Activity;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class ViewVideoActivity extends Activity implements SurfaceHolder.Callback {
	private SurfaceView mPreview;
	private SurfaceHolder mHolder;
	MediaPlayer mPlayer;
	private ArrayList<String> mPlayList = null;
	private ArrayList<String> mUriList = null;
	private int mCurrentIndex = -1;
	Button mPlayBtn;

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.playvideo);

		mPreview = (SurfaceView) findViewById(R.id.surface);
		mHolder = mPreview.getHolder();
		mHolder.addCallback(this);
		mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		
		Bundle bundle = getIntent().getExtras().getBundle("playlist");
		mPlayList = bundle.getStringArrayList("list");
		mUriList = bundle.getStringArrayList("urilist");
		mCurrentIndex = bundle.getInt("index");
		
		if (mPlayList == null || mCurrentIndex < 0
				|| mCurrentIndex >= mPlayList.size()) {

			Toast.makeText(this, "재생할 파일이 없습니다.", Toast.LENGTH_LONG).show();
			finish();
			return;
		}

		String uri = mPlayList.get(mCurrentIndex);

		mPlayBtn = (Button)findViewById(R.id.video_play);
		mPlayBtn.setOnClickListener(mClickPlay);
		findViewById(R.id.video_stop).setOnClickListener(mClickStop);
	}

	Button.OnClickListener mClickPlay = new View.OnClickListener() {
		public void onClick(View v) {
			if (mPlayer.isPlaying() == false) {
				mPlayer.start();
				mPlayBtn.setText("Pause");
			} else {
				mPlayer.pause();
				mPlayBtn.setText("Play");
			}
		}
	};

	Button.OnClickListener mClickStop = new View.OnClickListener() {
		public void onClick(View v) {
			mPlayer.stop();
			try {
				mPlayer.prepare();
			} catch (Exception e) {;}
		}
	};

	public void surfaceChanged(SurfaceHolder surfaceholder, int i, int j, int k) {
	}

	public void surfaceDestroyed(SurfaceHolder surfaceholder) {
	}

	public void surfaceCreated(SurfaceHolder holder) {
		if (mPlayer == null) {
			mPlayer = new MediaPlayer();
		} else {
			mPlayer.reset();
		}
		try {
			//mPlayer.setDataSource("/sdcard/testvideo.mp4");
			mPlayer.setDataSource(mUriList.get(mCurrentIndex));
			mPlayer.setDisplay(holder);
			mPlayer.prepare();
			mPlayer.setOnCompletionListener(mComplete);
			mPlayer.setOnVideoSizeChangedListener(mSizeChange);
		} catch (Exception e) {
			Toast.makeText(ViewVideoActivity.this, "error : " + e.getMessage(), 
					Toast.LENGTH_LONG).show();
		}
	}

	MediaPlayer.OnCompletionListener mComplete = 
		new MediaPlayer.OnCompletionListener() {
		public void onCompletion(MediaPlayer arg0) {
			mPlayBtn.setText("Play");
		}
	};

	MediaPlayer.OnVideoSizeChangedListener mSizeChange = 
		new MediaPlayer.OnVideoSizeChangedListener() {
		public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
		}
	};

	protected void onDestroy() {
		super.onDestroy();
		if (mPlayer != null) {
			mPlayer.release();
		}
	}
}

