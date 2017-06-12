package com.projetinfo.android.watchint;

import android.app.Activity;
import android.os.Bundle;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by Seven on 16/05/2017.
 */

public class CallReceiver extends Activity{
    TelephonyManager tm;

    private MainActivity activity;

    public CallReceiver(MainActivity activity)
    {
        this.activity = activity;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        tm = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        tm.listen(mPhoneListener, PhoneStateListener.LISTEN_CALL_STATE);
    }

    private PhoneStateListener mPhoneListener = new PhoneStateListener() {
        public void onCallStateChanged(int state, String incomingNumber) {      //Classe anonyme
            try {
                Toast.makeText(CallReceiver.this, "CALL_STATE_RINGING", Toast.LENGTH_SHORT).show(); //On envoie un Toast "CALL_STATE_RINGING" dès que l'on reçoit un appel
                }
             catch (Exception e) {
                Log.i("Exception", "PhoneStateListener() e = " + e);
            }
        }
    };
}

