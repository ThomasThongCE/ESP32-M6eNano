package com.example.khoaluantotnghiep.smartid;

import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;

import java.util.ArrayList;

public class Check extends AppCompatActivity {

    DbAdapter databaseUHF;
    Cursor cursor;
    ArrayList<Product> products = new ArrayList<>();
    ArrayList<Product> products_select = new ArrayList<>();
    Spinner comboBox;
    ArrayList<String> spinner = new ArrayList<>();
    ListView listProduct;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setContentView(R.layout.check_layout);
        listProduct = findViewById(R.id.product);
        comboBox = findViewById(R.id.comboBox);
        spinner.add("All");

        databaseUHF = new DbAdapter(this);
        databaseUHF.open();

        cursor = databaseUHF.getAllSANPHAM();
        cursor.moveToNext();
        while (!cursor.isAfterLast()) {
            Product product = new Product();
            product.setMaSp(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_ID)));
            product.setTenSp(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_NAME)));
            product.setSoluong(1);
            product.setGia(Integer.parseInt(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_GIA))));
            product.setCanNang(Float.parseFloat(cursor.getString(cursor.getColumnIndex(databaseUHF.KEY_CANNANG))));

            Log.e("LKL", product.getTenSp());

            for (int i = 0; i < products.size(); i++) {
                if (product.getTenSp().equals(products.get(i).getTenSp())) {
                    products.get(i).setSoluong(products.get(i).getSoluong() + 1);
                }
            }
            if (!spinner.contains(product.getTenSp())) {
                spinner.add(product.getTenSp());
                products.add(product);
            }
            cursor.moveToNext();
        }
        databaseUHF.close();

        ArrayAdapter spinner_adapter = new ArrayAdapter(this, android.R.layout.simple_list_item_1, spinner);
        comboBox.setAdapter(spinner_adapter);


        Log.e("Log", comboBox.getSelectedItem().toString());

        comboBox.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                if(comboBox.getSelectedItem().toString().equals("All")) {
                    Adapter_custom_check arrayAdapter = new Adapter_custom_check(Check.this, R.layout.custom_listview, products);

                    listProduct.setAdapter(arrayAdapter);
                }
                else {
                    products_select.clear();
                    for (int j = 0; j < products.size(); j++) {
                        if (comboBox.getSelectedItem().toString().equals(products.get(j).getTenSp())) {
                            products_select.add(products.get(j));
                        }
                    }
                    Adapter_custom_check arrayAdapter = new Adapter_custom_check(Check.this, R.layout.custom_listview, products_select);
                    listProduct.setAdapter(arrayAdapter);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {
                Adapter_custom_check arrayAdapter = new Adapter_custom_check(Check.this, R.layout.custom_listview, products);

                listProduct.setAdapter(arrayAdapter);
            }
        });
    }
}
