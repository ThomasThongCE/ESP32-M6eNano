package com.example.khoaluantotnghiep.smartid;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.List;

public class Adapter_custom_check extends BaseAdapter {
    private Context context;
    private List<Product> listProduct;
    private int layoutProduct;

    public Adapter_custom_check(Context context, int layoutProduct, List<Product> listProduct) {
        this.context = context;
        this.listProduct = listProduct;
        this.layoutProduct = layoutProduct;
    }

    public Context getContext() {
        return context;
    }

    public void setContext(Context context) {
        this.context = context;
    }

    public List<Product> getListProduct() {
        return listProduct;
    }

    public void setListProduct(List<Product> listProduct) {
        this.listProduct = listProduct;
    }

    public int getLayoutProduct() {
        return layoutProduct;
    }

    public void setLayoutProduct(int layoutProduct) {
        this.layoutProduct = layoutProduct;
    }

    @Override
    public int getCount() {
        Log.e("sl", listProduct.size() + " asd");
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
        view = inflater.inflate(R.layout.custom_listview, null);
        TextView tenSanPham = view.findViewById(R.id.tensanpham);
        tenSanPham.setText(listProduct.get(i).getTenSp());
        TextView soluong = view.findViewById(R.id.soluong);
        soluong.setText("Số Lượng: " + listProduct.get(i).getSoluong());
        return view;
    }
}
