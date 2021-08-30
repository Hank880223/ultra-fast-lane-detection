# Ultra-fast-lane-detection
## Build i.MX8M Plus BSP

* [BSP](https://github.com/Hank880223/ncnn-sort-vehicle/blob/main/doc/BSP.md)

## How to build Tengine libary with TIMVX for i.mx8m plus
* [Tengine](https://github.com/Hank880223/ultra-fast-lane-detection/blob/main/doc/How_to_build_tengine.md)

## Clone Tengine file
```bash
$ cp -r <tengine-lite-root-dir>/tests/ ultra-fast-lane-detection-master/
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
$ . /opt/bsp-5.4.70-2.3.3/environment-setup-aarch64-poky-linux
$ cmake ..
$ make -j`nproc`
```

## Run
```bash
$ ./tracking
```
