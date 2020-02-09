package com.example.khoaluantotnghiep.smartid;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.List;

public class Adapter_customlistview extends BaseAdapter {
    private Context context;
    private List<bluetooth> listBluetooth;
    private int layoutBluetooth;

    public void setContext(Context context) {
        this.context = context;
    }

    public void setListBluetooth(List<bluetooth> listBluetooth) {
        this.listBluetooth = listBluetooth;
    }

    public void setLayoutBluetooth(int layoutBluetooth) {
        this.layoutBluetooth = layoutBluetooth;
    }

    public Context getContext() {
        return context;
    }

    public List<bluetooth> getListBluetooth() {
        return listBluetooth;
    }

    public int getLayoutBluetooth() {
        return layoutBluetooth;
    }

    public Adapter_customlistview(Context context, int layoutBluetooth, List<bluetooth> listBluetooth) {
        this.context = context;
        this.listBluetooth = listBluetooth;
        this.layoutBluetooth = layoutBluetooth;
    }

    @Override
    public int getCount() {
        return listBluetooth.size();
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
        view = inflater.inflate(R.layout.item_layout, null);
        TextView nameBluetooth = view.findViewById(R.id.nameBluetooth);
        nameBluetooth.setText(listBluetooth.get(i).getNameBluetooth());
        TextView addressBluetooth = view.findViewById(R.id.addressBluetooth);
        addressBluetooth.setText(listBluetooth.get(i).getAddressBluetooth());
        return view;
    }
}
