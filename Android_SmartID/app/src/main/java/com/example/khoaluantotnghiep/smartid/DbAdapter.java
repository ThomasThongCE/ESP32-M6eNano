package com.example.khoaluantotnghiep.smartid;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.CancellationSignal;
import android.support.v4.app.NavUtils;

public class DbAdapter {
    public static final String KEY_ID = "MA_SP";
    public static final String KEY_NAME = "TEN_SP";
    public static final String KEY_GIA = "GIA_SP";
    public static final String KEY_CANNANG = "CANNANG_SP";
    boolean distinct = true;
    CancellationSignal cancellationSignal = new CancellationSignal();

    private DatabaseHelper dbHelper;
    private SQLiteDatabase sqLiteDatabase;
    private static final String DATABASE_NAME = "Database_SANPHAM";
    private static final String DATABASE_TABLE = "SANPHAM";
    private static final int DATABASE_VERSION = 2;
    private final Context context;

    public DbAdapter(Context ctx) {
        this.context = ctx;
    }

    public DbAdapter open() {
        dbHelper = new DatabaseHelper(context, DATABASE_NAME, null, DATABASE_VERSION);
        sqLiteDatabase = dbHelper.getWritableDatabase();
        return this;
    }

    public void close() {
        dbHelper.close();
    }

    public long createSANPHAM(String id, String name, String gia, String Can_Nang) {
        ContentValues inititalValues = new ContentValues();
        inititalValues.put(KEY_ID, id);
        inititalValues.put(KEY_NAME, name);
        inititalValues.put(KEY_GIA, gia);
        inititalValues.put(KEY_CANNANG, Can_Nang);
        return sqLiteDatabase.insert(DATABASE_TABLE, null, inititalValues);
    }

    public boolean deleteSANPHAM(String rowId) {
        return sqLiteDatabase.delete(DATABASE_TABLE, KEY_ID + "=?", new String[]{rowId}) > 0;
    }

    public boolean deleteAllSANPHAM() {
        return sqLiteDatabase.delete(DATABASE_TABLE, null, null) > 0;
    }

    public Cursor getAllSANPHAM() {
        return sqLiteDatabase.query(DATABASE_TABLE, new String[]{KEY_ID, KEY_NAME, KEY_GIA, KEY_CANNANG}, null, null, null, null, null);
    }

    public Cursor getAllTenSANPHAM() {
        return sqLiteDatabase.query(distinct,DATABASE_TABLE, new String[]{KEY_NAME}, null, null, null, null, null, null);
    }

    public Cursor getSANPHAM(String ID){
        return sqLiteDatabase.query(DATABASE_TABLE, new String[]{KEY_NAME}, "MA_SP = ?", new String[] { ID }, null, null, null);
    }
}
