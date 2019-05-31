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

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class BPSampleActivity extends AppCompatActivity {
    private ListView listview;
    private Button button;
    private Button button_permissions;
    private TextView text_permissions;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bp_sample_activity);

        button = findViewById(R.id.button_demo_mode);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                setArguments(new String[]{});

                Intent intent = new Intent(BPSampleActivity.this, BPNativeActivity.class);
                startActivity(intent);
            }
        });

        List<Sample> sample_list = new ArrayList<Sample>();

        if (LoadNativeLibrary(getResources().getString(R.string.native_lib_name))) {
            sample_list = Arrays.asList(getSamples());
            File external_files_dir = getExternalFilesDir("assets");
            File temp_files_dir = getCacheDir();
            if (external_files_dir != null && temp_files_dir != null){
                initFilePath(external_files_dir.toString(), temp_files_dir.toString());
            }
        }

        SampleArrayAdapter sample_list_adapter = new SampleArrayAdapter(this, sample_list);

        listview = findViewById(R.id.sample_list);
        listview.setAdapter(sample_list_adapter);
        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Sample selected_sample = (Sample)parent.getItemAtPosition(position);

                // Configure native code
                setArguments(new String[]{selected_sample.getId()});

                Intent intent = new Intent(BPSampleActivity.this, BPNativeActivity.class);
                startActivity(intent);
            }
        });

        button_permissions = findViewById(R.id.button_permissions);
        button_permissions.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                checkPermissions();
            }
        });
        text_permissions = findViewById(R.id.text_permissions);
        checkPermissions();
    }

    private boolean LoadNativeLibrary(String native_lib_name) {
        boolean status = true;

        try {
            System.loadLibrary(native_lib_name);
        } catch (UnsatisfiedLinkError e) {
            Toast
                .makeText(getApplicationContext(),
                          "Native code library failed to load.",
                          Toast.LENGTH_SHORT)
                .show();

            status = false;
        }

        return status;
    }

    private native Sample[] getSamples();

    private native void setArguments(String []args);

    private native void initFilePath(String asset_path, String temp_path);

    private static final int RC_READ_EXTERNAL_STORAGE = 1;
    private static final int RC_WRITE_EXTERNAL_STORAGE = 2;

    private void checkPermissions() {
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(BPSampleActivity.this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    RC_WRITE_EXTERNAL_STORAGE);
        } else if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(BPSampleActivity.this,
                    new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                    RC_READ_EXTERNAL_STORAGE);
        } else {
            showSamples();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case RC_WRITE_EXTERNAL_STORAGE: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    showPermissionsMessage();
                    break;
                }
            }
            case RC_READ_EXTERNAL_STORAGE: {
                if (grantResults.length > 0
                        && grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    showPermissionsMessage();
                    break;
                }
            }
            default:
                showSamples();
        }
    }

    public void showPermissionsMessage() {
        text_permissions.setVisibility(View.VISIBLE);
        button_permissions.setVisibility(View.VISIBLE);
        listview.setVisibility(View.INVISIBLE);
        button.setVisibility(View.INVISIBLE);
    }

    public void showSamples() {
        text_permissions.setVisibility(View.INVISIBLE);
        button_permissions.setVisibility(View.INVISIBLE);
        listview.setVisibility(View.VISIBLE);
        button.setVisibility(View.VISIBLE);
    }
}
