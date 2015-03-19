package org.urobot.controller;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.teleal.cling.android.AndroidUpnpService;
import org.teleal.cling.controlpoint.ActionCallback;
import org.teleal.cling.controlpoint.SubscriptionCallback;
import org.teleal.cling.model.action.ActionArgumentValue;
import org.teleal.cling.model.action.ActionInvocation;
import org.teleal.cling.model.gena.CancelReason;
import org.teleal.cling.model.gena.GENASubscription;
import org.teleal.cling.model.message.UpnpResponse;
import org.teleal.cling.model.meta.Action;
import org.teleal.cling.model.meta.Device;
import org.teleal.cling.model.meta.Service;
import org.teleal.cling.model.state.StateVariableValue;
import org.teleal.cling.model.types.Datatype;
import org.teleal.cling.model.types.Datatype.Builtin;
import org.teleal.cling.model.types.UDN;
import org.urobot.R;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.speech.RecognizerIntent;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TableLayout;
import android.widget.Toast;
import android.widget.ToggleButton;

public class RobotActivity extends Activity {
	
	public static String DEVICE_UUID = "device_uuid";
	
	private AndroidUpnpService upnpService;
	
	private String uuid;
	
	private Device device;
	private Service motionControlService;
	private Service sensorMonitoringService;
	private Service carSensorMonitoringService;
	private Service cameraMonitoringService;
	private Service streamingService;
	
	private String presentationUrl;
	
	protected Handler mHandler = new Handler();

	int acc1Value, acc2Value, acc3Value, adc1Value, adc2Value, adc3Value, adc4Value;
	boolean cam_start=false;
	
	private boolean subscribe;
	
	private CamView cam;
	private ProgressBar acc1, acc2, acc3, adc1, adc2, adc3, adc4;
	private JoystickView joystick;
	private TableLayout lightButtons, keyButtons;
	private LinearLayout sensorMonitor;
	
	private static final int CAMERA_TYPE_STREAM = 0;
	private static final int CAMERA_TYPE_STILL = 1;
	private static final int INPUT_KEYBUTTON_MODE = 0;
	private static final int INPUT_JOYSTICK_MODE = 1;
	
	private static final int VOICE_RECOGNITION_REQUEST_CODE = 1234;
	
	private static final String TAG = "URobotController";
	
	private boolean statusFrontLight = false;
	private boolean statusBackLight = false;
	private boolean statusDirLight = false;
	private boolean statusWarningLight = false;
	private boolean statusLeftLight = false;
	private boolean statusRightLight = false;
	
	private int input_mode = INPUT_KEYBUTTON_MODE;
	
	private long beforeRunningTime = 0;
	private long afterRunningTime = 0;
	
	private ServiceConnection serviceConnection = new ServiceConnection() {
		
		public void onServiceConnected(ComponentName className, IBinder service) {
			upnpService = (AndroidUpnpService) service;
			
			device = upnpService.getRegistry().getDevice(UDN.valueOf(uuid), true);
			
			sensorMonitoringService = device.findService(new URobotServiceId("SensorMonitoring"));
			if (sensorMonitoringService == null)
				sensorMonitor.setVisibility(View.INVISIBLE);
			carSensorMonitoringService = device.findService(new URobotServiceId("CarSensorMonitoring"));
			if (carSensorMonitoringService == null)
				lightButtons.setVisibility(View.INVISIBLE);
			cameraMonitoringService = device.findService(new URobotServiceId("CameraMonitoring"));
			streamingService = device.findService(new URobotServiceId("Streaming"));
			if (streamingService == null)
				cam.setVisibility(View.INVISIBLE);
			motionControlService = device.findService(new URobotServiceId("MotionController"));
			if (motionControlService == null)
				keyButtons.setVisibility(View.INVISIBLE);
			
			presentationUrl = device.getDetails().getPresentationURI().toASCIIString();
			if (presentationUrl.charAt(0) == '/')
				presentationUrl = presentationUrl.substring(1);
			presentationUrl = device.getDetails().getBaseURL() + presentationUrl; 
			
			//Toast t = Toast.makeText(getApplicationContext(), "presentation url is "+ presentationUrl, Toast.LENGTH_SHORT);
			
		}
		
		public void onServiceDisconnected(ComponentName className) {
			upnpService = null;
		}
	};
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		
		uuid = getIntent().getStringExtra(DEVICE_UUID);
		
		getApplicationContext().bindService(
				new Intent(this, BrowserUpnpService.class),
				serviceConnection,
				Context.BIND_AUTO_CREATE
		);
		
		setContentView(R.layout.robot);
		
		sensorMonitor = (LinearLayout)findViewById(R.id.SensorMonitor);
		final ToggleButton SensorButton = (ToggleButton)this.findViewById(R.id.SensorButton);
		acc1 = (ProgressBar)findViewById(R.id.acc1);
		acc2 = (ProgressBar)findViewById(R.id.acc2);
		acc3 = (ProgressBar)findViewById(R.id.acc3);
		adc1 = (ProgressBar)findViewById(R.id.adc1);
		adc2 = (ProgressBar)findViewById(R.id.adc2);
		adc3 = (ProgressBar)findViewById(R.id.adc3);
		adc4 = (ProgressBar)findViewById(R.id.adc4);
		
		cam = (CamView) this.findViewById(R.id.Camview);
		
		lightButtons = (TableLayout)this.findViewById(R.id.lightButtontable);

		final ImageButton FrontLight = (ImageButton)this.findViewById(R.id.FrontLight);
		final ImageButton FogLight = (ImageButton)this.findViewById(R.id.FogLight);
		final ImageButton BreakLight = (ImageButton)this.findViewById(R.id.BreakLight);
		final ImageButton EmergencyLight = (ImageButton)this.findViewById(R.id.EmergencyLight);
		final ImageButton LeftLight = (ImageButton)this.findViewById(R.id.LeftLight);
		final ImageButton RightLight = (ImageButton)this.findViewById(R.id.RightLight);
		
		ImageButton TurnLeft = (ImageButton)this.findViewById(R.id.TurnLeft);
		ImageButton GoForward = (ImageButton)this.findViewById(R.id.GoForward);
		ImageButton TurnRight = (ImageButton)this.findViewById(R.id.TurnRight);
		ImageButton Stop = (ImageButton)this.findViewById(R.id.Stop);
		ImageButton GoBackward = (ImageButton)this.findViewById(R.id.GoBackward);
		keyButtons = (TableLayout) findViewById(R.id.keyButtontable);
		joystick = (JoystickView) findViewById(R.id.joystickView);
		joystick.setOnJoystickMovedListener(_listener);
		
		//Thread Start
		mainProcessing(); 					//ProgressBar Display Thread
		
		SensorButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (SensorButton.isChecked() && subscribe == false) {
					subscribe = true;
					sensorMonitoringSubscribe();
				} else {
					subscribe = false;
					sensorMonitoringUnsubscribe();
				}
			}
		});
		
		cam.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if(cam_start == false){
					Toast t = Toast.makeText(getApplicationContext(), "Camera Image Loading...", Toast.LENGTH_SHORT);
					t.setGravity(CONTEXT_IGNORE_SECURITY, 530, 1);
					t.show();
					String url = doGetVideoURL(0, CAMERA_TYPE_STREAM, 0);
					cam.init(getApplicationContext());
					cam.setSource(MultipartJpegInputStream.read(url/*"http://192.168.0.5:8080/?action=stream"*/));
					cam.setDisplayMode(CamView.SIZE_BEST_FIT);
					cam.showFps(true);
					cam_start = true;
				}
				else{
					Toast t = Toast.makeText(getApplicationContext(), "Camera Image Pausel!!", Toast.LENGTH_SHORT);
					t.setGravity(CONTEXT_IGNORE_SECURITY, 520, 1);
					t.show();
					 // in onPause
					cam.stopPlayback();
					cam_start = false;
				}
			}
		});
		
		FrontLight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doSetLightAction("SetFrontLight", 1);
				
				if(!statusFrontLight){
					FrontLight.setImageResource(R.drawable.btn_front_light1);
					statusFrontLight = true;
				}
				else{
					FrontLight.setImageResource(R.drawable.btn_front_light2);
					statusFrontLight = false;
				}
			}
		});
		
		FogLight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doSetLightAction("SetFogLight", 1);
				
				if(!statusDirLight){
					FogLight.setImageResource(R.drawable.btn_dir_light1);
					statusDirLight = true;
				}
				else{
					FogLight.setImageResource(R.drawable.btn_dir_light2);
					statusDirLight = false;
				}
			}
		});
		
		BreakLight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doSetLightAction("SetBreakLight", 1);
				
				if(!statusBackLight){
					BreakLight.setImageResource(R.drawable.btn_back_light1);
					statusBackLight = true;
				}
				else{
					BreakLight.setImageResource(R.drawable.btn_back_light2);
					statusBackLight = false;
				}
			}
		});
		
		EmergencyLight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doSetLightAction("SetWinkerLight", 0);
				
				if(!statusWarningLight){
					EmergencyLight.setImageResource(R.drawable.btn_warning_light1);
					statusWarningLight = true;
				}
				else{
					EmergencyLight.setImageResource(R.drawable.btn_warning_light2);
					statusWarningLight = false; 
				}
			}
		});
		
		LeftLight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doSetLightAction("SetWinkerLight", 1);
				
				if(!statusLeftLight){
					LeftLight.setImageResource(R.drawable.btn_left_light1);
					statusLeftLight = true;
				}
				else{
					LeftLight.setImageResource(R.drawable.btn_left_light2);
					statusLeftLight = false;
				}
			}
		});
		
		RightLight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doSetLightAction("SetWinkerLight", 2);
				
				if(!statusRightLight){
					RightLight.setImageResource(R.drawable.btn_right_light1);
					statusRightLight = true;
				}
				else{
					RightLight.setImageResource(R.drawable.btn_right_light2);
					statusRightLight = false;
				}
				
			}
		});
		
		TurnLeft.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doMotionAction("TurnLeft", 1);
			}
		});
		
		GoForward.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doMotionAction("GoForward", 1);
			}
		});
		
		TurnRight.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doMotionAction("TurnRight", 1);
			}
		});
		
		GoBackward.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doMotionAction("GoBackward", 1);
			}
		});
		
		Stop.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				doMotionAction("Stop", 1);
			}
		});
		
		// Get display items for later interaction
		Button speakButton = (Button) findViewById(R.id.SpeakButton);
		
		// Check to see if a recognition activity is present
		PackageManager pm = getPackageManager();
		List<ResolveInfo> activities = pm.queryIntentActivities(
				new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH), 0);
		if (activities.size() != 0) {
			speakButton.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					startVoiceRecognitionActivity();
				}
			});
		} else {
			speakButton.setEnabled(false);
			speakButton.setText("Recognizer not present");
		}
	}
	
	private JoystickMovedListener _listener = new JoystickMovedListener() {
		
		@Override
		public void OnMoved(float radial, float angle) {
			doMoveAction("Move", radial, angle);
		}
		
		@Override
		public void OnReleased() {
		}
		
		public void OnReturnedToCenter() {
		};
	};
	
	public void doMotionAction(String name, int value) {
		Action setMotionControlAction = motionControlService.getAction(name);
		ActionInvocation setMotionControlInvocation = new ActionInvocation(setMotionControlAction);
		
		setMotionControlInvocation.setInput("Command", value);

		//new ActionCallback.Default(setMotionControlInvocation, upnpService.getControlPoint()).run();
		// boolean value = ((Boolean) setMotionControlInvocation.getOutput("Status").getValue()).booleanValue();
		
		ActionCallback setMotionControlCallback = new ActionCallback(setMotionControlInvocation) {
			
			@Override
			public void success(ActionInvocation invocation) {
				ActionArgumentValue[] output = invocation.getOutput();
				afterRunningTime = System.currentTimeMillis();
				//assertEquals(output.length, 0);
			}
			
			@Override
			public void failure(ActionInvocation invocation, UpnpResponse operation, String defaultMsg) {
				System.err.println(defaultMsg);
			}
		};
	
		beforeRunningTime = System.currentTimeMillis();
		upnpService.getControlPoint().execute(setMotionControlCallback);
	}
	
	public void doMoveAction(String name, float radial, float angle) {
		Action setMoveControlAction = motionControlService.getAction(name);
		ActionInvocation setMoveControlInvocation = new ActionInvocation(setMoveControlAction);
		
		setMoveControlInvocation.setInput("Velocity", radial+"");
		setMoveControlInvocation.setInput("Angular", angle+"");

		//new ActionCallback.Default(setMoveControlInvocation, upnpService.getControlPoint()).run();
		//String result = ((String) setMoveControlInvocation.getOutput("Status").getValue()).toString();
		
		ActionCallback setMoveControlCallback = new ActionCallback(setMoveControlInvocation) {
			
			@Override
			public void success(ActionInvocation invocation) {
				ActionArgumentValue[] output = invocation.getOutput();
				afterRunningTime = System.currentTimeMillis();
				//assertEquals(output.length, 0);
			}
			
			@Override
			public void failure(ActionInvocation invocation, UpnpResponse operation, String defaultMsg) {
				System.err.println(defaultMsg);
			}
		};
	
		beforeRunningTime = System.currentTimeMillis();
		upnpService.getControlPoint().execute(setMoveControlCallback);
		
	}
	
	public void doSetLightAction(String name, int value) {
		Action setLightAction = sensorMonitoringService.getAction(name);
		ActionInvocation setLightInvocation = new ActionInvocation(setLightAction);
		
		setLightInvocation.setInput("Command", value);
		
		beforeRunningTime = System.currentTimeMillis();
		new ActionCallback.Default(setLightInvocation, upnpService.getControlPoint()).run();
		String result = ((String) setLightInvocation.getOutput("Status").getValue()).toString();
		afterRunningTime = System.currentTimeMillis();
		
		/*
		ActionCallback setLightCallback = new ActionCallback(setLightInvocation) {
			
			@Override
			public void success(ActionInvocation invocation) {
				ActionArgumentValue[] output = invocation.getOutput();
				//assertEquals(output.length, 0);
			}
			
			@Override
			public void failure(ActionInvocation invocation, UpnpResponse operation, String defaultMsg) {
				System.err.println(defaultMsg);
			}
		};
		upnpService.getControlPoint().execute(setLightCallback);
		*/
	}
	
	SubscriptionCallback callback;
	String id;
	Long event;
	//String kkk;
	//float jjj;
	public void sensorMonitoringSubscribe() {
		callback = new SubscriptionCallback(sensorMonitoringService, 600/*sec*/) {
			
			@Override
			public void established(GENASubscription sub) {
				id = sub.getSubscriptionId();
				mHandler.post(new Runnable(){
					public void run() {
						Toast.makeText(getApplicationContext(), "Subscription Established: "+id,Toast.LENGTH_SHORT).show();
					}
				});
				System.out.println("Established: " + sub.getSubscriptionId());
			}
			
			@Override
			protected void failed(GENASubscription subscription,
					UpnpResponse responseStatus,
					Exception exception,
					String defaultMsg) {
				System.err.println(defaultMsg);
			}
			
			@Override
			public void ended(GENASubscription sub,
					CancelReason reason,
					UpnpResponse response) {
				mHandler.post(new Runnable(){
					public void run() {
						Toast.makeText(getApplicationContext(), "Subscription Ended: "+id,Toast.LENGTH_SHORT).show();
					}
				});
				assert reason == null;
				assert sub != null;  // DOC: EXC3
				assert response == null;
			}
			
			@Override
			public void eventReceived(GENASubscription sub) {
				
				event = sub.getCurrentSequence().getValue();
				/*
				mHandler.post(new Runnable(){
					public void run() {
						Toast.makeText(getApplicationContext(), "Event: "+ event,Toast.LENGTH_SHORT).show();
					}
				});
				*/

				Log.d(TAG, "Event: " + sub.getCurrentSequence().getValue());
				
				//Map<String, StateVariableValue> values = sub.getCurrentValues();
				//StateVariableValue sensorValue = values.get("SensorValue");
				
				List<StateVariableValue> values = new ArrayList();
				for (Map.Entry<String, StateVariableValue> entry :
					((Map<String, StateVariableValue>) sub.getCurrentValues()).entrySet()) {
					values.add(entry.getValue());
				}
				//controller.getStateVariablesTable().getValuesModel().setValues(values);
				
				// assertEquals(status.getDatatype().getClass(), BooleanDatatype.class);
				// assertEquals(status.getDatatype().getBuiltin(), Datatype.Builtin.BOOLEAN);
				
				int i=0;
				for (StateVariableValue svv : values) {
					/*
					if (i++ == 4) {
						kkk = svv.getStateVariable().getName();
						jjj = Float.parseFloat(svv.getValue().toString());
						mHandler.post(new Runnable(){
							public void run() {
								Toast.makeText(getApplicationContext(), "name: "+ kkk + ", value="+jjj,Toast.LENGTH_SHORT).show();
							}
						});
					}
					*/
					
					if (Datatype.Builtin.isNumeric(svv.getDatatype().getBuiltin()) ||
							svv.getDatatype().getBuiltin().equals(Builtin.R4) ||
							svv.getDatatype().getBuiltin().equals(Builtin.R8)) {
						String name = svv.getStateVariable().getName();
						float value = Float.parseFloat(svv.getValue().toString());
						if (name.equals("FrontDistance")) {
							adc1Value = (int) value;
						}
						else if (name.equals("BackDistance")) {
							adc2Value = (int)value;
						}
						else if (name.equals("LeftDistance")) {
							adc3Value = (int)value;
						}
						else if (name.equals("RightDistance")) {
							adc4Value = (int)value;
						}
						else if (name.equals("AcceleratorX")) {
							acc1Value = (int)value;
						}
						else if (name.equals("AcceleratorY")) {
							acc2Value = (int)value;
						}
						else if (name.equals("AcceleratorZ")) {
							acc3Value = (int)value;
						}
					}
				}
				
				mHandler.post(doUpdateGUI);
				
				// Log.d(TAG, "Sensor Value is: " + sensorValue.toString());
				
				if (sub.getCurrentSequence().getValue() == 0) {
					// assertEquals(sub.getCurrentValues().get("Status").toString(), "0");
				} else if (sub.getCurrentSequence().getValue() == 1) {
					// assertEquals(sub.getCurrentValues().get("Status").toString(), "1");
				} else {
				}
			}
			
			@Override
			public void eventsMissed(GENASubscription sub, int numberOfMissedEvents) {
				Log.d(TAG, "Missed events: " + numberOfMissedEvents);
			}
		};
		
		upnpService.getControlPoint().execute(callback);
	}
	
	public void sensorMonitoringUnsubscribe() {
		callback.end();
	}
	
	public String doGetVideoURL(int index, int type, int id) {
		Action getVideoUrlAction = streamingService.getAction("GetVideoURL");
		ActionInvocation getVideoUrlInvocation = new ActionInvocation(getVideoUrlAction);
		
		getVideoUrlInvocation.setInput("CameraIndex", index);
		getVideoUrlInvocation.setInput("CameraType", type);
		getVideoUrlInvocation.setInput("ObjectID", id);
		
		new ActionCallback.Default(getVideoUrlInvocation, upnpService.getControlPoint()).run();
		String result = ((String) getVideoUrlInvocation.getOutput("VideoURL").getValue()).toString();
		
		/*
		ActionCallback setLightCallback = new ActionCallback(setLightInvocation) {
			
			@Override
			public void success(ActionInvocation invocation) {
				ActionArgumentValue[] output = invocation.getOutput();
				//assertEquals(output.length, 0);
			}
			
			@Override
			public void failure(ActionInvocation invocation, UpnpResponse operation, String defaultMsg) {
				System.err.println(defaultMsg);
			}
		};
		upnpService.getControlPoint().execute(setLightCallback);
		*/
		return result;
	}
	
	// ProcessBar Thread : Sensor Value Display
	private void mainProcessing(){
		Thread thread = new Thread(null,doBackgoundThreadprocessing,"Background");
		thread.start();
	}
	
	private Runnable doBackgoundThreadprocessing = new Runnable(){
		public void run() {
			backgroundThreadProcessing();
		}
	};
	
	private void backgroundThreadProcessing() {
		while(true){
			mHandler.post(doUpdateGUI);
			/*
			try {
				Thread.sleep(100, 0);
			}
			catch(Exception e) {}
			*/
   		}
  	}

	private Runnable doUpdateGUI = new Runnable(){
		public void run() {
			updateGUI();
		}
	};
	
	//Sensor Bar Display
	private void updateGUI(){
		acc1.setProgress(acc1Value);
		acc2.setProgress(acc2Value);
		acc3.setProgress(acc3Value);
		adc1.setProgress(adc1Value);
		adc2.setProgress(adc2Value);
		adc3.setProgress(adc3Value);
		adc4.setProgress(adc4Value);
	}
	
	/**
	 * Fire an intent to start the speech recognition activity.
	 */
	private void startVoiceRecognitionActivity() {
		Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
		intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
				RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
		intent.putExtra(RecognizerIntent.EXTRA_PROMPT, getText(R.string.voice_recognizer_prompt));
		startActivityForResult(intent, VOICE_RECOGNITION_REQUEST_CODE);
	}
	
	/**
	 * Handle the results from the recognition activity.
	 */
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == VOICE_RECOGNITION_REQUEST_CODE && resultCode == RESULT_OK) {
			// Fill the list view with the strings the recognizer thought it could have heard
			ArrayList<String> matches = data.getStringArrayListExtra(
					RecognizerIntent.EXTRA_RESULTS);
			for (String str : matches) {
				if (str.matches(getString(R.string.go_forward_prompt))) {
					doMotionAction("GoForward", 1);
					break;
				}
				else if (str.matches(getString(R.string.go_backward_prompt))) {
					doMotionAction("GoBackward", 1);
					break;
				}
				else if (str.matches(getString(R.string.turn_left_prompt))) {
					doMotionAction("TurnLeft", 1);
					break;
				}
				else if (str.matches(getString(R.string.turn_right_prompt))) {
					doMotionAction("TurnRight", 1);
					break;
				}
				else if (str.matches(getString(R.string.stop_prompt))) {
					doMotionAction("Stop", 1);
					break;
				}
			}
		}
		super.onActivityResult(requestCode, resultCode, data);
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(0, 0, 0, R.string.view_presentation_url);
		menu.add(0, 1, 0, R.string.toggle_command_method);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case 0:
			Intent i = new Intent(Intent.ACTION_VIEW);
			Uri u = Uri.parse(presentationUrl);
			i.setData(u);
			startActivity(i);
			break;
		case 1:
			if (motionControlService != null) {
				if (input_mode == INPUT_KEYBUTTON_MODE) {
					input_mode = INPUT_JOYSTICK_MODE;
					Toast.makeText(this, R.string.switching_joystick_mode, Toast.LENGTH_SHORT).show();
					keyButtons.setVisibility(View.GONE);
					joystick.setVisibility(View.VISIBLE);
				} else {
					input_mode = INPUT_KEYBUTTON_MODE;
					Toast.makeText(this, R.string.switching_keybutton_mode, Toast.LENGTH_SHORT).show();
					joystick.setVisibility(View.GONE);
					keyButtons.setVisibility(View.VISIBLE);
				}
			}
			break;
		}
		return false;
	}
	
	protected void onPause() {
		Log.d("TAG", "onPause");
		super.onPause();
	}
	protected void onStop() {
		Log.d("TAG", "onStop");
		super.onStop();
	}
	protected void onDestroy() {
		Log.d("TAG", "onDestroy");
		super.onDestroy();
		//System.exit(0);
	}
} 
