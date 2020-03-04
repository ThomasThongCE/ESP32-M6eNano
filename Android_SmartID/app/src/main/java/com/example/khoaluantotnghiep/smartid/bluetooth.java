package com.example.khoaluantotnghiep.smartid;

public class bluetooth {
    private String nameBluetooth;
    private String addressBluetooth;

    public bluetooth(String nameBluetooth, String addressBluetooth) {
        this.nameBluetooth = nameBluetooth;
        this.addressBluetooth = addressBluetooth;
    }

    public String getNameBluetooth() {
        return nameBluetooth;
    }

    public String getAddressBluetooth() {
        return addressBluetooth;
    }

    public void setNameBluetooth(String nameBluetooth) {
        this.nameBluetooth = nameBluetooth;
    }

    public void setAddressBluetooth(String addressBluetooth) {
        this.addressBluetooth = addressBluetooth;
    }
}
