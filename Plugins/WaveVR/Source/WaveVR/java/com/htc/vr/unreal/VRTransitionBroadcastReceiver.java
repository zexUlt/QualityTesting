// "WaveVR SDK
// © 2021 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

package com.htc.vr.unreal;

import android.util.Log;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Bundle;

import com.epicgames.ue4.GameActivity; //Context

public class VRTransitionBroadcastReceiver {
	private static final String TAG = "VRTransitionBroadcastReceiver";
	private static final String ACTION_TRANSITION_STATE = "vive.wave.intent.ACTION_TRANSITION_STATE";

	private static VRTransitionBroadcastReceiver mInstance = null;
        private static Context mContext = null;
        private Handler mReceiverHandler = null;
        private HandlerThread mReceiverThread = null;
        private VRTransitionBroadcastReceiver_Internal mReceiver = null;

	public native void StateOnReceive(boolean state);

	private class VRTransitionBroadcastReceiver_Internal extends BroadcastReceiver {
                @Override
                public void onReceive(Context context, Intent intent) {
                        Log.d(TAG, "OnReceive, action: " + intent.getAction());
                        switch (intent.getAction()) {
                                case ACTION_TRANSITION_STATE:
                                        Log.d(TAG, "ACTION_TRANSITION_STATE changing");
                                        Bundle bundle = intent.getExtras();
                                        StateOnReceive(bundle.getBoolean("state"));
                                        break;
                        }
                }
        }

	private void registerReceiver() {
		Log.i(TAG, "Register");
		if (mReceiverThread == null) {
			mReceiverThread = new HandlerThread("Register VRTransitionBroadcastReceiver");
			mReceiverThread.start();
			mReceiverHandler = new Handler(mReceiverThread.getLooper());

			IntentFilter intentFilter = new IntentFilter();
			intentFilter.addAction(ACTION_TRANSITION_STATE);
			mReceiver = new VRTransitionBroadcastReceiver_Internal();
			if (mContext != null) {
                                mContext.registerReceiver(mReceiver, intentFilter, null, mReceiverHandler);
                        }
		}
	}

	private void unregisterReceiver() {
		Log.i(TAG, "Unregister");
		if (mReceiver != null) {
			if (mContext != null) {
                                mContext.unregisterReceiver(mReceiver);
                        }
			mReceiver = null;
		}
		if (mReceiverThread != null) {
			mReceiverThread.quitSafely();
			mReceiverThread = null;
		}
	}

	public VRTransitionBroadcastReceiver() {
		Log.i(TAG, "Constructor");
		if (mContext == null) {
                        mContext = GameActivity.Get().getApplicationContext();
                        registerReceiver();
                }
	}

        public void onDestroy() {
		Log.i(TAG, "onDestroy");
                unregisterReceiver();
        }

	public static VRTransitionBroadcastReceiver getInstance() {
		if (mInstance == null) {
			mInstance = new VRTransitionBroadcastReceiver();
		}
		return mInstance;
	}
}
