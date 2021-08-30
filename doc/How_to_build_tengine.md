## How to build Tengine libary with TIMVX for i.mx8m plus

### Clone TIMVX, Tengine and 3rdparty include floder
``` bash
$ git clone https://github.com/VeriSilicon/TIM-VX.git
$ git clone https://github.com/OAID/Tengine.git tengine-lite
$ wget -c https://github.com/VeriSilicon/TIM-VX/releases/download/v1.1.28/aarch64_S905D3_D312513_A294074_R311680_T312233_O312045.tgz
$ tar zxvf aarch64_S905D3_D312513_A294074_R311680_T312233_O312045.tgz
$ mv aarch64_S905D3_D312513_A294074_R311680_T312233_O312045 prebuild-sdk-s905d3
```

### Prepare dependencies
``` bash
$ cd <tengine-lite-root-dir>
$ cp -rf ../TIM-VX/include  ./source/device/tim-vx/
$ cp -rf ../TIM-VX/src      ./source/device/tim-vx/
$ mkdir -p ./3rdparty/tim-vx/include
$ mkdir -p ./3rdparty/tim-vx/lib/aarch64
$ cp -rf ../prebuild-sdk-s905d3/include/*  ./3rdparty/tim-vx/include/
```

### Compile
``` bash
$ mkdir build && cd build
$ cmake -DTENGINE_ENABLE_TIM_VX=ON ..
$ make -j`nproc` && make install
```
