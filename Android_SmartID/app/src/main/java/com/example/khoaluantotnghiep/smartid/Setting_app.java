package com.example.khoaluantotnghiep.smartid;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class Setting_app extends AppCompatActivity {
    Button setting;
    DbAdapter databaseUHF;
    @Override
    public void onCreate(Bundle bundle){
        super.onCreate(bundle);
        setContentView(R.layout.setting_layout);
        databaseUHF = new DbAdapter(this);
        setting = findViewById(R.id.deletedatabase);
        setting.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try{
                    databaseUHF.open();
                    databaseUHF.deleteAllSANPHAM();
                    databaseUHF.close();
                    Toast.makeText(Setting_app.this, "Xóa Database Thành Công", Toast.LENGTH_SHORT).show();
                }
                catch (Exception e)
                {
                    Toast.makeText(Setting_app.this, "Xóa Database Thất Bại", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }
}
