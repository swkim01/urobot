package org.urobot.media;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;

import org.urobot.R;

import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Html;
import android.view.Window;
import android.view.WindowManager;
import android.widget.MediaController;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

public class PlayVideoActivity extends Activity /*implements OnTouchListener*/ {
	private ArrayList<String> mPlayList = null;
	private ArrayList<String> mUriList = null;
	private int mCurrentIndex = -1;
	private VideoView video;
	private MediaController mc;
	private TextView subtitle;
	ArrayList<smiData> parsedSmi = new ArrayList<smiData>();
	private int countSmi;
	private boolean subFinishFlag = false;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		boolean isPortLayout = getResources().getBoolean(R.bool.isPortLayout);
		if(isPortLayout) {
		    // PORT
		} else {
		    // LAND
		    requestWindowFeature(Window.FEATURE_NO_TITLE);
		    getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}
		
		setContentView(R.layout.videoviewtest);
		
		video = (VideoView)findViewById(R.id.videoview);
		subtitle = (TextView)findViewById(R.id.subtitle);
		//video.setOnTouchListener(this);
		
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
		
		subtitle.setText("");

		mc = new MediaController(PlayVideoActivity.this);
		video.setMediaController(mc);
		
		//String uri = mUriList.get(mCurrentIndex);
		//video.setVideoPath(uri);
		//video.setVideoPath("/sdcard/testvideo.mp4");

		Uri uri = Uri.parse(mUriList.get(mCurrentIndex));
		video.setVideoURI(uri);
		//video.setVideoURI(Uri.parse("rtsp:"+uri.getEncodedSchemeSpecificPart()));
		
		video.postDelayed(new Runnable() {
			public void run() {
				mc.show(0);
			}
		},100);
		//video.start();
		
		// process subtitle file (.smi).
		String smiPath = uri.toString().substring(0,uri.toString().lastIndexOf(".")) + ".smi";
		URL smiUrl = null;
		try {
			smiUrl = new URL(smiPath);
		} catch (MalformedURLException e) {
		}
		if (smiUrl != null) {
			ArrayList<smiData> parsedSmi = parseSmi(smiUrl);
			if (parsedSmi != null) {
				new Thread(new Runnable() {
					public void run() {
						while(!subFinishFlag) {
							try {
								Thread.sleep(300);
								smiHandler.sendMessage(smiHandler.obtainMessage());
							} catch (Throwable t) {
								// Exit Thread
							}
						}
					}
				}).start();
			}
		}
	}
	
	/*
	@Override
	public boolean onTouch(View v, MotionEvent event) {
		int action = event.getAction();
		if (action == MotionEvent.ACTION_DOWN) {
			if (mc.getVisibility() != View.VISIBLE) {
				video.postDelayed(new Runnable() {
					public void run() {
						mc.show(0);
					}
				},100);
			}
			return true;
		}
		return false;
	}
	*/
	
	class smiData {
		long time;
		String text;
		smiData(long time, String text) {
			this.time = time;
			this.text = text;
		}
		public long gettime() {
			return time;
		}
		public String gettext() {
			return text;
		}
	}
	
	private ArrayList<smiData> parseSmi(URL url) {
		try {
			BufferedReader in = new BufferedReader(new InputStreamReader(url.openStream(),"MS949"));
			String s;
			long time = -1;
			String text = null;
			boolean smistart = false;
			
			while ((s = in.readLine()) != null) {
				if(s.contains("<SYNC")) {
					smistart = true;
					if(time != -1) {
						parsedSmi.add(new smiData(time, text));
					}
					time = Integer.parseInt(s.substring(s.indexOf("=")+1, s.indexOf(">")));
					text = s.substring(s.indexOf(">")+1, s.length());
				text = text.substring(text.indexOf(">")+1, text.length());
				} else {
					if(smistart == true) {
						text += s;
					}
				}
			}
			if(smistart == false) {
				in.close();
				return null;
			}
			in.close();
		} catch (IOException e) {
			System.err.println(e);
			//System.exit(1);
			return null;
		}
		return parsedSmi;
	}
	
	Handler smiHandler = new Handler() {
		public void handleMessage(Message msg) {
			countSmi = getSyncIndex(video.getCurrentPosition());
			if (countSmi >= 0)
				subtitle.setText(Html.fromHtml(parsedSmi.get(countSmi).gettext()));
			else
				subFinishFlag = true;
		}
	};
	
	public int getSyncIndex(long playTime) {
		int l=0,m,h=parsedSmi.size();
		
		while(l <= h) {
			m = (l+h)/2;
			if (m >= parsedSmi.size()-1)
				return -1;
			if(parsedSmi.get(m).gettime() <= playTime && playTime < parsedSmi.get(m+1).gettime()) {
				return m;
			}
			if(playTime > parsedSmi.get(m+1).gettime()) {
				l=m+1;
			} else {
				h=m-1;
			}
		}
		return 0;
	}
    
	@Override
	public void onSaveInstanceState(Bundle savedInstanceState) {
		super.onSaveInstanceState(savedInstanceState);
		//we use onSaveInstanceState in order to store the video playback position for orientation change
		savedInstanceState.putInt("Position", video.getCurrentPosition());
		video.pause();
	}
	
	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
		//we use onRestoreInstanceState in order to play the video playback from the stored position
		int position = savedInstanceState.getInt("Position");
		video.seekTo(position);
	}
}
