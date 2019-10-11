package com.example.techmaster.rfid;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class DbAdapter {
    public static final String KEY_ID = "MA_SP";
    public static final String KEY_NAME = "TEN_SP";

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

    public long createSANPHAM(String id, String name) {
        ContentValues inititalValues = new ContentValues();
        inititalValues.put(KEY_ID, id);
        inititalValues.put(KEY_NAME, name);
        return sqLiteDatabase.insert(DATABASE_TABLE, null, inititalValues);
    }

    public boolean deleteSANPHAM(long rowId) {
        return sqLiteDatabase.delete(DATABASE_TABLE, KEY_ID + "=" + rowId, null) > 0;
    }

    public boolean deleteAllSANPHAM() {
        return sqLiteDatabase.delete(DATABASE_TABLE, null, null) > 0;
    }

    public Cursor getAllSANPHAM() {
        return sqLiteDatabase.query(DATABASE_TABLE, new String[]{KEY_ID, KEY_NAME}, null, null, null, null, null);
    }

    public Cursor getSANPHAM(String ID){
        return sqLiteDatabase.query(DATABASE_TABLE, new String[]{KEY_NAME}, "MA_SP = ?", new String[] { ID }, null, null, null);
    }
}
