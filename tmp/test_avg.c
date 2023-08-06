#include <riscv_vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
/***
 * 无符号数右移一位向上取整求均值RVV 1.0 与C实现

实验环境
riscv64-unknown-linux-gnu-gcc -v
Using built-in specs.
COLLECT_GCC=riscv64-unknown-linux-gnu-gcc
COLLECT_LTO_WRAPPER=/home/sunmin/bins/gnu-toolchain-bins-rvv-1.0/bin/../libexec/gcc/riscv64-unknown-linux-gnu/14.0.0/lto-wrapper
Target: riscv64-unknown-linux-gnu
Configured with: /home/sunmin/aosp-out/riscv-gnu-toolchain/build-rvv-1.0-cintrinsic/../gcc/configure --target=riscv64-unknown-linux-gnu --prefix=/home/sunmin/aosp-out/gnu-toolchain-bins-rvv-1.0 --with-sysroot=/home/sunmin/aosp-out/gnu-toolchain-bins-rvv-1.0/sysroot --with-pkgversion=g80ae426a195 --with-system-zlib --enable-shared --enable-tls --enable-languages=c,c++,fortran --disable-libmudflap --disable-libssp --disable-libquadmath --disable-libsanitizer --disable-nls --disable-bootstrap --src=../../gcc --disable-multilib --with-abi=lp64d --with-arch=rv64gczve32x --with-tune=rocket --with-isa-spec=20191213 'CFLAGS_FOR_TARGET=-O2    -mcmodel=medlow' 'CXXFLAGS_FOR_TARGET=-O2    -mcmodel=medlow'
Thread model: posix
Supported LTO compression algorithms: zlib
gcc version 14.0.0 20230702 (experimental) (g80ae426a195)

qemu-riscv64 --version
qemu-riscv64 version 7.2.3 (v7.2.3)
Copyright (c) 2003-2022 Fabrice Bellard and the QEMU Project developers

编译
which riscv64-unknown-linux-gnu-gcc
/home/sunmin/bins/gnu-toolchain-bins-rvv-1.0/bin/riscv64-unknown-linux-gnu-gcc
riscv64-unknown-linux-gnu-gcc  -g -march=rv64gcv -mabi=lp64d -static -O0 test_avg.c -o test_avg

运行
export RISCV=/home/sunmin/bins/gnu-toolchain-bins-rvv-1.0
qemu-riscv64 -cpu rv64,v=true,vlen=256,vext_spec=v1.0  -L $RISCV/sysroot ./test_avg

调试
qemu-riscv64 -cpu rv64,v=true,vlen=256,vext_spec=v1.0  -g 2234 -L $RISCV/sysroot ./test_avg
riscv64-unknown-linux-gnu-gdb ./test_avg  -ex "target remote localhost:2234"
*/

vuint8m1_t test_avg_rvv(){
    
    const uint8_t a[] = { 32, 97, 245, 223,170,67,122,223,242,77,59,194,71,235,3,186};
    const uint8_t b[] = { 78, 27, 221, 2,184,189,140,248,118,112,129,178,121,232,86,24};

    //期望输出
    const uint8_t c[] = {55, 62, 233, 113, 177, 128, 131, 236, 180, 95, 94, 186, 96, 234, 45, 105};
    uint8_t d[16] = {0};

    vuint32m1_t va, vb;
    const size_t vl = 4;

    va = __riscv_vle32_v_u32m1(a, vl);
    vb = __riscv_vle32_v_u32m1(b, vl);

    //vor
    vuint32m1_t vr = __riscv_vor_vv_u32m1(va, vb, vl);

    //vxor
    vuint32m1_t vs = __riscv_vxor_vv_u32m1(va, vb, vl);

    // (vxor & 0xFEFEFEFEU)
    vs = __riscv_vand_vx_u32m1(vs, 0xFEFEFEFEU, vl);

    //(vxor & 0xFEFEFEFEU) >> 1
    vs = __riscv_vsrl_vx_u32m1(vs, 1, vl);

    //vor - (vxor & 0xFEFEFEFEU) >> 1

    vuint32m1_t vd =  __riscv_vsub_vv_u32m1(vr, vs, vl);

    vuint8m1_t v_u8 =  __riscv_vreinterpret_v_u32m1_u8m1(vd);

    __riscv_vse8_v_u8m1(d, v_u8, vl * 4);

    int i = 0;
    for (i; i < 16; ++i)
        printf("%d,", d[i]);

    return v_u8;
}

/***
 * 范文捷:
数字逻辑运算
a+b=(a|b)+(a&b)

范文捷:
a&b=(a|b)-(a^b)

范文捷:
a+b=(a|b)*2-(a^b)

范文捷:
已知要得到(a+b)/2四舍五入的值

孙敏:
哇，太数字逻辑了

范文捷:
那么在除以2之前将a+b向上取到最小的偶数就可

孙敏:
哇，精髓

范文捷:
(a|b)*2必然为偶数，那么要做的就是(a^b)为奇数时，将其最低位置0，这样a+b必然为偶数

范文捷:
之后将a+b右移一位，其偶数的2部分各右移

范文捷:
另外由于该数为偶数，右移之后下溢到其它通道的最高位的数也是0

孙敏:
太详细了

*/
uint32_t avg(uint32_t x1, uint32_t x2)
{
    return (x1 | x2) - (((x1 ^ x2) & 0xFEFEFEFEU) >> 1U);
}

void test_avg_c()
{
    uint8_t a[] = { 32, 97, 245, 223,170,67,122,223,242,77,59,194,71,235,3,186};
    uint8_t b[] = { 78, 27, 221, 2,184,189,140,248,118,112,129,178,121,232,86,24};
    uint8_t c[16] = { 0 };
    int i = 0;
    for (i; i < 4; ++i)
    {
        ((uint32_t*)c)[i] = avg(((uint32_t*)a)[i], ((uint32_t*)b)[i]);
    }
    i = 0;
    for (i; i < sizeof(c) / sizeof(*c); ++i)
        printf("%d,", c[i]);
    printf("\n");
    return 0; 
}

int main(void){
    test_avg_c();
    test_avg_rvv();
    return 0;
}