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


#include <stdio.h>
#include "common.h"

#if !defined(DOUBLE)
#define VSETVL(n) vsetvl_e32m2(n)
#define FLOAT_V_T vfloat32m2_t
#define VLEV_FLOAT vle32_v_f32m2
#define VLSEV_FLOAT vlse32_v_f32m2
#define VSEV_FLOAT vse32_v_f32m2
#define VBOOL_T vbool16_t
#define UINT_V_T vuint32m2_t
#define VID_V_UINT vid_v_u32m2
#define VMSLTU_VX_UINT vmsltu_vx_u32m2_b16
#define VMSEQ_VX_UINT vmseq_vx_u32m2_b16
#define VFMERGE_VFM_FLOAT  vfmerge_vfm_f32m2
#else
#define VSETVL(n) vsetvl_e64m2(n)
#define FLOAT_V_T vfloat64m2_t
#define VLEV_FLOAT vle64_v_f64m2
#define VLSEV_FLOAT vlse64_v_f64m2
#define VSEV_FLOAT vse64_v_f64m2
#define VBOOL_T     vbool32_t
#define UINT_V_T    vuint64m2_t
#define VID_V_UINT   vid_v_u64m2
#define VMSLTU_VX_UINT vmsltu_vx_u64m2_b32
#define VMSEQ_VX_UINT vmseq_vx_u64m2_b32
#define VFMERGE_VFM_FLOAT  vfmerge_vfm_f64m2
#endif

// Optimizes the implementation in ../arm64/tmmm_uncopy_sve_v1.c

int CNAME(BLASLONG m, BLASLONG n, FLOAT *a, BLASLONG lda, BLASLONG posX, BLASLONG posY, FLOAT *b){

    BLASLONG i, js, X;
    BLASLONG stride_lda = sizeof(FLOAT) * lda;
    FLOAT *ao;

    FLOAT_V_T vb, va1;
    size_t vl;

#ifdef UNIT
    VBOOL_T vbool_eq;
#endif

    VBOOL_T vbool_cmp;
    UINT_V_T vindex;

    for (js = n; js > 0; js -= vl)
    {
        vl = VSETVL(js);
        X = posX;

        if (posX <= posY) 
        {
            ao = a + posX + posY * lda;
        } 
        else 
        {
            ao = a + posY + posX * lda;
        }

        i = 0;
        do
        {
            if (X < posY) 
            {
                va1 = VLSEV_FLOAT(ao, stride_lda, vl);
                VSEV_FLOAT(b, va1, vl);

                ao ++;
                b += vl;
                X ++;
                i ++;
            } 
            else if (X > posY) 
            {
                ao += lda;
                b += vl;
                X ++;
                i ++;
            } 
            else 
            {
                vindex  = VID_V_UINT(vl);
                for (unsigned int j = 0; j < vl; j++) 
                {
                    va1 = VLSEV_FLOAT(ao, stride_lda, vl);
                    vbool_cmp = VMSLTU_VX_UINT(vindex, j, vl);
                    vb = VFMERGE_VFM_FLOAT(vbool_cmp, va1, ZERO, vl);
#ifdef UNIT
                    vbool_eq = VMSEQ_VX_UINT(vindex, j, vl);
                    vb =  VFMERGE_VFM_FLOAT(vbool_eq, vb, ONE, vl);
#endif
                    VSEV_FLOAT(b, vb, vl);
                    ao++;
                    b += vl;
                }

                X += vl;
                i += vl;
            }
        }while (i < m);

        posY += vl;
    }

    return 0;
}
