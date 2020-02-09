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

public class main extends AppCompatActivity {

    ListView listViewBluetooth;
    ArrayList<bluetooth> listBluetooth = new ArrayList<bluetooth>() {};
    Button buttonRefresh;

    BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
    public static final String EXTRA_BLUETOOTH_NAME_RFID = "EXTRA_BLUETOOTH_NAME_RFID";
    public static final String EXTRA_BLUETOOTH_ADDRESS_RFID = "EXTRA_BLUETOOTH_ADDRESS_RFID";
    public static final String EXTRA_BLUETOOTH_BUNDLE_RFID = "BundleBluetooth_RFID";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout);
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
                Intent NewIntent = new Intent(main.this, main_scale.class);
                Bundle bundle = new Bundle();
                bundle.putString(EXTRA_BLUETOOTH_NAME_RFID, listBluetooth.get(i).getNameBluetooth());
                bundle.putString(EXTRA_BLUETOOTH_ADDRESS_RFID, listBluetooth.get(i).getAddressBluetooth());
                NewIntent.putExtra(EXTRA_BLUETOOTH_BUNDLE_RFID, bundle);
                startActivity(NewIntent);
            }
        });
    }
}
