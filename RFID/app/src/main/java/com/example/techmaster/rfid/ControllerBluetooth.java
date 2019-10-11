package com.example.techmaster.rfid;

import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public abstract class ControllerBluetooth {
    String buffer = "";
    BluetoothSocket bluetoothSocket;
    InputStream inputStream;
    OutputStream outputStream;
    Context context;

    public ControllerBluetooth(BluetoothSocket bluetoothSocket, Context context) throws IOException {
        this.context = context;
        this.bluetoothSocket = bluetoothSocket;
        this.outputStream = bluetoothSocket.getOutputStream();
        this.inputStream = bluetoothSocket.getInputStream();
    }

    public abstract void error();

    public abstract void disconnect();

    public abstract void receiveData(String data);

    public void sendHello() {
        String json = "{\"TYPE\":\"HELLO\"}\0";

        try {
            sendData(json);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void sendStart() {
        String json = "{\"TYPE\":\"START\"}\0";

        try {
            sendData(json);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void sendStop() {
        String json = "{\"TYPE\":\"STOP\"}\0";

        try {
            sendData(json);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void sendData(String message) throws IOException {
        outputStream.write(message.getBytes());
    }

    public void startListening() {
        (new Thread() {
            @Override
            public void run() {
                try {
                    while (bluetoothSocket.isConnected()) {
                        if (inputStream.available() > 0) {
                            byte[] inputBytes = new byte[1];
                            Log.i("asdad", "asdasd");
                            inputBytes[0] = (byte) inputStream.read();

                            if (inputBytes[0] != '\0') {
                                buffer += new String(inputBytes);
                                //buffer += String.valueOf(inputBytes);
                            } else {
                                // Send to UI
                                Log.i("Kha", "Clear");
                                Log.i("Kha", buffer.toString());
                                receiveData(buffer);
                                buffer = "";
                            }
                        }
                    }

                    disconnect();
                } catch (IOException e) {
                    // Error
                    error();
                }
            }
        }).start();
    }

}
