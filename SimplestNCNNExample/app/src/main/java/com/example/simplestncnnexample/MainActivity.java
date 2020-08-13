package com.example.simplestncnnexample;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;


public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        AssetManager am = getAssets();

        // Bitmap
        String filename = "test.png";
        Bitmap bitmap = null;
        try
        {
            InputStream is = am.open(filename);
            bitmap = BitmapFactory.decodeStream(is);
            is.close();
        }catch (IOException e)
        {
            e.printStackTrace();
        }
        Log.i("OCR", "java forwarding ...");
        tv.setText(stringFromJNI(am, bitmap));
    }

    public native String stringFromJNI(AssetManager am, Bitmap bitmap);
}