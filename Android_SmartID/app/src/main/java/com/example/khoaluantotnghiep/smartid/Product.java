package com.example.khoaluantotnghiep.smartid;

public class Product {
    private String MaSp;
    private String TenSp;
    private Float CanNang;
    private int Gia;
    private int Soluong = 0;
    private float TongCanNang = 0;
    private int ThanhTien = 0;

    public int getSoluong() {
        return Soluong;
    }

    public float getTongCanNang() {
        return TongCanNang;
    }

    public void setTongCanNang(float tongCanNang) {
        TongCanNang = tongCanNang;
    }

    public int getThanhTien() {
        return Soluong*Gia;
    }

    public void setThanhTien(int thanhTien) {
        ThanhTien = thanhTien;
    }

    public void setSoluong(int soluong) {
        Soluong = soluong;
    }

    public void setMaSp(String maSp) {
        MaSp = maSp;
    }

    public void setTenSp(String tenSp) {
        TenSp = tenSp;
    }

    public void setCanNang(Float canNang) {
        CanNang = canNang;
    }

    public void setGia(int gia) {
        Gia = gia;
    }

    public String getMaSp() {
        return MaSp;
    }

    public String getTenSp() {
        return TenSp;
    }

    public Float getCanNang() {
        return CanNang;
    }

    public int getGia() {
        return Gia;
    }
}
