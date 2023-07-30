#include <assert.h>
#include "./vpx_dsp_rtcd.h"
#include <riscv_vector.h>

void vpx_comp_avg_pred_rvv(uint8_t *comp_pred, const uint8_t *pred, int width,
                         int height, const uint8_t *ref, int ref_stride) {
if(width > 8){
    printf("ddw 1\n");
    int x, y = height;
    size_t vl;
    vuint8m1_t vp,vr,vavg;
    do {
      for (x = 0; x < width; x += 16) {
        
        vl = 16;
        vp = __riscv_vle8_v_u8m1(pred + x, vl);
        vr = __riscv_vle8_v_u8m1(ref + x, vl);
        vavg = __riscv_vadd_vv_u8m1(vp, vr, vl);

        vavg = __riscv_vsrl_vx_u8m1(vavg, 1, vl);
        // const uint8x16_t p = vld1q_u8(pred + x);
        // const uint8x16_t r = vld1q_u8(ref + x);
        // const uint8x16_t avg = vrhaddq_u8(p, r);
        // vst1q_u8(comp + x, avg);

        __riscv_vse8_v_u8m1(comp_pred + x, vavg, vl);
        // __riscv_vse8_v_u8m1(out, vdata, vl);
      }
      comp_pred += width;
      pred += width;
      ref += ref_stride;
    } while (--y);
}
else if (width == 8) {
    printf("ddw 2\n");
    int i = width * height;
    size_t vl;
    vuint8m2_t vp, vr, vavg;
    
    vuint8m1_t vr_0, vr_1;
    do {
    vl = 16;  
    //   const uint8x16_t p = vld1q_u8(pred);
    vp = __riscv_vle8_v_u8m2(pred, vl);
    //   uint8x16_t r;

    //   const uint8x8_t r_0 = vld1_u8(ref);
    //   const uint8x8_t r_1 = vld1_u8(ref + ref_stride);
    vr_0 = __riscv_vle8_v_u8m1(ref, vl / 2);
    vr_1 = __riscv_vle8_v_u8m1(ref + ref_stride, vl / 2);

    vr = __riscv_vset_v_u8m1_u8m2 (vr, 0, vr_0);
    vr = __riscv_vset_v_u8m1_u8m2 (vr, 1, vr_1);

    //   r = vcombine_u8(r_0, r_1);
    //   ref += 2 * ref_stride;
    //   r = vrhaddq_u8(r, p);
    //   vst1q_u8(comp, r);

    vr = __riscv_vadd_vv_u8m2(vp, vr, vl);
    vr = __riscv_vsrl_vx_u8m2(vr, 1, vl);
    __riscv_vse8_v_u8m2(comp_pred, vr, vl);
    pred += 16;
    comp_pred += 16;
    i -= 16;
    } while (i);
  }
   else {
    printf("ddw 3\n");
    int i = width * height;
    size_t vl;
    vuint8m1_t vp, vr, vavg;
    
    // assert(width == 4);
    do {
         vp = __riscv_vle8_v_u8m1(pred, vl);

         vr = __riscv_vlse8_v_u8m1(ref, ref_stride, vl);
    //   const uint8x16_t p = vld1q_u8(pred);
    //   uint8x16_t r;

    //   r = load_unaligned_u8q(ref, ref_stride);
      ref += 4 * ref_stride;
    
    vr = __riscv_vadd_vv_u8m1(vp, vr, vl);

    vr = __riscv_vsrl_vx_u8m1(vr, 1, vl);
    __riscv_vse8_v_u8m1(comp_pred, vr, vl);

    //   r = vrhaddq_u8(r, p);
    //   vst1q_u8(comp, r);

    pred += 16;
    comp_pred += 16;
    i -= 16;
    } while (i);
  }
}