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
#define VSETVL(n) vsetvl_e32m1(n)
#define FLOAT_V_T vfloat32m1_t
#define VLEV_FLOAT vle32_v_f32m1
#define VLSEV_FLOAT vlse32_v_f32m1
#define VSEV_FLOAT vse32_v_f32m1
#define VLSSEG2_FLOAT vlsseg2e32_v_f32m1
#define VSSEG2_FLOAT vsseg2e32_v_f32m1
#define VLSSEG4_FLOAT vlsseg4e32_v_f32m1
#define VSSEG4_FLOAT vsseg4e32_v_f32m1
#define VLSSEG8_FLOAT vlsseg8e32_v_f32m1
#define VSSEG8_FLOAT vsseg8e32_v_f32m1
#else
#define VSETVL(n) vsetvl_e64m1(n)
#define FLOAT_V_T vfloat64m1_t
#define VLEV_FLOAT vle64_v_f64m1
#define VLSEV_FLOAT vlse64_v_f64m1
#define VSEV_FLOAT vse64_v_f64m1
#define VLSSEG2_FLOAT vlsseg2e64_v_f64m1
#define VSSEG2_FLOAT vsseg2e64_v_f64m1
#define VLSSEG4_FLOAT vlsseg4e64_v_f64m1
#define VSSEG4_FLOAT vsseg4e64_v_f64m1
#define VLSSEG8_FLOAT vlsseg8e64_v_f64m1
#define VSSEG8_FLOAT vsseg8e64_v_f64m1
#endif

int CNAME(BLASLONG m, BLASLONG n, IFLOAT *a, BLASLONG lda, IFLOAT *b)
{
    BLASLONG i, j;

    IFLOAT *aoffset;
    IFLOAT *aoffset1;

    IFLOAT *boffset, *boffset1, *boffset2, *boffset3, *boffset4;

    FLOAT_V_T v0, v1, v2, v3, v4, v5, v6, v7;

    // fprintf(stderr, "gemm_tcopy_8 m=%ld n=%ld lda=%ld\n", m, n, lda);

    aoffset   = a;
    boffset   = b;
    boffset2  = b + m  * (n & ~7);
    boffset3  = b + m  * (n & ~3);
    boffset4  = b + m  * (n & ~1);

    for(j = (m >> 3); j > 0; j--) {

        aoffset1  = aoffset;
        aoffset += 8 * lda;

        boffset1  = boffset;
        boffset  += 64;

        for(i = (n >> 3); i > 0; i--) {
            size_t vl = 8;

            VLSSEG8_FLOAT(&v0, &v1, &v2, &v3, &v4, &v5, &v6, &v7, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG8_FLOAT(boffset1, v0, v1, v2, v3, v4, v5, v6, v7, vl);

            aoffset1 += 8;
            boffset1 += m * 8;
        }

        if (n & 4) {
            size_t vl = 8;

            VLSSEG4_FLOAT(&v0, &v1, &v2, &v3, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG4_FLOAT(boffset2, v0, v1, v2, v3, vl);

            aoffset1 += 4;
            boffset2 += 32;
        }

        if (n & 2) {
            size_t vl = 8;

            VLSSEG2_FLOAT(&v0, &v1, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG2_FLOAT(boffset3, v0, v1, vl);

            aoffset1 += 2;
            boffset3 += 16;
        }

        if (n & 1) {
            size_t vl = 8;

            v0 = VLSEV_FLOAT(aoffset1, lda * sizeof(FLOAT), vl);
            VSEV_FLOAT(boffset4, v0, vl);

            aoffset1 += 1;
            boffset4 += 8;
        }

    }

    if (m & 4) {

        aoffset1  = aoffset;
        aoffset += 4 * lda;

        boffset1  = boffset;
        boffset  += 32;

        for(i = (n >> 3); i > 0; i--) {
            size_t vl = 4;

            VLSSEG8_FLOAT(&v0, &v1, &v2, &v3, &v4, &v5, &v6, &v7, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG8_FLOAT(boffset1, v0, v1, v2, v3, v4, v5, v6, v7, vl);

            aoffset1 += 8;
            boffset1 += m * 8;
        }

        if (n & 4) {
            size_t vl = 4;

            VLSSEG4_FLOAT(&v0, &v1, &v2, &v3, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG4_FLOAT(boffset2, v0, v1, v2, v3, vl);

            aoffset1 += 4;
            boffset2 += 16;
        }

        if (n & 2) {
            size_t vl = 4;

            VLSSEG2_FLOAT(&v0, &v1, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG2_FLOAT(boffset3, v0, v1, vl);

            aoffset1 += 2;
            boffset3 += 8;
	}

        if (n & 1) {
            size_t vl = 4;

            v0 = VLSEV_FLOAT(aoffset1, lda * sizeof(FLOAT), vl);
            VSEV_FLOAT(boffset4, v0, vl);

            aoffset1 += 1;
            boffset4 += 4;
        }
    }

    if (m & 2) {
        aoffset1  = aoffset;
        aoffset += 2 * lda;

        boffset1  = boffset;
        boffset  += 16;

        for(i = (n >> 3); i > 0; i--) {
            size_t vl = 2;

            VLSSEG8_FLOAT(&v0, &v1, &v2, &v3, &v4, &v5, &v6, &v7, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG8_FLOAT(boffset1, v0, v1, v2, v3, v4, v5, v6, v7, vl);

            aoffset1 += 8;
            boffset1 += m * 8;
        }

        if (n & 4) {
            size_t vl = 2;

            VLSSEG4_FLOAT(&v0, &v1, &v2, &v3, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG4_FLOAT(boffset2, v0, v1, v2, v3, vl);

            aoffset1 += 4;
            boffset2 += 8;
        }

        if (n & 2) {
            size_t vl = 2;

            VLSSEG2_FLOAT(&v0, &v1, aoffset1, lda * sizeof(FLOAT), vl);
            VSSEG2_FLOAT(boffset3, v0, v1, vl);

            aoffset1 += 2;
            boffset3 += 4;
	}
  
        if (n & 1) {
           size_t vl = 2;

            v0 = VLSEV_FLOAT(aoffset1, lda * sizeof(FLOAT), vl);
            VSEV_FLOAT(boffset4, v0, vl);

            aoffset1 += 1;
            boffset4 += 2;
        }
    }

    if (m & 1) {
        aoffset1  = aoffset;
        boffset1  = boffset;

        for(i = (n >> 3); i > 0; i--) {
            size_t vl = 8;

            v0 = VLEV_FLOAT(aoffset1, vl);
            VSEV_FLOAT(boffset1, v0, vl);

            aoffset1 += 8;
            boffset1 += 8 * m;
        }

        if (n & 4) {
            size_t vl = 4;

            v0 = VLEV_FLOAT(aoffset1, vl);
            VSEV_FLOAT(boffset2, v0, vl);

            aoffset1 += 4;
            //boffset2 += 4;
        }

        if (n & 2) {
            size_t vl = 2;

            v0 = VLEV_FLOAT(aoffset1, vl);
            VSEV_FLOAT(boffset3, v0, vl);

            aoffset1 += 2;
           // boffset3 += 2;
        }

        if (n & 1) {
           *(boffset4) = *(aoffset1);
           // aoffset1 ++;
           // boffset4 ++;
        }
    }

    return 0;
}
