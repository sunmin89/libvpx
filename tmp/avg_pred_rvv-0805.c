#include <assert.h>
#include "./vpx_dsp_rtcd.h"
#include <riscv_vector.h>

void u16to8(uint16_t *a, uint8_t * b, const size_t count){
  int i = 0;
  for(i; i < count; i++){
    b[i] = a[i];
  }
}
uint32_t avg(uint32_t x1, uint32_t x2)
{
    return (x1 | x2) - (((x1 ^ x2) & 0xFEFEFEFEU) >> 1U);
}
vuint8m1_t test_avg_rvv(const uint8_t *a,const uint8_t *b, const size_t vl){
    
    // const uint8_t a[] = { 32, 97, 245, 223,170,67,122,223,242,77,59,194,71,235,3,186};
    // const uint8_t b[] = { 78, 27, 221, 2,184,189,140,248,118,112,129,178,121,232,86,24};

    //期望输出
    const uint8_t c[] = {55, 62, 233, 113, 177, 128, 131, 236, 180, 95, 94, 186, 96, 234, 45, 105};
    uint8_t d[16] = {0};

    //va -> vp, vb -> vr
    vuint32m1_t va, vb;
    // const size_t vl = 4;

    va = __riscv_vle32_v_u32m1(a, vl);
    vb = __riscv_vle32_v_u32m1(b, vl);

    vuint8m1_t vva, vvb;

    vva =  __riscv_vreinterpret_v_u32m1_u8m1(va);
    vvb =  __riscv_vreinterpret_v_u32m1_u8m1(vb);

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

    return v_u8;
}
int test_avg_c(void)
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
        printf("%d\n", c[i]);
    return 0; 
}
void vpx_comp_avg_pred_rvv(uint8_t *comp_pred, const uint8_t *pred, int width,
                         int height, const uint8_t *ref, int ref_stride) {
 printf("ddww rvv = %dx%d %d\n",width,height,ref_stride);                            
if(width > 8){
    // printf("ddw 1\n");
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
    //   printf("ddw rvv = %d\n",comp_pred[0]);
      comp_pred += width;
      pred += width;
      ref += ref_stride;
    } while (--y);
}
else if (width == 8) {
    // printf("ddw 2\n");
    int i = width * height;
    size_t vl;
    vuint8m2_t vr, vavg;
    vuint8m1_t vr_0, vr_1;
    do {
    vl = 16;  

    const vuint8m2_t vp = __riscv_vle8_v_u8m2(pred, vl);
    vr_0 = __riscv_vle8_v_u8m1(ref, vl / 2);
    vr_1 = __riscv_vle8_v_u8m1(ref + ref_stride, vl / 2);

    vr = __riscv_vset_v_u8m1_u8m2 (vr, 0, vr_0);
    vr = __riscv_vset_v_u8m1_u8m2 (vr, 1, vr_1);
    vr = __riscv_vadd_vv_u8m2(vp, vr, vl);
    
    ref += 2 * ref_stride;

    vr = __riscv_vsrl_vx_u8m2(vr, 1, vl);
    __riscv_vse8_v_u8m2(comp_pred, vr, vl);
    pred += 16;
    comp_pred += 16;
    i -= 16;
    } while (i);
  } else {
    // printf("ddw 3\n");
    int i = width * height;

    size_t vl = __riscv_vsetvl_e8m1(16);
    // vuint8m1_t vp, vr, vavg;
    
    assert(width == 4);
   
    if(width == ref_stride){
      do {
        // test_avg_rvv();
// alias ddw=target remote localhost:1234
        size_t vl = 16;
        vuint8m1_t vr = test_avg_rvv(pred, ref, vl / 4);
        // const vuint8m1_t vp  = __riscv_vle8_v_u8m1(pred, vl);
        // vuint16m2_t vv;//= __riscv_vlse8_v_u8m1(ref, ref_stride, vl);

        // vr = __riscv_vle8_v_u8m1(ref, vl);
        // vr = __riscv_vadd_vx_u8m1(vr, 1, vl);

        // vr = __riscv_vor_vx_u8m1(vr, 1, vl);
        // vr = __riscv_vxor_vx_u8m1(vr, 1, vl);
        // vv = __riscv_vwaddu_vv_u16m2(vr, vp, vl/2);
        // vv = __riscv_vsrl_vx_u16m2(vv, 1, vl/2);

        // vuint16m1_t vm = __riscv_vget_v_u16m2_u16m1(vv, 0);
        // __riscv_vse16_v_u16m1(comp_pred, vm, vl/2);

        // vuint8m2_t vvr = __riscv_vreinterpret_v_u16m2_u8m2(vv);

        // vuint8m1_t mm = __riscv_vget_v_u8m2_u8m1(vvr, 0);
        // vuint8m1_t nn = __riscv_vget_v_u8m2_u8m1(vvr, 1);
        // vr = __riscv_vadd_vx_u8m1(vr, 1, vl);
        ref += 4 * ref_stride;
      
        // vr = __riscv_vadd_vv_u8m1(__riscv_vsrl_vx_u8m1(vr, 1, vl),__riscv_vsrl_vx_u8m1(vp, 1, vl),vl);
        // __riscv_vse8_v_u8m1(comp_pred, vr, vl);

        // vuint16m2_t vv;//vle16_v_u16m1(data, 4);

        //实际输出
        // uint8_t d[] = {0};

        // vuint8m1_t vr,vp,vc;
        // size_t vl = 8;


        // vv = __riscv_vwaddu_vv_u16m2(vr, vp, vl);

        // vv = __riscv_vsrl_vx_u16m2(vv, 1, vl);

        // uint16_t aa[] = {0};
        
        // __riscv_vse16_v_u16m2(aa, vv, vl);

        // u16to8(aa,comp_pred,vl);
        __riscv_vse8_v_u8m1(comp_pred, vr, vl);

        pred += 16;
        comp_pred += 16;
        i -= 16; 

      }while (i);
  }
  else {
      do{
        /* code */
        uint32_t a;
        vuint32m1_t a_u32;
        size_t vl = 16;
        const vuint8m1_t vp  = __riscv_vle8_v_u8m1(pred, vl);
        a_u32 = __riscv_vlse32_v_u32m1(ref, ref_stride, vl / 4);

        vuint8m1_t vr = __riscv_vreinterpret_v_u32m1_u8m1(a_u32);

        //return vreinterpretq_u8_u32(a_u32);

        ref += 4 * ref_stride;

        vr = __riscv_vadd_vv_u8m1(__riscv_vsrl_vx_u8m1(vr, 1, vl),__riscv_vsrl_vx_u8m1(vp, 1, vl),vl);
        vr = __riscv_vadd_vx_u8m1(vr, 1, vl);

        __riscv_vse8_v_u8m1(comp_pred, vr, vl);

        pred += 16;
        comp_pred += 16;
        i -= 16;      
    } while (i);
  }
  }
}