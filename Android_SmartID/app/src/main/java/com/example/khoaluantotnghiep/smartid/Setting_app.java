package com.example.khoaluantotnghiep.smartid;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class Setting_app extends AppCompatActivity {
    Button setting;
    Button addData;
    DbAdapter databaseUHF;
    @Override
    public void onCreate(Bundle bundle){
        super.onCreate(bundle);
        setContentView(R.layout.setting_layout);
        databaseUHF = new DbAdapter(this);
        setting = findViewById(R.id.deletedatabase);
        addData = findViewById(R.id.asd);
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

        addData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try{
                    databaseUHF.open();
                    databaseUHF.createSANPHAM("E280110C20003A51BC900018", "Thiet Ke Luan Ly So", "28000", "227");
                    databaseUHF.createSANPHAM("E280110C20003A91BC900018", "Linh Kien Dien Tu", "33000", "217");
                    databaseUHF.createSANPHAM("E280110C20003191BCA90018", "Kien Truc May Tinh", "36000", "237");
                    databaseUHF.createSANPHAM("E280110C20003711BCC30018", "He Dieu Hanh", "24000", "206");
                    databaseUHF.close();
                    Toast.makeText(Setting_app.this, "Thêm Database Thành Công", Toast.LENGTH_SHORT).show();
                }catch (Exception e){
                    Toast.makeText(Setting_app.this, "Data Đã Tồn Tại", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }
}
