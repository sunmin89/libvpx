
#include <assert.h>
#include "./vpx_dsp_rtcd.h"
#include <riscv_vector.h>

#include "./vpx_dsp_rtcd.h"
#include "./vpx_config.h"

//AverageTest
uint32_t vpx_avg_4x4_rvv(const uint8_t *a, int a_stride) {
const size_t vl = 16;
vuint32m1_t a_u32 = __riscv_vlse32_v_u32m1(a, a_stride, vl / 4);

vuint8m1_t vv =  __riscv_vreinterpret_v_u32m1_u8m1(a_u32);

uint8_t tmp[vl];
__riscv_vse8_v_u8m1(tmp, vv, vl);

vuint8m1_t va = __riscv_vle8_v_u8m1(tmp, vl/2);
vuint8m1_t vb = __riscv_vle8_v_u8m1(tmp + vl/2, vl/2);

vuint16m2_t vc = __riscv_vwaddu_vv_u16m2(va, vb, vl);

vuint16m1_t scalar = __riscv_vmv_v_x_u16m1(0,vl/2);
vuint16m1_t vd = __riscv_vredsum_vs_u16m2_u16m1(vc, scalar, vl/2);

vd = __riscv_vadd_vx_u16m1(vd,1<<3, vl/2);
vd = __riscv_vsrl_vx_u16m1(vd,4, vl/2);
return __riscv_vmv_x_s_u16m1_u16(vd);

}

uint32_t vpx_avg_8x8_rvv(const uint8_t *a, int a_stride) {
    int i;
    vuint8m1_t vb, vc;
    vuint16m2_t sum;
    const size_t vl = 8;

    vb = __riscv_vle8_v_u8m1(a, vl);
    a += a_stride;
    vc = __riscv_vle8_v_u8m1(a, vl);
    a += a_stride;

    sum = __riscv_vwaddu_vv_u16m2(vb, vc, vl);

    for (i = 0; i < 6; ++i) {
        const vuint8m1_t vd = __riscv_vle8_v_u8m1(a, vl);
        a += a_stride;
        sum =  __riscv_vwaddu_wv_u16m2 (sum, vd, vl);
    }

    vuint16m1_t scalar = __riscv_vmv_v_x_u16m1(0, vl);
    vuint16m1_t vd = __riscv_vredsum_vs_u16m2_u16m1(sum, scalar, vl);

    vd = __riscv_vadd_vx_u16m1(vd,1<<5, vl/2);
    vd = __riscv_vsrl_vx_u16m1(vd,6, vl/2);
    return __riscv_vmv_x_s_u16m1_u16(vd);
}

// int vpx_satd_rvv(const tran_low_t *coeff, int length) {
//     const size_t vl = 4;
//     vint32m2_t sum_s32_0 = __riscv_vmv_v_x_i32m2(0, vl);
//     vint32m2_t sum_s32_1 = __riscv_vmv_v_x_i32m2(0, vl);
//     do {
//         vint16m1_t v0, v1;
//         vint16m1x2_t v2 = __riscv_vlseg2e16_v_i16m1x2(coeff, vl);
//         //odd index
//         v0 = __riscv_vget_v_i16m1x2_i16m1(v2, 0);
//         //even index
//         v1 = __riscv_vget_v_i16m1x2_i16m1(v2, 1);

//         vbool16_t b0 = __riscv_vmslt_vx_i16m1_b16(v0, 0, vl);
//         v0 = __riscv_vneg_v_i16m1_m(b0, v0, vl);
//         v1 = __riscv_vneg_v_i16m1_m(b1, v1, vl);
//         vint32m2_t va = __riscv_vwadd_vv_i32m2(v0, v1, vl);
//         sum_s32_0 =__riscv_vadd_vv_i32m2(sum_s32_0, va, vl);
        
//         v2 = __riscv_vlseg2e16_v_i16m1x2(coeff + 8, vl);

//         v0 = __riscv_vget_v_i16m1x2_i16m1(v2, 0);
//         v1 = __riscv_vget_v_i16m1x2_i16m1(v2, 1);
//         b0 = __riscv_vmslt_vx_i16m1_b16(v0, 0, vl);
//         v0 = __riscv_vneg_v_i16m1_m(b0, v0, vl);

//         vbool16_t b1 = __riscv_vmslt_vx_i16m1_b16(v1, 0, vl);
//         b1 = __riscv_vmslt_vx_i16m1_b16(v1, 0, vl);
//         v1 = __riscv_vneg_v_i16m1_m(b1, v1, vl);

//         vint32m2_t vb = __riscv_vwadd_vv_i32m2(v0, v1, vl);
//         sum_s32_1 =__riscv_vadd_vv_i32m2(sum_s32_1, vb, vl);

//         length -= 16;
//         coeff += 16;

//     }while (length != 0);

//     vint32m1_t scalar = __riscv_vmv_v_x_i32m1(0, vl);
//     return __riscv_vmv_x_s_i32m1_i32(__riscv_vredsum_vs_i32m2_i32m1(sum_s32_0, scalar, vl)) + __riscv_vmv_x_s_i32m1_i32(    __riscv_vredsum_vs_i32m2_i32m1(sum_s32_1, scalar, vl));
// }

/***
 * 
 Long pairwise add and accumulate:  
 adds adjacent pairs of elements in the second vector, sign extends or zero extends the 
results to twice the original width.  It then accumulates this with the corresponding  
element in the first vector and places the final results in the destination vector--
vpadal -> r0 = a0 + (b0 + b1), r1 = a0 + (b2 + b3),..., r3 = a3 + (b6 + b7); 
int16x4_t vpadal_s8 (int16x4_t __a, int8x8_t __b); 
*/

int vpx_satd_rvv(const tran_low_t *coeff, int length) {
    const size_t vl = 4;
    vint32m2_t sum_s32_0 = __riscv_vmv_v_x_i32m2(0, vl);
    vint32m2_t sum_s32_1 = __riscv_vmv_v_x_i32m2(0, vl);
    do {
        vint16m1_t v0, v1;
        vbool16_t b0, b1;
        vint16m1x2_t s0 = __riscv_vlseg2e16_v_i16m1x2(coeff, vl);
        //odd index
        v0 = __riscv_vget_v_i16m1x2_i16m1(s0, 0);
        b0 = __riscv_vmslt_vx_i16m1_b16(v0, 0, vl);
        v0 = __riscv_vneg_v_i16m1_m(b0, v0, vl);

        //even index
        v1 = __riscv_vget_v_i16m1x2_i16m1(s0, 1);
        b1 = __riscv_vmslt_vx_i16m1_b16(v1, 0, vl);
        v1 = __riscv_vneg_v_i16m1_m(b1, v1, vl);

        sum_s32_0 =__riscv_vadd_vv_i32m2(sum_s32_0, __riscv_vwadd_vv_i32m2(v0, v1, vl), vl);
        
        vint16m1x2_t s1 = __riscv_vlseg2e16_v_i16m1x2(coeff + 8, vl);
        v0 = __riscv_vget_v_i16m1x2_i16m1(s1, 0);
        b0 = __riscv_vmslt_vx_i16m1_b16(v0, 0, vl);
        v0 = __riscv_vneg_v_i16m1_m(b0, v0, vl);

        v1 = __riscv_vget_v_i16m1x2_i16m1(s1, 1);
        b1 = __riscv_vmslt_vx_i16m1_b16(v1, 0, vl);
        v1 = __riscv_vneg_v_i16m1_m(b1, v1, vl);

        sum_s32_1 =__riscv_vadd_vv_i32m2(sum_s32_1, __riscv_vwadd_vv_i32m2(v0, v1, vl), vl);

        length -= 16;
        coeff += 16;

    }while (length != 0);

    vint32m1_t scalar = __riscv_vmv_v_x_i32m1(0, vl);
    return __riscv_vmv_x_s_i32m1_i32(__riscv_vredsum_vs_i32m2_i32m1(sum_s32_0, scalar, vl)) + __riscv_vmv_x_s_i32m1_i32(    __riscv_vredsum_vs_i32m2_i32m1(sum_s32_1, scalar, vl));
}