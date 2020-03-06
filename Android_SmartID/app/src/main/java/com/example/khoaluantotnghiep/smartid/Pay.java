package com.example.khoaluantotnghiep.smartid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Stack;
import java.util.UUID;

public class Pay extends AppCompatActivity {
    ListView listViewProduct;
    ArrayList<String> listProduct = new ArrayList<>();
    Button scan, stopScan;
    Button confirm;
    Button scale;
    ImageButton clear;
    TextView numberPay;
    TextView numberScale;
    BluetoothSocket bluetoothSocket;
    BluetoothSocket bluetoothSocket_Scale;
    ControllerBluetooth controllerBluetooth;
    ControllerBluetooth controllerBluetooth_Scale;
    Handler handler = new Handler();
    Handler handler_try = new Handler();
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final int UPDATE_LISTVIEW = 0;
    private static final  String LOGTAG = "PAY";
    DbAdapter databaseUHF;
    ArrayList<Product> ListProductDB = new ArrayList<>();
    ArrayList<Product> ListProductScan = new ArrayList<>();
    ArrayList<bluetooth> ListBluetooth = new ArrayList<>();
    ArrayList<String> ListID = new ArrayList<>();
    ArrayList<String> ListNameSP = new ArrayList<>();
    ArrayList<String> ListIDDatabase = new ArrayList<>();
    Cursor cursor;
    int SumProduct = 0;
    float SumScale = 0;
    Locale locale = new Locale("vi", "VN");
    NumberFormat format = NumberFormat.getInstance(locale);
    LinearLayout cover;
    TextView cooldown;
    Apdater_Pay apdater_pay;

    Stack<String> stack = new Stack<String>();
    Handler updateUIHandler;
    CheckBox rawData;
    TextView totalItems;
    ArrayAdapter<String> rawDataAdapter ;
    boolean rawAdapterType;
    private static final Object lockObject = new Object(), rawFlagLock = new Object();

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setContentView(R.layout.pay_layout);
        cover = findViewById(R.id.layout_cover_pay);
        cooldown = findViewById(R.id.cooldown);
        cover.setVisibility(View.VISIBLE);
        cooldown.setText("Đang kết nối Bluetooth RFID...");

        listViewProduct = findViewById(R.id.listProduct);

        rawAdapterType = false;
        rawDataAdapter = new ArrayAdapter<String>(Pay.this, R.layout.simple_listview, ListID);
        apdater_pay = new Apdater_Pay(Pay.this, R.layout.custom_listview, ListProductScan);
        listViewProduct.setAdapter(apdater_pay);

        updateUIHandler = new Handler(){
            @Override
            public void handleMessage(Message msg) {
                if(msg.what==UPDATE_LISTVIEW){
                    boolean temp ;
                    synchronized (rawFlagLock) {
                        temp = rawAdapterType;
                    }
                    if (temp){
                        totalItems.setText(String.valueOf(ListID.size()));
                        rawDataAdapter.notifyDataSetChanged();
                    } else {
                        apdater_pay.notifyDataSetChanged();
                        totalItems.setText(String.valueOf(ListProductScan.size()));
                    }

                    Log.d(LOGTAG, "Update UI");
                }
                super.handleMessage(msg);
            }
        };

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
                    Toast.makeText(Pay.this, "Kết Nối Bluetooth RFID Thành Công", Toast.LENGTH_SHORT).show();
                } catch (Exception e) {
                    cooldown.setText("Không có kết nối Bluetooth với Thiết Bị RFID \n Chạm vào màng hình để kết nối lại");
                    Toast.makeText(Pay.this, "Kết Nối Bluetooth RFID Thất Bại", Toast.LENGTH_LONG).show();
                    Toast.makeText(Pay.this, "Vui Lòng Thử Lại", Toast.LENGTH_LONG).show();
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
                    Toast.makeText(Pay.this, "Kết Nối Bluetooth Cân Thất Bại", Toast.LENGTH_LONG).show();
                    Toast.makeText(Pay.this, "Vui Lòng Thử Lại", Toast.LENGTH_LONG).show();
                    finish();
                }
            }
        });

        final Thread dataProcess = new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(LOGTAG, "Data process thread running");
                try {
                    boolean tempRawAdapterType;
                    while(true) {
                        synchronized (lockObject) {
                            if (stack.empty()) {
                                Log.d(LOGTAG, "Data process thread wait");
                                lockObject.wait();
                                Log.d(LOGTAG, "Data process thread be notify");
                            }
                        }
                        synchronized (rawFlagLock) {
                            tempRawAdapterType = rawAdapterType;
                        }

                        if (!stack.empty()) {
                            String data = stack.pop();
                            Log.d(LOGTAG, "data process: " + data);
                            if ((ListIDDatabase.contains(data.trim()) || tempRawAdapterType) && !ListID.contains(data.trim())) {
                                ListID.add(data);

                                if (!tempRawAdapterType || ListIDDatabase.contains(data.trim())){
                                    if (ListNameSP.contains(getNameSp(data))) {
                                        ListProductScan.get(getIndexListProductScan(getNameSp(data)))
                                                .setSoluong(ListProductScan.get(getIndexListProductScan(getNameSp(data))).getSoluong() + 1);
                                    } else {
                                        Product product = new Product();
                                        product.setMaSp(ListProductDB.get(getIndexListProductDB(data)).getMaSp());
                                        product.setTenSp(ListProductDB.get(getIndexListProductDB(data)).getTenSp());
                                        product.setGia(ListProductDB.get(getIndexListProductDB(data)).getGia());
                                        product.setCanNang(ListProductDB.get(getIndexListProductDB(data)).getCanNang());
                                        product.setSoluong(product.getSoluong() + 1);
                                        ListProductScan.add(product);
                                        ListNameSP.add(product.getTenSp());
                                    }
                                }

                                updateUI();
                            }
                        }
                    }

                } catch (InterruptedException e) {
//                    Toast.makeText(Pay.this, "Kết Nối Bluetooth Cân Thất Bại", Toast.LENGTH_LONG).show();
//                    Toast.makeText(Pay.this, "Vui Lòng Thử Lại", Toast.LENGTH_LONG).show();
//                    finish();
                    Log.d(LOGTAG, "TThead caught exception");
                }

            }});
        dataProcess.start();

        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                thread.run();
            }
        }, 1000);

        cover.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                cooldown.setText("Đang kết nối Bluetooth RFID...");
                handler_try.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        thread.run();
                    }
                }, 1000);
            }
        });
        confirm = findViewById(R.id.confirm);

        format.setMaximumIntegerDigits(10);
        format.setMinimumIntegerDigits(3);

        //Database

        databaseUHF = new DbAdapter(this);
        databaseUHF.open();


        cursor = databaseUHF.getAllSANPHAM();
        try {
            do {
                cursor.moveToNext();
                Product product = new Product();
                product.setMaSp(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_ID)));
                product.setTenSp(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME)));
                product.setGia(Integer.parseInt(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_GIA))));
                product.setCanNang(Float.parseFloat(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_CANNANG))));
                ListProductDB.add(product);
                ListIDDatabase.add(product.getMaSp().trim());
                Log.e("Base", product.getMaSp().trim());
            } while (!cursor.isLast());
        } catch (Exception e) {
            Toast.makeText(getApplicationContext(), "Database Trống", Toast.LENGTH_LONG).show();
        }

        databaseUHF.close();



        numberPay = findViewById(R.id.numberPay);

        numberPay.setText(SumProduct + " VNĐ");
        scan = findViewById(R.id.Scan);
        stopScan = findViewById(R.id.StopScan);
        clear = findViewById(R.id.clear);

        numberScale = findViewById(R.id.numberScale);
        scale = findViewById(R.id.scale_button);

        rawData = findViewById(R.id.rawdata);
        totalItems = findViewById(R.id.totalItems);

        rawData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (((CheckBox) view).isChecked()) {
                    synchronized (rawFlagLock) {
                        rawAdapterType = true;
                    }
                    listViewProduct.setAdapter(rawDataAdapter);
                } else {
                    synchronized (rawFlagLock) {
                        rawAdapterType = false;
                    }
                    listViewProduct.setAdapter(apdater_pay);
                }
                updateUI();
            }
        });

        scale.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    thread1.run();
                    controllerBluetooth_Scale.sendData("c");
                    scale.setEnabled(false);
                    scale.setBackground(getResources().getDrawable(R.drawable.background_content));
                    scale.setText("Đang Kiểm Tra...");
                } catch (Exception e) {
                    Log.e("scale", e.toString());
                    Log.e("Loi", "Khong gui duoc");
                }
            }
        });

        scan.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                scan.setText("Đang Quét...");
                try {
                    controllerBluetooth.sendData("a");
                } catch (Exception e) {
                    Toast.makeText(Pay.this, "Press Scan Again", Toast.LENGTH_SHORT).show();
                }
            }
        });

        stopScan.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                scan.setText("Quét");
                try {
                    controllerBluetooth.sendData("b");
                    ThanhToan();
                } catch (Exception e) {
                    Toast.makeText(Pay.this, "Press Scan Again", Toast.LENGTH_SHORT).show();
                }

            }
        });

        clear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                numberPay.setText("0 VNĐ");
                numberScale.setText("0 gram");
                SumScale = 0;
                SumProduct = 0;
                listProduct.clear();
                ListID.clear();
                ListNameSP.clear();
                ListProductScan.clear();
                updateUI();
                confirm.setEnabled(false);
                confirm.setBackground(getDrawable(R.color.colorButtonConfirm_sss));
                numberScale.setTextColor(getColor(R.color.colorTextWhite));
            }
        });

        confirm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                confirm.setEnabled(false);
                confirm.setBackground(getDrawable(R.color.colorButtonConfirm_sss));

                cover.setVisibility(View.VISIBLE);
                cooldown.setText("Thanh Toán Thành Công");
                cooldown.setTextSize(30);

                handler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        finish();
                    }
                }, 2000);

            }
        });
    }

    void updateUI(){
        Log.d(LOGTAG, "sending message to handler");
        Message msg = handler.obtainMessage();
        msg.what = UPDATE_LISTVIEW;
        updateUIHandler.sendMessage(msg);
    }

    void ConnectDevice(String deviceAddress) throws Exception {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
        bluetoothSocket.connect();

        controllerBluetooth = new ControllerBluetooth(bluetoothSocket, Pay.this) {
            @Override
            public void error() {
                Toast.makeText(getApplicationContext(), "Error RFID", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void disconnect() {
                try {
                    bluetoothSocket.close();
                } catch (Exception e) {
                    Toast.makeText(Pay.this, "Disconnect Fail", Toast.LENGTH_SHORT).show();
                    Log.e("Disconnect", "Fail");
                }
                Log.e("Disconnect", "Complete");
            }

            @Override
            public void receiveData(final String data) {
                Log.d(LOGTAG, "Bluetooth recieve: " + data);
                synchronized (lockObject) {
                    stack.push(data);
                    lockObject.notify();
                }
            }
        };
    }


    void ConnectDevice_Scale(String deviceAddress) throws Exception {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothSocket_Scale = device.createRfcommSocketToServiceRecord(MY_UUID);
        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
        bluetoothSocket_Scale.connect();

        controllerBluetooth_Scale = new ControllerBluetooth(bluetoothSocket_Scale, Pay.this) {
            @Override
            public void error() {
                Toast.makeText(getApplicationContext(), "Error RFID", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void disconnect() {
                try {
                    bluetoothSocket_Scale.close();
                } catch (Exception e) {
                    Toast.makeText(Pay.this, "Disconnect Fail", Toast.LENGTH_SHORT).show();
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
                            if (Float.parseFloat(data) < SumScale + SumScale * 0.05 && Float.parseFloat(data) > SumScale - SumScale * 0.05) {
                                confirm.setEnabled(true);
                                confirm.setBackground(getDrawable(R.drawable.confirm_buy));
                                numberScale.setTextColor(getResources().getColor(R.color.Ok));
                            } else {
                                numberScale.setTextColor(getResources().getColor(R.color.colorButtonConfirm));
                            }
                            scale.setEnabled(true);
                            scale.setText("Cân Kiểm Tra");
                            scale.setBackground(getResources().getDrawable(R.drawable.button));
                            controllerBluetooth_Scale.disconnect();
                        }
                    }
                });
            }
        };
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            controllerBluetooth.disconnect();
        } catch (Exception e) {

        }
    }

    public int getIndexListProductDB(String s) {
        for (int i = 0; i < ListProductDB.size(); i++) {
            if (ListProductDB.get(i).getMaSp().equals(s))
                return i;
        }
        ;
        return ListProductDB.size();
    }

    ;

    public String getNameSp(String data) {
        for (int i = 0; i < ListProductDB.size(); i++) {
            if (ListProductDB.get(i).getMaSp().equals(data)) {
                return ListProductDB.get(i).getTenSp();
            }
        }
        return "NULL";
    }


    public int getIndexListProductScan(String s) {
        for (int i = 0; i < ListProductScan.size(); i++) {
            if (ListProductScan.get(i).getTenSp().equals(s))
                return i;
        }
        ;
        return ListProductScan.size();
    }

    int index = 0;
    public void ThanhToan() {
//        Log.e("size List", String.valueOf(ListID.size()));
//        for(int i = 0; i < ListID.size(); i++){
//            Log.e("ThamChieu", ListID.get(i));
//        }

        int j = ListProductScan.size() - 1;
        while (j >= index) {
            SumScale = SumScale + ListProductScan.get(j).getCanNang();
            SumProduct = SumProduct + ListProductScan.get(j).getThanhTien();
            j--;

        }

        index = ListProductScan.size();
        numberPay.setText(SumProduct + " VNĐ");
        numberScale.setText(SumScale + " gram");
    }
}
