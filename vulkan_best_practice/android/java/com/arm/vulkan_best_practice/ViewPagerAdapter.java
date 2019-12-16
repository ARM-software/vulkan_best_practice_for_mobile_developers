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

import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.view.ViewGroup;
import android.widget.AdapterView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.TreeSet;

public class ViewPagerAdapter extends FragmentPagerAdapter {

    private static final List<String> defined_category_order = Collections.unmodifiableList(
        Arrays.asList("api", "performance"));

    private TabFragment currentFragment;

    private List<String> categories;

    private HashMap<String, List<Sample>> sampleMap;

    private AdapterView.OnItemClickListener clickListener;

    ViewPagerAdapter(FragmentManager manager, @NonNull HashMap<String, List<Sample>> categorizedSampleMap, AdapterView.OnItemClickListener clickListener) {
        super(manager);
        // Define order of category tabs
        TreeSet<String> category_set = new TreeSet<String>(new Comparator<String>() {
            @Override
            public int compare(String o1, String o2) {
                int ret;
                int o1_defined_order = defined_category_order.indexOf(o1);
                int o2_defined_order = defined_category_order.indexOf(o2);
                if (o1_defined_order > -1 && o2_defined_order > -1) {
                    // If in the pre-defined list, sort in the order they appear there
                    ret =  o1_defined_order > o2_defined_order ? 1 : -1;
                } else if (o1_defined_order > -1 || o2_defined_order > -1) {
                    // If not in the pre-defined list, sort after those that are
                    ret = 1;
                } else {
                    // Sort alphabetically
                    ret =  o1.compareTo(o2);
                }
                return ret;
            }
        });
        category_set.addAll(categorizedSampleMap.keySet());
        this.categories = new ArrayList<>();
        this.categories.addAll(category_set);
        this.sampleMap = categorizedSampleMap;
        this.clickListener = clickListener;
    }

    @Override
    public Fragment getItem(int position) {
        TabFragment fragment = (TabFragment) TabFragment.getInstance(categories.get(position));
        fragment.prepare(clickListener, sampleMap.get(categories.get(position)));
        return fragment;
    }

    @Override
    public void setPrimaryItem(@NonNull ViewGroup container, int position, @NonNull Object object) {
        if (getCurrentFragment() != object) {
            currentFragment = (TabFragment) object;
        }
        super.setPrimaryItem(container, position, object);
    }

    @Override
    public int getCount() {
        return categories.size();
    }

    @Override
    public CharSequence getPageTitle(int position) {
        return categories.get(position);
    }

    TabFragment getCurrentFragment() {
        return currentFragment;
    }
}
