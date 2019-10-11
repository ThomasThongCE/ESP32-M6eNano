package com.example.techmaster.rfid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Adapter;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

public class RFID_UHF extends AppCompatActivity {
    DbAdapter databaseUHF;
    Cursor cursor;
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    String deviceName;
    String deviceAddress;
    BluetoothSocket bluetoothSocket;
    ControllerBluetooth controllerBluetooth;
    Handler mainHandler;
    ListView ListViewDataRFID;
    List ListDataRFID = new ArrayList();

    void ConnectDevice() throws Exception{
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();

        bluetoothSocket.connect();

        controllerBluetooth = new ControllerBluetooth(bluetoothSocket, RFID_UHF.this) {
            @Override
            public void error() {
                Toast.makeText(getApplicationContext(), "Error RFID", Toast.LENGTH_SHORT).show();
                finish();
            }

            @Override
            public void disconnect() {
                mainHandler.post(new Runnable() {

                    @Override
                    public void run() {
                        Toast.makeText(getApplicationContext(), "Device disconnected", Toast.LENGTH_SHORT).show();
                        try {
                            bluetoothSocket.close();
                        } catch (Exception e)
                        {
                            Log.e("Disconnect","Fail");
                        }
                        finish();
                    }
                });
            }

            @Override
            public void receiveData(final String data) {
                //Toast.makeText(RFID_UHF.this, data, Toast.LENGTH_LONG).show();
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        //Toast.makeText(RFID_UHF.this, data, Toast.LENGTH_LONG).show();
                        cursor = databaseUHF.getSANPHAM(data);
                        //Log.i("UHF", "error");
                        cursor.moveToNext();
                        if(cursor.getCount() != 0 && !ListDataRFID.contains(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME))))
                            ListDataRFID.add(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME)));
                        else
                            ListDataRFID.add(data);
                        //ListDataRFID.add(data);
                        ArrayAdapter adapterListData = new ArrayAdapter(RFID_UHF.this, android.R.layout.simple_list_item_1, ListDataRFID);
                        ListViewDataRFID.setAdapter(adapterListData);
                    }
                });
            }
        };
    }


    @Override
    protected void onCreate(Bundle saveInstanceSate){
        super.onCreate(saveInstanceSate);
        databaseUHF = new DbAdapter(this);
        databaseUHF.open();
        databaseUHF.deleteAllSANPHAM();
        databaseUHF.createSANPHAM("E20000162513", "DELL VOSTRO 5468");
        databaseUHF.createSANPHAM("0123456789AB", "THINK PAD T480");
        //Log.i("cusor", cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME)));
        setContentView(R.layout.rfid_uhf);
        Intent intent = getIntent();
        deviceName = intent.getStringExtra(MainActivity.EXTRA_BLUETOOTH_NAME);
        deviceAddress = intent.getStringExtra(MainActivity.EXTRA_BLUETOOTH_ADDRESS);
        mainHandler = new Handler(this.getMainLooper());
        final Button button = findViewById(R.id.buttonSend);
        ListViewDataRFID = findViewById(R.id.listData);
        Log.i("Device", deviceName);
        Log.i("Address", deviceAddress);
        try
        {
            ConnectDevice();
        }
        catch (Exception e)
        {
            Toast.makeText(getApplicationContext(), "Error", Toast.LENGTH_SHORT).show();
            finish();
        }

        Timer timerObj = new Timer();
        TimerTask timerTaskObj = new TimerTask() {
            public void run() {
                //perform your action here
            }
        };
        timerObj.schedule(timerTaskObj, 0, 15000);

        controllerBluetooth.startListening();
        Log.i("Listening", "ERROR");
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.i("Kha",button.getText().toString());
                if(button.getText().toString().equals("Scan")){
                    try {
                        controllerBluetooth.sendData("s");
                        Log.i("EROR","win");
                    }
                    catch (Exception e)
                    {
                        Log.i("EROR","fail");
                        controllerBluetooth.disconnect();
                        finish();
                    }
                    button.setText("Cancel Scan");
                }
                else
                {
                    try {
                        controllerBluetooth.sendData("a");
                        Log.i("EROR","win");
                    }
                    catch (Exception e)
                    {
                        Log.i("EROR","fail");
                        controllerBluetooth.disconnect();
                        finish();
                    }
                    button.setText("Scan");
                }

            }
        });

    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        controllerBluetooth.disconnect();
        Log.i("Disconnect", "ESP32");
    }
}
