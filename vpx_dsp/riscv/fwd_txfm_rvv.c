/*
 *  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/fwd_txfm.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <riscv_vector.h>

void vpx_fdct4x4_1_rvv(const int16_t *input, tran_low_t *output, int stride){

    const size_t vl = 4;
    vint16mf2_t a0, a1, a2, a3, b0, b1, c0;

    a0 = __riscv_vle16_v_i16mf2(input,vl);
    input += stride;
    a1 = __riscv_vle16_v_i16mf2(input,vl);
    input += stride;
    a2 = __riscv_vle16_v_i16mf2(input,vl);
    input += stride;
    a3 = __riscv_vle16_v_i16mf2(input,vl);

    b0 = __riscv_vadd_vv_i16mf2 (a0, a1, vl);
    b1 = __riscv_vadd_vv_i16mf2 (a2, a3, vl);

    c0 = __riscv_vadd_vv_i16mf2 (b0, b1, vl);

    vint16m1_t vzero =  __riscv_vmv_v_x_i16m1(0, vl);
    vint16m1_t vd = __riscv_vredsum_vs_i16mf2_i16m1(c0, vzero, vl);

    vd = __riscv_vsll_vx_i16m1 (vd, 1, vl);

    uint8_t mask[] = {1};
    vbool16_t vmask = __riscv_vlm_v_b16 (mask, vl);
    __riscv_vse16_v_i16m1_m(vmask, output, vd, vl);
}
