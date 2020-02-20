package com.example.khoaluantotnghiep.smartid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.util.UUID;

public class Import extends AppCompatActivity {

    DbAdapter databaseUHF;
    Button read_id;
    BluetoothSocket bluetoothSocket;
    BluetoothSocket bluetoothSocket_Scale;
    ControllerBluetooth controllerBluetooth;
    ControllerBluetooth controllerBluetooth_Scale;
    Handler handler = new Handler();
    TextView ID;
    EditText name;
    EditText price;
    TextView weight;
    Button confirm;
    Button can;
    Boolean status = false;
    LinearLayout cover;
    TextView cooldown;
    String numberweight = "0";
    Handler handler_try = new Handler();


    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setContentView(R.layout.import_layout);
        cover = findViewById(R.id.layout_cover_pay);
        cooldown = findViewById(R.id.cooldown);
        cover.setVisibility(View.VISIBLE);
        cooldown.setText("Đang kết nối Bluetooth RFID...");

        bundle = getIntent().getBundleExtra(FunctionApp.EXTRA_BLUETOOTH_ALL);
        final String addressRF = bundle.getString(main.EXTRA_BLUETOOTH_ADDRESS_RFID);
        final String addressScale = bundle.getString(main_scale.EXTRA_BLUETOOTH_ADDRESS_SCALE);

        Log.e("RFID_Address", addressRF);
        Log.e("Scale_Address", addressScale);

        final Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    ConnectDevice(addressRF);
                    controllerBluetooth.startListening();
                    cover.setVisibility(View.GONE);
                    Toast.makeText(Import.this, "Kết Nối Bluetooth RFID Thành Công", Toast.LENGTH_SHORT).show();
                } catch (Exception e) {
                    cooldown.setText("Không có kết nối Bluetooth với Thiết Bị RFID \n Chạm vào màng hình để kết nối lại");
                    Toast.makeText(Import.this, "Kết Nối Bluetooth RFID Thất Bại", Toast.LENGTH_LONG).show();
                    Toast.makeText(Import.this, "Vui Lòng Thử Lại", Toast.LENGTH_LONG).show();
                    //finish();
                }
            }
        });

        final Thread thread1 = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    ConnectDevice_Scale(addressScale);
                    controllerBluetooth_Scale.startListening();
                } catch (Exception e) {
                    Toast.makeText(Import.this, "Kết Nối Bluetooth Cân Thất Bại", Toast.LENGTH_LONG).show();
                    Toast.makeText(Import.this, "Vui Lòng Thử Lại", Toast.LENGTH_LONG).show();
                    finish();
                }
            }
        });

        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                thread.run();
            }
        },5000);



        cover.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                cooldown.setText("Đang kết nối Bluetooth RFID...");
                handler_try.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        thread.run();
                    }
                },5000);
            }
        });

        read_id = findViewById(R.id.ReadID);
        ID = findViewById(R.id.ID);
        name = findViewById(R.id.name);
        price = findViewById(R.id.price);
        weight = findViewById(R.id.weight);
        confirm = findViewById(R.id.confirm);
        can = findViewById(R.id.ReadScale);

        can.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try{
                    thread1.run();
                    Log.e("scale" , "Success");
                    controllerBluetooth_Scale.sendData("c");
                }
                catch (Exception e){
                    Log.e("scale" , e.toString());
                    Log.e("Loi", "Khong gui duoc");
                }
            }
        });

        databaseUHF = new DbAdapter(this);
        databaseUHF.open();

        read_id.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    ID.setText("");
                    controllerBluetooth.sendData("a");
                    Toast.makeText(Import.this, "Reading...", Toast.LENGTH_SHORT).show();
                } catch (Exception e) {
                    Toast.makeText(Import.this, "Reading Fail", Toast.LENGTH_SHORT).show();
                    controllerBluetooth.disconnect();
                    finish();
                }
            }
        });

        confirm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //
                if (name.getText().toString().equals("") || price.getText().toString().equals("") || ID.getText().toString().trim().equals("") || weight.getText().toString().equals("")) {
                    Toast.makeText(Import.this, "Thông Tin Sản Phẩm Bị Thiếu", Toast.LENGTH_SHORT).show();
                } else {
                    try {
                        Cursor cursor = databaseUHF.getSANPHAM(ID.getText().toString().trim());
                        cursor.moveToNext();
                        cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME));
                    } catch (Exception e) {
                        status = true; //không tồn tại ID
                    }

                    if (status) {
                        Toast.makeText(Import.this, "Thêm Sản Phẩm Thành Công", Toast.LENGTH_SHORT).show();
                        databaseUHF.createSANPHAM(ID.getText().toString().trim(), name.getText().toString().trim(), price.getText().toString().trim(), numberweight);
                    } else {
                        Toast.makeText(Import.this, "ID " + ID.getText().toString().trim() + " Đã Tồn Tại", Toast.LENGTH_SHORT).show();
                    }
                    status = false; //tồn tại ID tron database

                }
            }
        });
    }

    void ConnectDevice(String deviceAddress) throws Exception {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
        bluetoothSocket.connect();

        controllerBluetooth = new ControllerBluetooth(bluetoothSocket, Import.this) {
            @Override
            public void error() {
                Toast.makeText(getApplicationContext(), "Error RFID", Toast.LENGTH_SHORT).show();
                finish();
            }

            @Override
            public void disconnect() {
                try {
                    bluetoothSocket.close();
                } catch (Exception e) {
                    Toast.makeText(Import.this, "Disconnect Fail", Toast.LENGTH_SHORT).show();
                    Log.e("Disconnect", "Fail");
                }
                Log.e("Disconnect", "Complete");
            }

            @Override
            public void receiveData(final String data) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            Log.e("TagID", data);
                            if (ID.getText().equals("")) {

                                try {
                                    Cursor cursor = databaseUHF.getSANPHAM(data);
                                    cursor.moveToNext();
                                    cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME));
                                } catch (Exception e) {
                                    status = true; //không tồn tại ID
                                }

                                if (status) {
                                    Toast.makeText(Import.this, "Read Success", Toast.LENGTH_SHORT).show();
                                    ID.setText(data.trim());
                                }
                                status = false; //tồn tại ID tron database
                            }
                        } catch (Exception e) {
                            Toast.makeText(Import.this, "Read Fail", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
            }
        };
    }

    void ConnectDevice_Scale(String deviceAddress) throws Exception {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothSocket_Scale = device.createRfcommSocketToServiceRecord(MY_UUID);
        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
        bluetoothSocket_Scale.connect();

        controllerBluetooth_Scale = new ControllerBluetooth(bluetoothSocket_Scale, Import.this) {
            @Override
            public void error() {
                Toast.makeText(getApplicationContext(), "Error RFID", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void disconnect() {
                try {
                    bluetoothSocket_Scale.close();
                } catch (Exception e) {
                    Toast.makeText(Import.this, "Disconnect Fail", Toast.LENGTH_SHORT).show();
                    Log.e("Disconnect", "Fail");
                }
                Log.e("Disconnect", "Complete");
            }

            @Override
            public void receiveData(final String data) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.e("Nhan", "OK");
                        if (data.contains(".")) {
                            Log.e("Kg", data);
                            weight.setText(data + "gram");
                            controllerBluetooth_Scale.disconnect();
                            numberweight = data;
                        }
                    }
                });
            }
        };
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try{
            controllerBluetooth.disconnect();
        }
        catch (Exception e){
        }
    }
}
