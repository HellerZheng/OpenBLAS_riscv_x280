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

#if !defined(DOUBLE)
#define VSETVL(n) vsetvl_e32m2(n)
#define FLOAT_V_T vfloat32m2_t
#define VLEV_FLOAT vle32_v_f32m2
#define VSEV_FLOAT vse32_v_f32m2
#define VLSEG2_FLOAT vlseg2e32_v_f32m2
#define VSSEG2_FLOAT vsseg2e32_v_f32m2
#define VFMVVF_FLOAT vfmv_v_f_f32m2
#define VFMACCVF_FLOAT vfmacc_vf_f32m2
#define VFNMSACVF_FLOAT vfnmsac_vf_f32m2
#else
#define VSETVL(n) vsetvl_e64m2(n)
#define FLOAT_V_T vfloat64m2_t
#define VLEV_FLOAT vle64_v_f64m2
#define VSEV_FLOAT vse64_v_f64m2
#define VLSEG2_FLOAT vlseg2e64_v_f64m2
#define VSSEG2_FLOAT vsseg2e64_v_f64m2
#define VFMVVF_FLOAT vfmv_v_f_f64m2
#define VFMACCVF_FLOAT vfmacc_vf_f64m2
#define VFNMSACVF_FLOAT vfnmsac_vf_f64m2
#endif

#if defined(NN) || defined(NT) || defined(TN) || defined(TT)
#define OP_rr       VFMACCVF_FLOAT
#define OP_ir       VFMACCVF_FLOAT
#define OP_ii       VFNMSACVF_FLOAT
#define OP_ri       VFMACCVF_FLOAT
#elif defined(NR) || defined(NC) || defined(TR) || defined(TC)
#define OP_rr       VFMACCVF_FLOAT
#define OP_ir       VFMACCVF_FLOAT
#define OP_ii       VFMACCVF_FLOAT
#define OP_ri       VFNMSACVF_FLOAT
#elif defined(RN) || defined(RT) || defined(CN) || defined(CT)
#define OP_rr       VFMACCVF_FLOAT
#define OP_ir       VFNMSACVF_FLOAT
#define OP_ii       VFMACCVF_FLOAT
#define OP_ri       VFMACCVF_FLOAT
#elif defined(RR) || defined(RC) || defined(CR) || defined(CC)
#define OP_rr       VFMACCVF_FLOAT
#define OP_ir       VFNMSACVF_FLOAT
#define OP_ii       VFNMSACVF_FLOAT
#define OP_ri       VFNMSACVF_FLOAT
#endif

int CNAME(BLASLONG bm,BLASLONG bn,BLASLONG bk,FLOAT alphar,FLOAT alphai,FLOAT* ba,FLOAT* bb,FLOAT* C,BLASLONG ldc
#ifdef	TRMMKERNEL
		, BLASLONG offset
#endif
		)
{
    BLASLONG i,j,k;
    FLOAT *C0, *C1, *C2, *C3, *ptrba,*ptrbb;

    FLOAT_V_T va0, va1, va2, va3, va4, va5, va6, va7;
    FLOAT_V_T vres0, vres1, vres2, vres3, vres4, vres5, vres6, vres7;

    //fprintf(stderr, "%s, bn=%ld bm=%ld bk=%ld alphar=%f alphai=%f ldc=%ld\n", __FUNCTION__, bn, bm, bk, alphar, alphai, ldc); // Debug

    size_t vl;
    for (j = bn/4; j > 0; j--)
    {
        C0 = C;
        C1 = C0 + 2 * ldc;
        C2 = C1 + 2 * ldc;
        C3 = C2 + 2 * ldc;
        ptrba = ba;
        for (i = bm; i > 0; i -= vl)
        {
            vl = VSETVL(i);
            ptrbb = bb;

            vres0 = VFMVVF_FLOAT(0.0, vl);
            vres1 = VFMVVF_FLOAT(0.0, vl);
            vres2 = VFMVVF_FLOAT(0.0, vl);
            vres3 = VFMVVF_FLOAT(0.0, vl);
            vres4 = VFMVVF_FLOAT(0.0, vl);
            vres5 = VFMVVF_FLOAT(0.0, vl);
            vres6 = VFMVVF_FLOAT(0.0, vl);
            vres7 = VFMVVF_FLOAT(0.0, vl);

            for (k = bk/4; k > 0; k--)
            {
                VLSEG2_FLOAT(&va0, &va1, ptrba, vl);
                ptrba += vl*2;

                VLSEG2_FLOAT(&va2, &va3, ptrba, vl);
                ptrba += vl*2;

                vres0 =  OP_rr(vres0, *(ptrbb + 0), va0, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va1, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va1, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va0, vl);

                vres2 =  OP_rr(vres2, *(ptrbb + 2), va0, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va1, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va1, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va0, vl);

                vres4 =  OP_rr(vres4, *(ptrbb + 4), va0, vl);
                vres5 =  OP_ir(vres5, *(ptrbb + 4), va1, vl);
                vres4 =  OP_ii(vres4, *(ptrbb + 5), va1, vl);
                vres5 =  OP_ri(vres5, *(ptrbb + 5), va0, vl);

                vres6 =  OP_rr(vres6, *(ptrbb + 6), va0, vl);
                vres7 =  OP_ir(vres7, *(ptrbb + 6), va1, vl);
                vres6 =  OP_ii(vres6, *(ptrbb + 7), va1, vl);
                vres7 =  OP_ri(vres7, *(ptrbb + 7), va0, vl);

                ptrbb += 8;

                VLSEG2_FLOAT(&va4, &va5, ptrba, vl);
                ptrba += vl*2;

                vres0 =  OP_rr(vres0, *(ptrbb + 0), va2, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va3, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va3, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va2, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va2, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va3, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va3, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va2, vl);
                
                vres4 =  OP_rr(vres4, *(ptrbb + 4), va2, vl);
                vres5 =  OP_ir(vres5, *(ptrbb + 4), va3, vl);
                vres4 =  OP_ii(vres4, *(ptrbb + 5), va3, vl);
                vres5 =  OP_ri(vres5, *(ptrbb + 5), va2, vl);
                
                vres6 =  OP_rr(vres6, *(ptrbb + 6), va2, vl);
                vres7 =  OP_ir(vres7, *(ptrbb + 6), va3, vl);
                vres6 =  OP_ii(vres6, *(ptrbb + 7), va3, vl);
                vres7 =  OP_ri(vres7, *(ptrbb + 7), va2, vl);
                
                ptrbb += 8;

                VLSEG2_FLOAT(&va6, &va7, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va4, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va5, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va5, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va4, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va4, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va5, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va5, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va4, vl);
                
                vres4 =  OP_rr(vres4, *(ptrbb + 4), va4, vl);
                vres5 =  OP_ir(vres5, *(ptrbb + 4), va5, vl);
                vres4 =  OP_ii(vres4, *(ptrbb + 5), va5, vl);
                vres5 =  OP_ri(vres5, *(ptrbb + 5), va4, vl);
                
                vres6 =  OP_rr(vres6, *(ptrbb + 6), va4, vl);
                vres7 =  OP_ir(vres7, *(ptrbb + 6), va5, vl);
                vres6 =  OP_ii(vres6, *(ptrbb + 7), va5, vl);
                vres7 =  OP_ri(vres7, *(ptrbb + 7), va4, vl);
                ptrbb += 8;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va6, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va7, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va7, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va6, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va6, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va7, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va7, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va6, vl);
                
                vres4 =  OP_rr(vres4, *(ptrbb + 4), va6, vl);
                vres5 =  OP_ir(vres5, *(ptrbb + 4), va7, vl);
                vres4 =  OP_ii(vres4, *(ptrbb + 5), va7, vl);
                vres5 =  OP_ri(vres5, *(ptrbb + 5), va6, vl);
                
                vres6 =  OP_rr(vres6, *(ptrbb + 6), va6, vl);
                vres7 =  OP_ir(vres7, *(ptrbb + 6), va7, vl);
                vres6 =  OP_ii(vres6, *(ptrbb + 7), va7, vl);
                vres7 =  OP_ri(vres7, *(ptrbb + 7), va6, vl);

                ptrbb += 8;
            }

            for (k = (bk & 3); k > 0; k--)
            {
                VLSEG2_FLOAT(&va0, &va1, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va0, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va1, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va1, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va0, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va0, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va1, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va1, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va0, vl);
                
                vres4 =  OP_rr(vres4, *(ptrbb + 4), va0, vl);
                vres5 =  OP_ir(vres5, *(ptrbb + 4), va1, vl);
                vres4 =  OP_ii(vres4, *(ptrbb + 5), va1, vl);
                vres5 =  OP_ri(vres5, *(ptrbb + 5), va0, vl);
                
                vres6 =  OP_rr(vres6, *(ptrbb + 6), va0, vl);
                vres7 =  OP_ir(vres7, *(ptrbb + 6), va1, vl);
                vres6 =  OP_ii(vres6, *(ptrbb + 7), va1, vl);
                vres7 =  OP_ri(vres7, *(ptrbb + 7), va0, vl);

                ptrbb += 8;
            }

            VLSEG2_FLOAT(&va0, &va1, C0, vl);
            VLSEG2_FLOAT(&va2, &va3, C1, vl);

            va0 =  VFMACCVF_FLOAT(va0, alphar, vres0, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphar, vres1, vl);
            va0 = VFNMSACVF_FLOAT(va0, alphai, vres1, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphai, vres0, vl);
            VSSEG2_FLOAT(C0, va0, va1, vl);

            va2 =  VFMACCVF_FLOAT(va2, alphar, vres2, vl);
            va3 =  VFMACCVF_FLOAT(va3, alphar, vres3, vl);
            va2 = VFNMSACVF_FLOAT(va2, alphai, vres3, vl);
            va3 =  VFMACCVF_FLOAT(va3, alphai, vres2, vl);
            VSSEG2_FLOAT(C1, va2, va3, vl);

            VLSEG2_FLOAT(&va0, &va1, C2, vl);
            VLSEG2_FLOAT(&va2, &va3, C3, vl);

            va0 =  VFMACCVF_FLOAT(va0, alphar, vres4, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphar, vres5, vl);
            va0 = VFNMSACVF_FLOAT(va0, alphai, vres5, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphai, vres4, vl);
            VSSEG2_FLOAT(C2, va0, va1, vl);

            va2 =  VFMACCVF_FLOAT(va2, alphar, vres6, vl);
            va3 =  VFMACCVF_FLOAT(va3, alphar, vres7, vl);
            va2 = VFNMSACVF_FLOAT(va2, alphai, vres7, vl);
            va3 =  VFMACCVF_FLOAT(va3, alphai, vres6, vl);
            VSSEG2_FLOAT(C3, va2, va3, vl);

            C0 += vl * 2;
            C1 += vl * 2;
            C2 += vl * 2;
            C3 += vl * 2;
        }

        bb += (bk << 3);
        C  += (ldc << 3);
    }

    if (bn & 2)
    {
        C0 = C;
        C1 = C0 + 2 * ldc;
        ptrba = ba;
        for (i = bm; i > 0; i -= vl)
        {
            vl = VSETVL(i);
            ptrbb = bb;

            vres0 = VFMVVF_FLOAT(0.0, vl);
            vres1 = VFMVVF_FLOAT(0.0, vl);
            vres2 = VFMVVF_FLOAT(0.0, vl);
            vres3 = VFMVVF_FLOAT(0.0, vl);

            for (k = bk/4; k > 0; k--)
            {
                VLSEG2_FLOAT(&va0, &va1, ptrba, vl);
                ptrba += vl*2;
                VLSEG2_FLOAT(&va2, &va3, ptrba, vl);
                ptrba += vl*2;

                vres0 =  OP_rr(vres0, *(ptrbb + 0), va0, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va1, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va1, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va0, vl);

                vres2 =  OP_rr(vres2, *(ptrbb + 2), va0, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va1, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va1, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va0, vl);

                ptrbb += 4;

                VLSEG2_FLOAT(&va4, &va5, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va2, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va3, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va3, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va2, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va2, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va3, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va3, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va2, vl);
                
                ptrbb += 4;

                VLSEG2_FLOAT(&va6, &va7, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va4, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va5, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va5, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va4, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va4, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va5, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va5, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va4, vl);
                
                ptrbb += 4;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va6, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va7, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va7, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va6, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va6, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va7, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va7, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va6, vl);
                
                ptrbb += 4;
            }

            for (k = (bk & 3); k > 0; k--)
            {
                VLSEG2_FLOAT(&va0, &va1, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va0, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va1, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va1, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va0, vl);
                
                vres2 =  OP_rr(vres2, *(ptrbb + 2), va0, vl);
                vres3 =  OP_ir(vres3, *(ptrbb + 2), va1, vl);
                vres2 =  OP_ii(vres2, *(ptrbb + 3), va1, vl);
                vres3 =  OP_ri(vres3, *(ptrbb + 3), va0, vl);
                
                ptrbb += 4;
            }

            VLSEG2_FLOAT(&va0, &va1, C0, vl);
            VLSEG2_FLOAT(&va2, &va3, C1, vl);

            va0 =  VFMACCVF_FLOAT(va0, alphar, vres0, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphar, vres1, vl);
            va0 = VFNMSACVF_FLOAT(va0, alphai, vres1, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphai, vres0, vl);
            VSSEG2_FLOAT(C0, va0, va1, vl);

            va2 =  VFMACCVF_FLOAT(va2, alphar, vres2, vl);
            va3 =  VFMACCVF_FLOAT(va3, alphar, vres3, vl);
            va2 = VFNMSACVF_FLOAT(va2, alphai, vres3, vl);
            va3 =  VFMACCVF_FLOAT(va3, alphai, vres2, vl);
            VSSEG2_FLOAT(C1, va2, va3, vl);

            C0 += vl * 2;
            C1 += vl * 2;
        }

        bb += (bk << 2);
        C  += (ldc << 2);
    }

    if (bn & 1)
    {
        C0 = C;
        ptrba = ba;
        for (i = bm; i > 0; i -= vl)
        {
            vl = VSETVL(i);
            ptrbb = bb;

            vres0 = VFMVVF_FLOAT(0.0, vl);
            vres1 = VFMVVF_FLOAT(0.0, vl);

            for (k = bk/4; k > 0; k--)
            {
                VLSEG2_FLOAT(&va0, &va1, ptrba, vl);
                ptrba += vl*2;
                VLSEG2_FLOAT(&va2, &va3, ptrba, vl);
                ptrba += vl*2;

                vres0 =  OP_rr(vres0, *(ptrbb + 0), va0, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va1, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va1, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va0, vl);
                ptrbb += 2;

                VLSEG2_FLOAT(&va4, &va5, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va2, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va3, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va3, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va2, vl);

                ptrbb += 2;

                VLSEG2_FLOAT(&va6, &va7, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va4, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va5, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va5, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va4, vl);
                ptrbb += 2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va6, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va7, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va7, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va6, vl);
                ptrbb += 2;
            }

            for (k = (bk & 3); k > 0; k--)
            {
                VLSEG2_FLOAT(&va0, &va1, ptrba, vl);
                ptrba += vl*2;
                
                vres0 =  OP_rr(vres0, *(ptrbb + 0), va0, vl);
                vres1 =  OP_ir(vres1, *(ptrbb + 0), va1, vl);
                vres0 =  OP_ii(vres0, *(ptrbb + 1), va1, vl);
                vres1 =  OP_ri(vres1, *(ptrbb + 1), va0, vl);
                ptrbb += 2;
            }
            
            VLSEG2_FLOAT(&va0, &va1, C0, vl);
            va0 =  VFMACCVF_FLOAT(va0, alphar, vres0, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphar, vres1, vl);
            va0 = VFNMSACVF_FLOAT(va0, alphai, vres1, vl);
            va1 =  VFMACCVF_FLOAT(va1, alphai, vres0, vl);
            VSSEG2_FLOAT(C0, va0, va1, vl);
            C0 += vl * 2;
        }

        bb += bk << 1;
        C  += ldc << 1;
   }
   return 0;
}

