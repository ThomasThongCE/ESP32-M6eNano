package com.example.khoaluantotnghiep.smartid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Set;
import java.util.UUID;

public class main_scale extends AppCompatActivity {

    ListView listViewBluetooth;
    ArrayList<bluetooth> listBluetooth = new ArrayList<bluetooth>() {};
    Button buttonRefresh;
    Bundle globalBundle;

    BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
    public static final String EXTRA_BLUETOOTH_NAME_SCALE = "EXTRA_BLUETOOTH_NAME_SCALE";
    public static final String EXTRA_BLUETOOTH_ADDRESS_SCALE = "EXTRA_BLUETOOTH_ADDRESS_SCALE";
    public static final String EXTRA_BLUETOOTH_BUNDLE_SCALE = "BundleBluetooth_SCALE";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout_scale);
        globalBundle = getIntent().getBundleExtra(main.EXTRA_BLUETOOTH_BUNDLE_RFID);
        listViewBluetooth = findViewById(R.id.listBluetooth);
        buttonRefresh = findViewById(R.id.Refresh_btn);
        update();
        buttonRefresh.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                update();
            }
        });
    }

    void update() {
        listBluetooth.clear();
        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {
                listBluetooth.add(new bluetooth(device.getName(), device.getAddress()));
            }
        }
        final Adapter_customlistview adapterListDeviceBluetooth =
                new Adapter_customlistview(this, R.layout.item_layout, listBluetooth);
        listViewBluetooth.setAdapter(adapterListDeviceBluetooth);

        listViewBluetooth.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                Intent NewIntent = new Intent(main_scale.this, FunctionApp.class);
                Bundle bundle = new Bundle();
                bundle.putString(EXTRA_BLUETOOTH_NAME_SCALE, listBluetooth.get(i).getNameBluetooth());
                bundle.putString(EXTRA_BLUETOOTH_ADDRESS_SCALE, listBluetooth.get(i).getAddressBluetooth());
                bundle.putBundle(main.EXTRA_BLUETOOTH_BUNDLE_RFID, globalBundle);
                NewIntent.putExtra(EXTRA_BLUETOOTH_BUNDLE_SCALE, bundle);
                startActivity(NewIntent);
            }
        });
    }
}
