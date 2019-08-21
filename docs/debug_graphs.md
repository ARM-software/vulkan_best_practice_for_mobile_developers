<!--
- Copyright (c) 2019, Arm Limited and Contributors
-
- SPDX-License-Identifier: MIT
-
- Permission is hereby granted, free of charge,
- to any person obtaining a copy of this software and associated documentation files (the "Software"),
- to deal in the Software without restriction, including without limitation the rights to
- use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
- and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
-
- The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
-
- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
- IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
- WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-
-->

![Debug Graph Image](./images/debug-graph.png)

# Debug Graphs

You can view the debug graphs by clicking on "Save Debug Graphs" in the debug menu. This will then output graphs at:

```
desktop: output/graphs/
android: Android/data/com.arm.vulkan_best_practice/files/output/graphs
```

These graphs can be opened in the visualiser which you can find in the assets section of the release.

## Debug Graph Info Bar

The info bar allows you to inspect the data saved for a selected node.

## Debug Graphs Controls

| Action                       |                 Desktop                  |                  Android                  |
| :--------------------------- | :--------------------------------------: | :---------------------------------------: |
| Select Node                  |             <kbd>click</kbd>             |              <kbd>tap</kbd>               |
| Drag node                    | <kbd>click</kbd> + <kbd>drag</kbd> node  |  <kbd>hold</kbd> + <kbd>drag</kbd> node   |
| Drag graph                   | <kbd>click</kbd> + <kbd>drag</kbd> graph |  <kbd>tap</kbd> + <kbd>drag</kbd> graph   |
| Multi-select                 |   <kbd>shift</kbd> + <kbd>click</kbd>    | <kbd>tap</kbd> + <kbd>hold</kbd> multiple |
| Area-select                  |    <kbd>shift</kbd> + <kbd>drag</kbd>    |                     -                     |
| Select direct decendant tree |     double <kbd>click</kbd> on node      |       double <kbd>tap</kbd> on node       |
| Open/Close info bar          |       <kbd>click</kbd> on info bar       |        <kbd>tap</kbd> on info bar         |
| Zoom                         |            <kbd>scroll</kbd>             |             <kbd>pinch</kbd>              |
