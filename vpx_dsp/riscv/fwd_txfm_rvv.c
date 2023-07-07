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

void vpx_fdct4x4_1_rvv(const int16_t *input, tran_low_t *output, int stride) {

    printf("ddw vpx_fdct4x4_1_c stride = %d, sizeof(input) = %d\n",stride,sizeof(input));
    const size_t vl = 4;
    int16_t ones[4] = {1,1,1,1};
    int16_t zeros[4] = {0};

    vuint16m1_t output_v = __riscv_vle16_v_u16m1(zeros,vl);;
    vuint16m1_t input_v;
    vuint16m1_t ones_v = __riscv_vle16_v_u16m1(ones,vl);
    vuint16m1_t zeros_v = __riscv_vle16_v_u16m1(zeros,vl);
    tran_low_t *output_tmp =  (int16_t *)malloc(vl * sizeof(int16_t));;

    int r, c;
    tran_low_t sum = 0;

    for (r = 0; r < 4; ++r){
        input_v = __riscv_vle16_v_u16m1(input + r * stride, vl);
        output_v = __riscv_vmacc_vv_u16m1 (output_v, input_v, ones_v,vl);
    }

    output_v = __riscv_vredsum_vs_u16m1_u16m1 (output_v, zeros_v, vl);
    __riscv_vse16_v_u16m1(output_tmp, output_v, vl);
    output[0] = output_tmp[0] * 2;

    //printf("ddw vpx_fdct4x4_1_c\n");
}
