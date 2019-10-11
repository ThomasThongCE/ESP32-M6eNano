package com.example.techmaster.rfid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    ListView ListViewDeviceBluetooth;
    Button ScanBluetooth;
    BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    List ListDeviceBluetooth = new ArrayList();
    Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
    private BluetoothSocket bluetoothSocket;
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    public static final String EXTRA_BLUETOOTH_NAME = "EXTRA_BLUETOOTH_NAME";
    public static final String EXTRA_BLUETOOTH_ADDRESS = "EXTRA_BLUETOOTH_ADDRESS";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ListViewDeviceBluetooth = findViewById(R.id.listBlue);
        ScanBluetooth = findViewById(R.id.scan);
        ScanBluetooth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                update();
            }
        });
    }

    void update() {
        ListDeviceBluetooth.clear();
        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {

                ListDeviceBluetooth.add(device.getName() + "\n" + device.getAddress());
            }
        }
        final ArrayAdapter adapterListDeviceBluetooth =
                new ArrayAdapter(this, android.R.layout.simple_list_item_1, ListDeviceBluetooth);
        ListViewDeviceBluetooth.setAdapter(adapterListDeviceBluetooth);

        ListViewDeviceBluetooth.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                String[] XuLyChuoi = ListDeviceBluetooth.get(i).toString().split("\n");
                Intent NewIntent = new Intent(MainActivity.this, RFID_UHF.class);
                NewIntent.putExtra(EXTRA_BLUETOOTH_NAME, XuLyChuoi[0]);
                NewIntent.putExtra(EXTRA_BLUETOOTH_ADDRESS, XuLyChuoi[1]);
                startActivity(NewIntent);
            }
        });
    }
}
