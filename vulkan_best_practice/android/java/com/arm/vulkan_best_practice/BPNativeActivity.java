/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

package com.arm.vulkan_best_practice;

import android.app.NativeActivity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.NotificationManagerCompat;
import android.support.v4.content.FileProvider;
import android.util.Log;
import android.view.View;
import android.webkit.MimeTypeMap;
import android.widget.Toast;

import java.io.File;
import java.lang.Runnable;
import java.util.concurrent.atomic.AtomicInteger;

public class BPNativeActivity extends NativeActivity {

    private Context context;

    private static final String NOTIFICATION_PREFS = "NOTIFICATION_PREFS";

    private static final String NOTIFICATION_KEY = "NOTIFICATION_KEY";

    private static final String CHANNEL_ID = "vkb";

    private static int initialId = 0;

    @Override
    protected void onCreate(Bundle instance) {
        super.onCreate(instance);

        
         // Create Notification Channel API 26+
         // 
         // API 26+ implements notifications using a channel where notifications are submitted to channel
         // The following initialises the channel used by vkb to push notifications to.
         // IMPORTANCE_HIGH = push notificaitons
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, getString(R.string.channel_name), NotificationManager.IMPORTANCE_HIGH);
            channel.setDescription(getString(R.string.channel_desc));
            NotificationManager nm = getSystemService(NotificationManager.class);
            nm.createNotificationChannel(channel);
        }

        context = this;
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            View decorView = getWindow().getDecorView();
            decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_FULLSCREEN);
        }
    }

    private int getId() {
        SharedPreferences prefs = this.getSharedPreferences(NOTIFICATION_PREFS, context.MODE_PRIVATE);
        int id = initialId;
        if(prefs.contains(NOTIFICATION_KEY)) {
            id = prefs.getInt(NOTIFICATION_KEY, initialId);
        }

        SharedPreferences.Editor editor = prefs.edit();
        editor.putInt(NOTIFICATION_KEY, id + 1);
        editor.apply();

        return id;
    }

    public void fatalError(final String log_file) {
        File file = new File(log_file);

        Uri path = FileProvider.getUriForFile(context, context.getApplicationContext().getPackageName() + ".provider", file);


        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        Intent applicationChooser = Intent.createChooser(intent, context.getResources().getString(R.string.open_file_with));
        intent.setDataAndType(path, context.getContentResolver().getType(path));
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

        PendingIntent pi = PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setSmallIcon(R.drawable.icon)
                .setContentTitle("Vulkan Best Practice Error")
                .setContentText("Fatal Error: click to view")
                .setStyle(new NotificationCompat.BigTextStyle().bigText("Log: " + log_file))
                .setAutoCancel(true)
                .setContentIntent(pi)
                .setPriority(NotificationCompat.PRIORITY_HIGH);

        NotificationManagerCompat nm = NotificationManagerCompat.from(context);
        nm.notify(getId(), builder.build());
    }

    /**
     * @brief Create a push notification from JNI with a message
     *
     * @param message A string of a message to be shown
     */
    private void notification(final String message) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setSmallIcon(R.drawable.icon)
                .setContentTitle("Vulkan Best Practice Error")
                .setContentText(message)
                .setStyle(new NotificationCompat.BigTextStyle()
                .bigText(message))
                .setPriority(NotificationCompat.PRIORITY_HIGH);

        NotificationManagerCompat nm = NotificationManagerCompat.from(context);
        nm.notify(getId(), builder.build());
    }

    /**
     * @brief Create a toast from JNI on the android main UI thread with a message
     *
     * @param message A string of a message to be shown
     */
    private void toast(final String message) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, message, Toast.LENGTH_LONG).show();
            }
        });
    }
}
