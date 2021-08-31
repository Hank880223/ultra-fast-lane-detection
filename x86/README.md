
## Clone Tengine file
```bash
$ cp -r <tengine-lite-root-dir>/tests/common/tengine_operations.c ultra-fast-lane-detection-master/src/
$ cp -r <tengine-lite-root-dir>/tests/common/ ultra-fast-lane-detection-master/include/
$ cp -r <tengine-lite-root-dir>/3rdparty/ ultra-fast-lane-detection-master/
$ cp -r <tengine-lite-root-dir>/3rdparty/tim-vx/include/CL/cl_viv_vx_ext.h ultra-fast-lane-detection-master/src/
$ cp -r <tengine-lite-root-dir>/build/install/lib ultra-fast-lane-detection-master/
$ cp -r <tengine-lite-root-dir>/build/install/include/tengine ultra-fast-lane-detection-master/include/
```

## Build & Compile
```bash
$ mkdir build && cd build
$ cmake ..
$ make -j`nproc`
```

## Run
```bash
$ ./tracking
```

