package org.urobot.media;

import java.io.IOException;
import java.util.ArrayList;

import org.urobot.R;

import android.app.Activity;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

public class PlayAudioActivity extends Activity {
	MediaPlayer mPlayer;
	private ArrayList<String> mPlayList = null;
	private ArrayList<String> mUriList = null;
	private int mCurrentIndex = -1;
	ImageButton mPlayBtn;
	TextView mFileName;
	SeekBar mProgress;
	boolean wasPlaying;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.playaudio);
		
		Bundle bundle = getIntent().getExtras().getBundle("playlist");
		mPlayList = bundle.getStringArrayList("list");
		mUriList = bundle.getStringArrayList("urilist");
		mCurrentIndex = bundle.getInt("index");

		if (mPlayList == null || mCurrentIndex < 0
				|| mCurrentIndex >= mPlayList.size()) {

			Toast.makeText(this, R.string.no_more_file_for_playing, Toast.LENGTH_LONG).show();
			finish();
			return;
		}

		String uri = mPlayList.get(mCurrentIndex);
		mPlayer = new MediaPlayer();

		// Register Button Click Listeners
		mFileName = (TextView)findViewById(R.id.filename);
		mPlayBtn = (ImageButton)findViewById(R.id.play);
		mPlayBtn.setOnClickListener(mClickPlay);
		findViewById(R.id.stop).setOnClickListener(mClickStop);
		findViewById(R.id.prev).setOnClickListener(mClickPrevNext);
		findViewById(R.id.next).setOnClickListener(mClickPrevNext);
		
		// Register CompletionListener, SeekCompleteListener, and SeekChangeListener
		mPlayer.setOnCompletionListener(mOnComplete);
		mPlayer.setOnSeekCompleteListener(mOnSeekComplete);
		mProgress = (SeekBar)findViewById(R.id.progress);
		mProgress.setOnSeekBarChangeListener(mOnSeek);
		mProgressHandler.sendEmptyMessageDelayed(0,200);
		
		// ready for first song
		if (LoadMedia(mCurrentIndex) == false) {
			Toast.makeText(this, R.string.cannot_read_file, Toast.LENGTH_LONG).show();
			finish();
		}
	}

	// finish playing when activity is finished
	public void onDestroy() {
		super.onDestroy();
		if (mPlayer != null) {
			mPlayer.release();
			mPlayer = null;
		}
	}

	// LoadMedia
	boolean LoadMedia(int idx) {
		try {
			mPlayer.setDataSource(mUriList.get(idx));
		} catch (IllegalArgumentException e) {
			return false;
		} catch (IllegalStateException e) {
			return false;
		} catch (IOException e) {
			return false;
		}
		if (Prepare() == false) {
			return false;
		}
		mFileName.setText(mPlayList.get(idx));
		mProgress.setMax(mPlayer.getDuration());
		return true;
    }
    
    boolean Prepare() {
		try {
			mPlayer.prepare();
		} catch (IllegalStateException e) {
			return false;
		} catch (IOException e) {
			return false;
		}
		return true;
	}

	// Play and Pause
	Button.OnClickListener mClickPlay = new View.OnClickListener() {
		public void onClick(View v) {
			if (mPlayer.isPlaying() == false) {
				mPlayer.start();
				mPlayBtn.setImageResource(android.R.drawable.ic_media_pause);		
			} else {
				mPlayer.pause();
				mPlayBtn.setImageResource(android.R.drawable.ic_media_play);		
			}
		}
	};

	// Stop. ready for restart.
	Button.OnClickListener mClickStop = new View.OnClickListener() {
		public void onClick(View v) {
			mPlayer.stop();
			mPlayBtn.setImageResource(android.R.drawable.ic_media_play);		
			mProgress.setProgress(0);
			Prepare();
		}
	};
	
	Button.OnClickListener mClickPrevNext = new View.OnClickListener() {
		public void onClick(View v) {
			boolean wasPlaying = mPlayer.isPlaying();
			
			if (v.getId() == R.id.prev) {
				mCurrentIndex = (mCurrentIndex == 0 ? mPlayList.size() - 1 : mCurrentIndex - 1);
			} else {
				mCurrentIndex = (mCurrentIndex == mPlayList.size() - 1 ? 0 : mCurrentIndex + 1);
			}
			
			mPlayer.reset();
			LoadMedia(mCurrentIndex);

			// play next song
			if (wasPlaying) {
				mPlayer.start();
				mPlayBtn.setImageResource(android.R.drawable.ic_media_pause);		
			}
		}
	};

	// next song when playing finished
	MediaPlayer.OnCompletionListener mOnComplete = new MediaPlayer.OnCompletionListener() {
		public void onCompletion(MediaPlayer arg0) {
			mCurrentIndex = (mCurrentIndex == mPlayList.size() - 1 ? 0 : mCurrentIndex + 1);
			mPlayer.reset();
			LoadMedia(mCurrentIndex);
			mPlayer.start();
		}
	};

	// output message on error
	MediaPlayer.OnErrorListener mOnError = new MediaPlayer.OnErrorListener() {
		public boolean onError(MediaPlayer mp, int what, int extra) {
			String err = "OnError occured. what = " + what + " ,extra = " + extra;
			Toast.makeText(PlayAudioActivity.this, err, Toast.LENGTH_LONG).show();
			return false;
		}
	};

	// process seek completed
	MediaPlayer.OnSeekCompleteListener mOnSeekComplete = new MediaPlayer.OnSeekCompleteListener() {
		public void onSeekComplete(MediaPlayer mp) {
			if (wasPlaying) {
				mPlayer.start();
			}
		}
	};

	// update playing position in 0.2 sec interval
	Handler mProgressHandler = new Handler() {
		public void handleMessage(Message msg) {
			if (mPlayer == null) return;
			if (mPlayer.isPlaying()) {
				mProgress.setProgress(mPlayer.getCurrentPosition());
			}
			mProgressHandler.sendEmptyMessageDelayed(0,200);
		}
	};

	// move playing position
	SeekBar.OnSeekBarChangeListener mOnSeek = new SeekBar.OnSeekBarChangeListener() {
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			if (fromUser) {
				mPlayer.seekTo(progress);
			}
		}

		public void onStartTrackingTouch(SeekBar seekBar) {
			wasPlaying = mPlayer.isPlaying();
			if (wasPlaying) {
				mPlayer.pause();
			}
		}

		public void onStopTrackingTouch(SeekBar seekBar) {
		}
	};
}
