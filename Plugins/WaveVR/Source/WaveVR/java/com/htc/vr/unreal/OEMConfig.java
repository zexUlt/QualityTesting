// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

package com.htc.vr.unreal;

import android.util.Log;
import com.htc.vr.sdk.VRActivityDelegate;
import java.util.HashMap;
import android.os.Bundle;
import java.util.function.Consumer;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.os.Handler;
import android.os.HandlerThread;
import android.content.Intent;
import android.content.Context;

/**
 * Created by devin_yu on 2018/9/12.
 */

public class OEMConfig {
	private static final String TAG = "OEMConfig";
	private static OEMConfig mInstance = null;
	private VRActivityDelegate mVRActivityDelegate = null;
    private Context mContext;

	public native void initNative();
	public native void ConfigChangedNative();

	private boolean mRuntimeStarted = false;
	private boolean configChanged = false;
	HashMap<String, String> configStrMap = new HashMap<String, String>();
	private String[] KeyArray = {
		"controller_property",
		"battery_indicator",
		"controller_singleBeam"
	};

	public OEMConfig() {
		Log.i(TAG, "constructor and initNative");
		initNative();
	}

    private Handler mRuntimeEventReceiverHandler = null;
    private HandlerThread mRuntimeReceiverThread = null;
    private RuntimeEventReceiver mRuntimeEventReceiver = null;

	// Must align with the definitions in VR.java of wvr_client
	private static final String WAVE_ACTION_OEMCONFIG_CHANGED = "com.htc.vr.sdk.action.OEM_CONFIG_CHANGED";
	private static final String WAVE_EXTRA_OEMCONFIGDATA = "com.htc.vr.sdk.extra.OEMConfigData";

	private class RuntimeEventReceiver extends BroadcastReceiver {
	    @Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG, "[RuntimeEventReceiver] onReceive, action: " + intent.getAction());
			switch (intent.getAction()) {
				case WAVE_ACTION_OEMCONFIG_CHANGED:
					Log.d(TAG, "[RuntimeEventReceiver] WAVE_ACTION_OEMCONFIG_CHANGED");
					updateOEMConfig();

					break;
			}
		}
	}

	private void registerWaveEventReceiver() {
		if (mRuntimeReceiverThread == null) {
			mRuntimeReceiverThread = new HandlerThread("Wave Runtime Event Receiver");
			mRuntimeReceiverThread.start();
			mRuntimeEventReceiverHandler = new Handler(mRuntimeReceiverThread.getLooper());

			IntentFilter intentFilter = new IntentFilter();
			intentFilter.addAction(WAVE_ACTION_OEMCONFIG_CHANGED);
			mRuntimeEventReceiver = new RuntimeEventReceiver();
			mContext.registerReceiver(mRuntimeEventReceiver, intentFilter, null, mRuntimeEventReceiverHandler);
		}
	}

	private void unregisterWaveEventReceiver() {
		if (mRuntimeEventReceiver != null) {
			mContext.unregisterReceiver(mRuntimeEventReceiver);
			mRuntimeEventReceiver = null;
		}
		if (mRuntimeReceiverThread != null) {
			mRuntimeReceiverThread.quitSafely();
			mRuntimeReceiverThread = null;
		}
	}
	public static OEMConfig getInstance() {
		if (mInstance == null) {
			mInstance = new OEMConfig();
		}

		return mInstance;
	}

	private void handleVrEnvironmentVerifyCompleted(boolean hasExternalDisplay) {
        updateOEMConfig();
	}

	public void setDelegate(VRActivityDelegate tVRActivityDelegate) {
		Log.i(TAG, "mVR is updated.");
		mVRActivityDelegate = tVRActivityDelegate;
		mVRActivityDelegate.supportExternalDisplay().thenAcceptAsync(new Consumer<Boolean>(){
			@Override
			public void accept(Boolean hasExternalDisplay) {
				Log.d(TAG, "supportExternalDisplay() hasExternalDisplay: " + hasExternalDisplay);
				handleVrEnvironmentVerifyCompleted(hasExternalDisplay);
			}
		});
	}

    public void setContext(Context ctx) {
		mContext = ctx;
        registerWaveEventReceiver();
	}

    public void onDestroy() {
        unregisterWaveEventReceiver();
    }

    public void onResume() {
        updateOEMConfig();
    }

	public String getJsonRawData() {
		return getJsonRawDataByKey("controller_property");
	}

	public String getBatteryIndicatorData() {
		return getJsonRawDataByKey("battery_indicator");
	}

	public String getSingleBeamData() {
		return getJsonRawDataByKey("controller_singleBeam");
	}

	public String getJsonRawDataByKey(final String key) {
		String ret = "";
		if (configStrMap.containsKey(key)) {
			ret = configStrMap.get(key);
		} else {
			String value = mVRActivityDelegate.getConfigData(key);
			if (value == null) {
				Log.i(TAG, "OEMConfig key " + key + " not found");
			} else {
				ret = value;
			}
		}
		Log.i(TAG, "OEMConfig key: " + key + " value: " + ret);
		return ret;
	}

	public void updateOEMConfig() {
		if (!mRuntimeStarted) {
			Log.i(TAG, "updateOEMConfig, runtime is not ready yet.");
			return;
		}

		if (mVRActivityDelegate == null) {
			Log.i(TAG, "updateOEMConfig, VRActivityDelegate is null.");
			return;
		}

        Bundle data = mVRActivityDelegate.getConfigBundle(null);

        if (data == null) {
            Log.i(TAG, "updateOEMConfig, getConfigBundle return null.");
            return;
        }

		String value = "";
		configChanged = false;

		for (int i=0; i<KeyArray.length; i++) {
			String key = KeyArray[i];

			value = data.getString(key);

			Log.d(TAG, key + " = " + value);
			if (value != null) {
				if (configStrMap.containsKey(key)) {
					String mapValue = configStrMap.get(key);
					if (!value.equals(mapValue)) {
						Log.i(TAG, "OEMConfig key " + key + " is updated to " + value);
						configStrMap.remove(key);
						configStrMap.put(key, value);
						configChanged = true;
					}
				} else {
					Log.i(TAG, "OEMConfig key " + key + " with value " + value + " added");
					configStrMap.put(key, value);
					configChanged = true;
				}
			}
		}

		if (configChanged) {
			Log.i(TAG, "onConfigChanged, call native.");
			ConfigChangedNative();
		}
	}
}
