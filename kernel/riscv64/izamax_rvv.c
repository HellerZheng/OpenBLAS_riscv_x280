/***************************************************************************
Copyright (c) 2022, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "common.h"

#if defined(DOUBLE)
#define VSETVL(n) vsetvl_e64m4(n)
#define VSETVL_MAX vsetvlmax_e64m4()
#define FLOAT_V_T vfloat64m4_t
#define FLOAT_V_T_M1 vfloat64m1_t
#define VLEV_FLOAT vle64_v_f64m4
#define VLSEV_FLOAT vlse64_v_f64m4
#define VLSEG_FLOAT vlseg2e64_v_f64m4
#define VLSSEG_FLOAT vlsseg2e64_v_f64m4
#define VFREDMAXVS_FLOAT vfredmax_vs_f64m4_f64m1
#define MASK_T vbool16_t
#define VMFLTVF_FLOAT vmflt_vf_f64m4_b16
#define VMFLTVV_FLOAT vmflt_vv_f64m4_b16
#define VMFGEVF_FLOAT vmfge_vf_f64m4_b16
#define VFMVVF_FLOAT vfmv_v_f_f64m4
#define VFMVVF_FLOAT_M1 vfmv_v_f_f64m1
#define VFABSV_FLOAT vfabs_v_f64m4
#define VFMAXVV_FLOAT vfmax_vv_f64m4
#define VFADDVV_FLOAT vfadd_vv_f64m4
#define VFIRSTM vfirst_m_b16
#define UINT_V_T vuint64m4_t
#define VIDV_MASK_UINT vid_v_u64m4_m
#define VIDV_UINT vid_v_u64m4
#define VADDVX_MASK_UINT vadd_vx_u64m4_m
#define VADDVX_UINT vadd_vx_u64m4
#define VMVVX_UINT vmv_v_x_u64m4
#define VFMVFS_FLOAT_M1 vfmv_f_s_f64m1_f64
#define VSLIDEDOWN_UINT vslidedown_vx_u64m4
#define VMVVXS_UINT vmv_x_s_u64m4_u64
#else
#define VSETVL(n) vsetvl_e32m4(n)
#define VSETVL_MAX vsetvlmax_e32m4()
#define FLOAT_V_T vfloat32m4_t
#define FLOAT_V_T_M1 vfloat32m1_t
#define VLEV_FLOAT vle32_v_f32m4
#define VLSEV_FLOAT vlse32_v_f32m4
#define VLSEG_FLOAT vlseg2e32_v_f32m4
#define VLSSEG_FLOAT vlsseg2e32_v_f32m4
#define VFREDMAXVS_FLOAT vfredmax_vs_f32m4_f32m1
#define MASK_T vbool8_t
#define VMFLTVF_FLOAT vmflt_vf_f32m4_b8
#define VMFLTVV_FLOAT vmflt_vv_f32m4_b8
#define VMFGEVF_FLOAT vmfge_vf_f32m4_b8
#define VFMVVF_FLOAT vfmv_v_f_f32m4
#define VFMVVF_FLOAT_M1 vfmv_v_f_f32m1
#define VFABSV_FLOAT vfabs_v_f32m4
#define VFMAXVV_FLOAT vfmax_vv_f32m4
#define VFADDVV_FLOAT vfadd_vv_f32m4
#define VFIRSTM vfirst_m_b8
#define UINT_V_T vuint32m4_t
#define VIDV_MASK_UINT vid_v_u32m4_m
#define VIDV_UINT vid_v_u32m4
#define VADDVX_MASK_UINT vadd_vx_u32m4_m
#define VADDVX_UINT vadd_vx_u32m4
#define VMVVX_UINT vmv_v_x_u32m4
#define VFMVFS_FLOAT_M1 vfmv_f_s_f32m1_f32
#define VSLIDEDOWN_UINT vslidedown_vx_u32m4
#define VMVVXS_UINT vmv_x_s_u32m4_u32
#endif

BLASLONG CNAME(BLASLONG n, FLOAT *x, BLASLONG inc_x)
{
    unsigned int max_index = 0;
    if (n <= 0 || inc_x <= 0) return(max_index);

    FLOAT_V_T vx0, vx1, v_max;
    UINT_V_T v_max_index;
    MASK_T mask;
  
    size_t vlmax = VSETVL_MAX;
    v_max_index = VMVVX_UINT(0, vlmax);
    v_max = VFMVVF_FLOAT(-1, vlmax);
    BLASLONG j=0;
    FLOAT maxf=0.0;

    if(inc_x == 1) {

        for (size_t vl; n > 0; n -= vl, x += vl*2, j += vl) {
            vl = VSETVL(n);

            VLSEG_FLOAT(&vx0, &vx1, x, vl);

            vx0 = VFABSV_FLOAT(vx0, vl);
            vx1 = VFABSV_FLOAT(vx1, vl);

            vx0 = VFADDVV_FLOAT(vx0, vx1, vl);

            //index where element greater than v_max
            mask = VMFLTVV_FLOAT(v_max, vx0, vl);
            v_max_index = VIDV_MASK_UINT(mask, v_max_index, vl);
            v_max_index = VADDVX_MASK_UINT(mask, v_max_index, v_max_index, j, vl);

            //update v_max and start_index j
            v_max = VFMAXVV_FLOAT(v_max, vx0, vl);
        }
    }
    else {
        
        BLASLONG stride_x = inc_x * sizeof(FLOAT) * 2;
        for (size_t vl; n > 0; n -= vl, x += vl*inc_x*2, j += vl) {
            vl = VSETVL(n);
        
            VLSSEG_FLOAT(&vx0, &vx1, x, stride_x, vl);
        
            vx0 = VFABSV_FLOAT(vx0, vl);
            vx1 = VFABSV_FLOAT(vx1, vl);
        
            vx0 = VFADDVV_FLOAT(vx0, vx1, vl);
        
            //index where element greater than v_max
            mask = VMFLTVV_FLOAT(v_max, vx0, vl);
            v_max_index = VIDV_MASK_UINT(mask, v_max_index, vl);
            v_max_index = VADDVX_MASK_UINT(mask, v_max_index, v_max_index, j, vl);
        
            //update v_max and start_index j
            v_max = VFMAXVV_FLOAT(v_max, vx0, vl);
        }

    }
    FLOAT_V_T_M1 v_res, v_z0;
    v_res = VFMVVF_FLOAT_M1(0, vlmax);
    v_z0 = VFMVVF_FLOAT_M1(0, vlmax);

    v_res = VFREDMAXVS_FLOAT(v_res, v_max, v_z0, vlmax);
    maxf = VFMVFS_FLOAT_M1(v_res);
    mask = VMFGEVF_FLOAT(v_max, maxf, vlmax);
    max_index = VFIRSTM(mask, vlmax);
    
    v_max_index = VSLIDEDOWN_UINT(v_max_index, v_max_index, max_index, vlmax);
    max_index = VMVVXS_UINT(v_max_index);

    return(max_index+1);
}
