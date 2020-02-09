package com.example.khoaluantotnghiep.smartid;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.List;

public class Apdater_Pay extends BaseAdapter {


    private Context context;
    private List<Product> listProduct;
    private int layoutPay;

    public Apdater_Pay(Context context, int layoutPay,  List<Product> listProduct) {
        this.context = context;
        this.listProduct = listProduct;
        this.layoutPay = layoutPay;
    }

    @Override
    public int getCount() {
        return listProduct.size();
    }

    @Override
    public Object getItem(int i) {
        return null;
    }

    @Override
    public long getItemId(int i) {
        return 0;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        view = inflater.inflate(R.layout.detail_pay, null);
        TextView TenSp = view.findViewById(R.id.NameProduct);
        TenSp.setText("Tên Sản Phẩm: " + listProduct.get(i).getTenSp());
        TextView DonGia = view.findViewById(R.id.Value);
        DonGia.setText("Đơn Giá: " + listProduct.get(i).getGia());
        TextView SoLuong = view.findViewById(R.id.SL);
        SoLuong.setText("Số Lượng: " + listProduct.get(i).getSoluong());
        TextView ThanhTien = view.findViewById(R.id.ThanhTien);
        ThanhTien.setText("Thành Tiền: " + listProduct.get(i).getThanhTien());
        return view;
    }
}
