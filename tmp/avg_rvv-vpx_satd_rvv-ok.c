
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
    const size_t vl = 8;
    vint32m2_t sum_s32_0 = __riscv_vmv_v_x_i32m2(0,vl/2);//,__riscv_vmv_v_x_u32m2(0,vl/2)};
    vint32m2_t sum_s32_1 = __riscv_vmv_v_x_i32m2(0,vl/2);
    do {
        vint16m1_t abs0, abs1;

        // const vint16m1_t s0 = __riscv_vle16_v_i16m1(coeff, vl);
        // const vint16m1_t s1 = __riscv_vle16_v_i16m1(coeff + 8, vl);

        // s0 = __riscv_vabs_v
        //judge if less than zero
        // vbool16_t s0b = __riscv_vmslt_vx_i16m1_b16 (s0, 0, vl);
        //vzext.vf v combine
        //LMUL truncation and LMUL extension functions
        //vlmul_ext
        //vlmul_trunc
        // abs0 = __riscv_vneg_v_i16m1_m(s0b, s0, vl);

        // vbool16_t s1b = vmslt_vx_i16m1_b16 (s1, 0, vl);

        //neg_m
        // sum_s32_0 = __riscv_vwadd_wv_i32m2(sum_s32_0, abs0, vl);

        //
        // sum_32_0[0] = sum_s32_0[0] + (abs0 + abs1)
        // sum_32_0[1] = sum_s32_0[1] + (abs2 + abs3)
        // sum_32_0[2] = sum_s32_0[2] + (abs4 + abs5)
        // sum_32_0[3] = sum_s32_0[3] + (abs6 + abs7)

        // 01 23 45 67

        //0 2 4 6
        //1 3 5 7
        // if(1){
            vint16m1_t v0, v1;// = __riscv_vlse16_v_i16m1(coeff, 2, vl / 2);
            // vint16m1_t v1 = __riscv_vlse16_v_i16m1(coeff + 1, 2, vl / 2);

            // vbool16_t b0 = __riscv_vmslt_vx_i16m1_b16 (v0, 0, vl / 2);

            vint16m1x2_t v2 = __riscv_vlseg2e16_v_i16m1x2(coeff, vl / 2);

            //odd index
            v0 = __riscv_vget_v_i16m1x2_i16m1(v2, 0);

            //even index
            v1 = __riscv_vget_v_i16m1x2_i16m1(v2, 1);

            vbool16_t b0 = __riscv_vmslt_vx_i16m1_b16(v0, 0, vl/2);
            v0 = __riscv_vneg_v_i16m1_m(b0, v0, vl/2);

            vbool16_t b1 = __riscv_vmslt_vx_i16m1_b16(v1, 0, vl/2);
            v1 = __riscv_vneg_v_i16m1_m(b1, v1, vl/2);

            vint32m2_t aa = __riscv_vwadd_vv_i32m2(v0, v1, vl/2);

            // sum_s32_0 = __riscv_vwadd_wv_i32m2(sum_s32_0, abs0, vl);
            sum_s32_0 =__riscv_vadd_vv_i32m2(sum_s32_0, aa, vl);
            
            v2 = __riscv_vlseg2e16_v_i16m1x2(coeff + 8, vl / 2);
            //odd index
            v0 = __riscv_vget_v_i16m1x2_i16m1(v2, 0);

            //even index
            v1 = __riscv_vget_v_i16m1x2_i16m1(v2, 1);

            b0 = __riscv_vmslt_vx_i16m1_b16(v0, 0, vl/2);
            v0 = __riscv_vneg_v_i16m1_m(b0, v0, vl/2);

            b1 = __riscv_vmslt_vx_i16m1_b16(v1, 0, vl/2);
            v1 = __riscv_vneg_v_i16m1_m(b1, v1, vl/2);

            aa = __riscv_vwadd_vv_i32m2(v0, v1, vl/2);

            // sum_s32_0 = __riscv_vwadd_wv_i32m2(sum_s32_0, abs0, vl);
            sum_s32_1 =__riscv_vadd_vv_i32m2(sum_s32_1, aa, vl);

            printf("\n");
            //vzext.vf v combine
            //LMUL truncation and LMUL extension functions
            //vlmul_ext
            //vlmul_trunc
            // abs0 = __riscv_vneg_v_i16m1_m(s0b, s0, vl);
        // }
        // }


        //0 1 2 3 4 5 6 7

        // __riscv_vredsum_vs_i32m2_i32m1(vector, scalar, vl);

        // vbool16_t s1b = __riscv_vmslt_vx_i16m1_b16 (s1, 0, vl);
        // abs1 = __riscv_vneg_v_i16m1_m(s1b, s1, vl);
        // sum_s32[1] = __riscv_vwadd_wv_i32m2(sum_s32[1], abs1, vl);

        


        length -= 16;
        coeff += 16;

    }while (length != 0);

    // vint64m4_t vd = __riscv_vwadd_vv_i64m4(sum_s32_0, sum_s32_0,vl/2);
    // vint32m1_t vredsum_vs_i32m2_i32m1 (vint32m1_t dest, vint32m2_t, vl)

    vint32m1_t scalar = __riscv_vmv_v_x_i32m1(0, vl/2);

    vint32m1_t aaa = __riscv_vredsum_vs_i32m2_i32m1(sum_s32_0, scalar, vl/2);
    
    vint32m1_t bbb = __riscv_vredsum_vs_i32m2_i32m1(sum_s32_1, scalar, vl/2);


    return __riscv_vmv_x_s_i32m1_i32(aaa) + __riscv_vmv_x_s_i32m1_i32(bbb);
}