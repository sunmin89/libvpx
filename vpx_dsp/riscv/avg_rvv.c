
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

void vpx_int_pro_row_rvv(int16_t hbuf[16], uint8_t const *ref,
                          const int ref_stride, const int height) {
int i;
const size_t vl = 16;

vuint8m1_t r0h, r0l, r1h, r1l, r2h, r2l, r3h, r3l;
vuint16m2_t  sum_lo_0, sum_lo_1, sum_hi_0, sum_hi_1, tmp_lo_0, tmp_lo_1, tmp_hi_0, tmp_hi_1;
vint16m2_t avg_lo, avg_hi;

const int norm_factor = (height >> 5) + 3;

const vuint16m2_t neg_norm_factor = __riscv_vmv_v_x_u16m2(-norm_factor, vl);

assert(height >= 4 && height % 4 == 0);

r0l = __riscv_vle8_v_u8m1(ref, vl/2);
r0h = __riscv_vle8_v_u8m1(ref + vl / 2, vl /2);

r1l = __riscv_vle8_v_u8m1(ref + 1 * ref_stride, vl/2);
r1h = __riscv_vle8_v_u8m1(ref + 1 * ref_stride + vl / 2, vl /2);

r2l = __riscv_vle8_v_u8m1(ref + 2 * ref_stride, vl/2);
r2h = __riscv_vle8_v_u8m1(ref + 2 * ref_stride + vl / 2, vl /2);

r3l = __riscv_vle8_v_u8m1(ref + 3 * ref_stride, vl/2);
r3h = __riscv_vle8_v_u8m1(ref + 3 * ref_stride + vl / 2, vl /2);

//sum_lo = 
sum_lo_0 = __riscv_vwaddu_vv_u16m2(r0l, r1l, vl / 2);
// sum_lo =  __riscv_vset_v_u16m2_u16m2x2 (sum_lo, 0, lo_0);

sum_lo_1 = __riscv_vwaddu_vv_u16m2(r2l, r3l, vl / 2);
// sum_lo =  __riscv_vset_v_u16m2_u16m2x2 (sum_lo, 1, lo_1);


sum_hi_0 = __riscv_vwaddu_vv_u16m2(r0h, r1h, vl / 2);
// sum_hi =  __riscv_vset_v_u16m2_u16m2x2 (sum_hi, 0, hi_0);

sum_hi_1 = __riscv_vwaddu_vv_u16m2(r2h, r3h, vl / 2);
// sum_hi =  __riscv_vset_v_u16m2_u16m2x2 (sum_hi, 1, hi_1);

for (i = 4; i < height; i += 4) {
    r0l = __riscv_vle8_v_u8m1(ref, vl/2);
    r0h = __riscv_vle8_v_u8m1(ref + vl / 2, vl /2);

    r1l = __riscv_vle8_v_u8m1(ref + 1 * ref_stride, vl/2);
    r1h = __riscv_vle8_v_u8m1(ref + 1 * ref_stride + vl / 2, vl /2);

    r2l = __riscv_vle8_v_u8m1(ref + 2 * ref_stride, vl/2);
    r2h = __riscv_vle8_v_u8m1(ref + 2 * ref_stride + vl / 2, vl /2);

    r3l = __riscv_vle8_v_u8m1(ref + 3 * ref_stride, vl/2);
    r3h = __riscv_vle8_v_u8m1(ref + 3 * ref_stride + vl / 2, vl /2);

    tmp_lo_0 = __riscv_vwaddu_vv_u16m2(r0l, r1l, vl / 2);
    // tmp_lo =  __riscv_vset_v_u16m2_u16m2x2 (tmp_lo, 0, lo_0);

    tmp_lo_1 = __riscv_vwaddu_vv_u16m2(r2l, r3l, vl / 2);
    // tmp_lo =  __riscv_vset_v_u16m2_u16m2x2 (tmp_lo, 1, lo_1);

    tmp_hi_0 = __riscv_vwaddu_vv_u16m2(r0h, r1h, vl / 2);
    // tmp_hi =  __riscv_vset_v_u16m2_u16m2x2 (tmp_hi, 0, hi_0);

    tmp_hi_1 = __riscv_vwaddu_vv_u16m2(r2h, r3h, vl / 2);
    // tmp_hi =  __riscv_vset_v_u16m2_u16m2x2 (tmp_hi, 1, hi_1);


    sum_lo_0 = __riscv_vadd_vv_u16m2(sum_lo_0, tmp_lo_0, vl / 2);
    sum_lo_1 = __riscv_vadd_vv_u16m2(sum_lo_1, tmp_lo_1, vl / 2);

    sum_hi_0 = __riscv_vadd_vv_u16m2(sum_hi_0, tmp_hi_0, vl / 2);
    sum_hi_1 = __riscv_vadd_vv_u16m2(sum_hi_1, tmp_hi_1, vl / 2);

    ref += 4 * ref_stride;
    }

    sum_lo_0 = __riscv_vadd_vv_u16m2(sum_lo_0, sum_lo_1, vl / 2);
    sum_hi_0 = __riscv_vadd_vv_u16m2(sum_hi_0, sum_hi_1, vl / 2);

    avg_lo = __riscv_vreinterpret_v_u16m2_i16m2(sum_lo_0);
    avg_lo = __riscv_vsll_vv_i16m2(avg_lo, neg_norm_factor, vl / 2);

    avg_hi = __riscv_vreinterpret_v_u16m2_i16m2(sum_hi_0);
    avg_hi = __riscv_vsll_vv_i16m2(avg_hi, neg_norm_factor, vl / 2);

    __riscv_vse16_v_i16m2(hbuf, avg_lo, vl / 2);
    __riscv_vse16_v_i16m2(hbuf + 8, avg_hi, vl / 2);

}

/**
 * make -j4 test_libvpx && 
 * qemu-riscv64 -cpu rv64,v=true,vlen=512,vext_spec=v1.0  -L $RISCV/sysroot ./test_libvpx --gtest_filter='RVV/IntProRowTest.Random/0'
*/
int16_t vpx_int_pro_col_neon(uint8_t const *ref, const int width) {

}                 
