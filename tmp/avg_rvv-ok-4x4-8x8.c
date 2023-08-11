
#include <assert.h>
#include "./vpx_dsp_rtcd.h"
#include <riscv_vector.h>

#include "./vpx_dsp_rtcd.h"
#include "./vpx_config.h"

//AverageTest
uint32_t vpx_avg_4x4_rvv(const uint8_t *a, int a_stride) {


//a_stride = 4
// uint8_t aa[]={32, 97, 245, 223, 225, 200, 197, 124, 
//              225, 200, 197, 124
//              45, 144, 74, 200, 244, 234, 245, 118};

//va = 32, 97, 245, 223, 225, 200, 197, 124
//vb = 45, 144, 74, 200, 244, 234, 245, 118
//return 2647/16 = 165

//   const uint8x16_t b = load_unaligned_u8q(a, a_stride);
const size_t vl = 16;
vuint32m1_t a_u32 = __riscv_vlse32_v_u32m1(a, a_stride, vl / 4);

vuint8m1_t vv =  __riscv_vreinterpret_v_u32m1_u8m1(a_u32);

// // vuint16m1_t test_vwredsumu_vs_u8m1_u16m1(vuint8m1_t vector, vuint16m1_t scalar, size_t vl) {
// vuint16m1_t scalarm = __riscv_vmv_v_x_u16m1(0,vl);
// vuint16m1_t vm  = __riscv_vwredsumu_vs_u8m1_u16m1(vv, scalarm, vl);
// vm = __riscv_vsrl_vx_u16m1(vm,4, vl);

// // vuint16m1_t scalarm = __riscv_vmv_v_x_u32m1(0,vl/4);
// return (__riscv_vmv_x_s_u16m1_u16(vm) + 1);

// vuint32m2_t a_u32 = __riscv_vlse32_v_u32m2(a, a_stride, vl / 8);
// vuint8m1_t vv =  __riscv_vreinterpret_v_u32m1_u8m1(a_u32);

uint8_t tmp[vl];
// 
__riscv_vse8_v_u8m1(tmp, vv, vl);

vuint8m1_t va = __riscv_vle8_v_u8m1(tmp, vl/2);
vuint8m1_t vb = __riscv_vle8_v_u8m1(tmp + vl/2, vl/2);

// vuint16m1_t scalarm = __riscv_vmv_v_x_u16m1(0,vl);
// vuint8m1_t vc = __riscv_vaaddu_vv_u8m1(va, vb, __RISCV_VXRM_RNU, vl);
// vuint16m2_t vc = __riscv_vwaddu_vv_u16m2(va, vb, vl);
// vuint16m1_t vm  = __riscv_vwredsumu_vs_u8m1_u16m1(vc, scalarm, vl);
// vm = __riscv_vsrl_vx_u16m1(vm,4, vl);
// __riscv_vmv_x_s_u16m1_u16(vm);

// $5 = {[0] = 32, [1] = 97, [2] = 245, [3] = 223, [4] = 225, [5] = 200, 
//   [6] = 197, [7] = 124}
// 26	  const uint16x8_t c = vaddl_u8(aa, bb);
// $6 = {[0] = 45, [1] = 144, [2] = 74, [3] = 200, [4] = 244, [5] = 234, 
//   [6] = 245, [7] = 118}
// $7 = {[0] = 32, [1] = 97, [2] = 245, [3] = 223, [4] = 225, [5] = 200, 
//   [6] = 197, [7] = 124, [8] = 45, [9] = 144, [10] = 74, [11] = 200, 
//   [12] = 244, [13] = 234, [14] = 245, [15] = 118}

// $8 = {[0] = 77, [1] = 241, [2] = 319, [3] = 423, [4] = 469, [5] = 434, 
//   [6] = 442, [7] = 242}

// $10 = 2647

// vuint8m1_t vr = __riscv_vreinterpret_v_u32m1_u8m1(a_u32);
//vr -> 8 * 16 => 16 *8

// __riscv_vreinterpret_v_u8m1_u16m1(src);

vuint16m2_t vc = __riscv_vwaddu_vv_u16m2(va, vb, vl);

// vuint16m1_t test_vredsum_vs_u16m2_u16m1(vuint16m2_t vector, vuint16m1_t scalar, size_t vl) {

//11 ok
vuint16m1_t scalar = __riscv_vmv_v_x_u16m1(0,vl/2);
vuint16m1_t vd = __riscv_vredsum_vs_u16m2_u16m1(vc, scalar, vl/2);

vd = __riscv_vadd_vx_u16m1(vd,1<<3, vl/2);
vd = __riscv_vsrl_vx_u16m1(vd,4, vl/2);
// vd = __riscv_vadd_vx_u16m1(vd, 1, vl/2);

// printf("\n");
return __riscv_vmv_x_s_u16m1_u16(vd);

// vd = __riscv_vadd_vx_u16m1(vd, (1 << 3), vl);
// vd = __riscv_vsrl_vx_u16m1(vd,1, vl);


// }

// vuint32m1_t scalar = __riscv_vmv_v_x_u32m1(0, vl/4);
// vuint32m1_t test_vwredsumu_vs_u16m2_u32m1(vuint16m2_t vector, vuint32m1_t scalar, size_t vl) {
// vuint32m1_t b_u32 = __riscv_vwredsumu_vs_u16m2_u32m1(vc, scalar, vl/4);

// b_u32 = __riscv_vadd_vx_u32m1(b_u32, (1 << 3), vl);
// b_u32 = __riscv_vsrl_vx_u32m1(b_u32, 4,vl);
// }
// vaddl_u8 -> vaddw
//   const uint16x8_t c = vaddl_u8(vget_low_u8(b), vget_high_u8(b));
// (uint8x8_t a, uint8x8_t b) -> uint16x8_t

//vget_low_u8(b) uint8x16_t -> uint8x8_t
// Duplicate vector element to vector or 
// scalar. This instruction duplicates the vector element at the specified element index in the source SIMD&FP register into a scalar or each element in a vector, 
// and writes the result to the destination SIMD&FP register.

 
// return __riscv_vmv_x_s_u32m1_u32(b_u32);//(horizontal_add_uint16x8(c) + (1 << 3)) >> 4;
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