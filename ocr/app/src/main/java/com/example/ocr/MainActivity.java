package com.example.ocr;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.Image;
import android.net.Uri;
import android.os.Bundle;
import android.os.Build;
import android.provider.MediaStore;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.content.res.AssetManager;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

    //调用系统相册-选择图片
//    private static final int IMAGE = 1;
    private Button button;
    private ImageView imageView;
    private AssetManager am;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        button = (Button)findViewById(R.id.choose);
        imageView = (ImageView)findViewById(R.id.imageview);

        am = getAssets();
        initModel(am);
//        String s = stringFromJNI(am);
//        Log.i("###OCR###", s);

        button.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view)
            {
//                Intent intent = new Intent(Intent.ACTION_PICK, null);
//                intent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
//                Intent intent = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.setType("image/*");
                startActivityForResult(intent, 2);
            }
        });
        // Example of a call to a native method
//        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());

    }
    protected void onActivityResult(int requestCode, int resultCode, Intent data){
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 2) {
            // 从相册返回的数据
            if (data != null) {
                // 得到图片的全路径
                Uri uri = data.getData();
                Bitmap bitmap = null;
                if(uri != null)
                {
                    try
                    {
                        // 读取图片
                        bitmap = MediaStore.Images.Media.getBitmap(this.getContentResolver(), uri);
//                        stringFromJNI(am, bitmap);
                        detect(bitmap);
                        int [] rects = {0,0,2,2};
                        drawRectangles(bitmap, rects);
                    }catch(IOException e)
                    {
                        e.printStackTrace();
                        imageView.setImageURI(uri);
                    }
                }
            }
        }
    }



    private void drawRectangles(Bitmap imageBitmap, int[] Rects) {
        int left, top, right, bottom;
        Bitmap mutableBitmap = imageBitmap.copy(Bitmap.Config.ARGB_8888, true);
        Canvas canvas = new Canvas(mutableBitmap);
        Paint paint = new Paint();
        for (int i = 0; i < 1; i++) {
            left = Rects[i * 4];
            top = Rects[i * 4 + 1];
            right = Rects[i * 4 + 2];
            bottom = Rects[i * 4 + 3];
            paint.setColor(Color.RED);
            paint.setStyle(Paint.Style.STROKE);//不填充
            paint.setStrokeWidth(10);  //线的宽度
            canvas.drawRect(left, top, right, bottom, paint);
        }
        imageView.setImageBitmap(mutableBitmap);//imageView: 定义在xml布局中的ImagView控件
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
//    public native String stringFromJNI(AssetManager am, Bitmap bitmap);
    public native int initModel(AssetManager am);
    public native String detect(Bitmap bitmap);
}