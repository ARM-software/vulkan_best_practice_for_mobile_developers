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

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class BPSampleActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bp_sample_activity);

        Button button = findViewById(R.id.button_demo_mode);
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
        }

        SampleArrayAdapter sample_list_adapter = new SampleArrayAdapter(this, sample_list);

        ListView listview = findViewById(R.id.sample_list);
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
}
