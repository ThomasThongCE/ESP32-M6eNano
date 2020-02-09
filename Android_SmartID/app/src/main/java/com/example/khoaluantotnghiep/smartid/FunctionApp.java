package com.example.khoaluantotnghiep.smartid;

import android.content.Intent;
import android.os.Bundle;

import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;


public class FunctionApp extends AppCompatActivity {
    Button pay_btn;
    Button import_btn;
    Button check_btn;
    Button setting_btn;
    String BluetoothName_RFID;
    String BluetoothAddress_RFID;
    String BluetoothName_Scale;
    String BluetoothAddress_Scale;
    public static final String EXTRA_BLUETOOTH_ALL = "EXTRA_BLUETOOTH_ALL";
    @Override
    protected void onCreate(Bundle bundle){
        super.onCreate(bundle);
        setContentView(R.layout.functionapp_layout);

        try{
            bundle = getIntent().getBundleExtra(main_scale.EXTRA_BLUETOOTH_BUNDLE_SCALE);
            BluetoothName_Scale = bundle.getString(main_scale.EXTRA_BLUETOOTH_NAME_SCALE);
            BluetoothAddress_Scale = bundle.getString(main_scale.EXTRA_BLUETOOTH_ADDRESS_SCALE);
            Log.e("Bluetooth Name Scale",BluetoothName_Scale);
            Log.e("Bluetooth Address Scale", BluetoothAddress_Scale);

            bundle = bundle.getBundle(main.EXTRA_BLUETOOTH_BUNDLE_RFID);
            BluetoothName_RFID = bundle.getString(main.EXTRA_BLUETOOTH_NAME_RFID);
            BluetoothAddress_RFID = bundle.getString(main.EXTRA_BLUETOOTH_ADDRESS_RFID);
            Log.e("Bluetooth Name RFID",BluetoothName_RFID);
            Log.e("Bluetooth Address RFID", BluetoothAddress_RFID);
        }catch (Exception e){

        }
        Log.e(BluetoothAddress_Scale, "can");
        Log.e(BluetoothAddress_RFID, "RFID");

        pay_btn = findViewById(R.id.pay_btn);
        import_btn = findViewById(R.id.import_btn);
        check_btn = findViewById(R.id.check_btn);
        setting_btn = findViewById(R.id.setting_btn);

        pay_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(FunctionApp.this, Pay.class);
                Bundle bundle = new Bundle();
                bundle.putString(main.EXTRA_BLUETOOTH_NAME_RFID, BluetoothName_RFID);
                bundle.putString(main.EXTRA_BLUETOOTH_ADDRESS_RFID, BluetoothAddress_RFID);
                bundle.putString(main_scale.EXTRA_BLUETOOTH_NAME_SCALE, BluetoothName_Scale);
                bundle.putString(main_scale.EXTRA_BLUETOOTH_ADDRESS_SCALE, BluetoothAddress_Scale);
                intent.putExtra(EXTRA_BLUETOOTH_ALL, bundle);
                startActivity(intent);
            }
        });

        import_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(FunctionApp.this, Import.class);
                Bundle bundle = new Bundle();
                bundle.putString(main.EXTRA_BLUETOOTH_NAME_RFID, BluetoothName_RFID);
                bundle.putString(main.EXTRA_BLUETOOTH_ADDRESS_RFID, BluetoothAddress_RFID);
                bundle.putString(main_scale.EXTRA_BLUETOOTH_NAME_SCALE, BluetoothName_Scale);
                bundle.putString(main_scale.EXTRA_BLUETOOTH_ADDRESS_SCALE, BluetoothAddress_Scale);
                intent.putExtra(EXTRA_BLUETOOTH_ALL, bundle);
                startActivity(intent);
            }
        });

        check_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(FunctionApp.this, Check.class);
                startActivity(intent);
            }
        });

        setting_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(FunctionApp.this, Setting_app.class);
                startActivity(intent);
            }
        });
    }
}
