How to build this kernel open source
Steps for building

1.Untar xxx.tar.xz
Example:xz -d xxx.tar.xz
        tar zxf xxx.tar

2.prepare environment
export PATH=/yourCodebase/$toolchain:$PATH
Example:export PATH=/yourCodebase/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin:$PATH

3.build kernel
cd kernel-3.10
mkdir out
make O=out TARGET_ARCH=$TARGET_ARCH ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE pixi45_defconfig
make -j8 O=out TARGET_ARCH=$TARGET_ARCH ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE
