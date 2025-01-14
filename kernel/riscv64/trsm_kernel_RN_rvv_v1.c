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
#define VSETVL_MAX vsetvlmax_e32m2()
#define FLOAT_V_T vfloat32m2_t
#define VLEV_FLOAT vle32_v_f32m2
#define VSSEV_FLOAT vsse32_v_f32m2
#define VSEV_FLOAT vse32_v_f32m2
#define VLSEG2_FLOAT vlseg2e32_v_f32m2
#define VSSEG2_FLOAT vsseg2e32_v_f32m2
#define VLSSEG2_FLOAT vlsseg2e32_v_f32m2
#define VSSSEG2_FLOAT vssseg2e32_v_f32m2
#define VFMACCVF_FLOAT vfmacc_vf_f32m2
#define VFNMSACVF_FLOAT vfnmsac_vf_f32m2
#define VFMULVF_FLOAT vfmul_vf_f32m2
#else
#define VSETVL(n) vsetvl_e64m2(n)
#define VSETVL_MAX vsetvlmax_e64m2()
#define FLOAT_V_T vfloat64m2_t
#define VLEV_FLOAT vle64_v_f64m2
#define VSSEV_FLOAT vsse64_v_f64m2
#define VSEV_FLOAT vse64_v_f64m2
#define VLSEG2_FLOAT vlseg2e64_v_f64m2
#define VSSEG2_FLOAT vsseg2e64_v_f64m2
#define VLSSEG2_FLOAT vlsseg2e64_v_f64m2
#define VSSSEG2_FLOAT vssseg2e64_v_f64m2
#define VFMVVF_FLOAT vfmv_v_f_f64m2
#define VFMACCVF_FLOAT vfmacc_vf_f64m2
#define VFNMSACVF_FLOAT vfnmsac_vf_f64m2
#define VFMULVF_FLOAT vfmul_vf_f64m2
#endif

static FLOAT dm1 = -1.;

#ifdef CONJ
#define GEMM_KERNEL   GEMM_KERNEL_R
#else
#define GEMM_KERNEL   GEMM_KERNEL_N
#endif

#if GEMM_DEFAULT_UNROLL_N == 1
#define GEMM_UNROLL_N_SHIFT 0
#endif

#if GEMM_DEFAULT_UNROLL_N == 2
#define GEMM_UNROLL_N_SHIFT 1
#endif

#if GEMM_DEFAULT_UNROLL_N == 4
#define GEMM_UNROLL_N_SHIFT 2
#endif

#if GEMM_DEFAULT_UNROLL_N == 8
#define GEMM_UNROLL_N_SHIFT 3
#endif

#if GEMM_DEFAULT_UNROLL_N == 16
#define GEMM_UNROLL_N_SHIFT 4
#endif

// Optimizes the implementation in ../arm64/trsm_kernel_RN_sve.c

#ifndef COMPLEX

static inline void solve(BLASLONG m, BLASLONG n, FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc) {

    FLOAT bb;
    FLOAT *pci, *pcj;

    int i, j, k;
    FLOAT_V_T va, vc;

    size_t vl;
    for (i = 0; i < n; i++) {

        bb = *(b + i);
        pci = c + i * ldc;
        pcj = c;
        for (j = m; j > 0; j -= vl) {
            vl = VSETVL(j);
            va = VLEV_FLOAT(pci, vl);
            va = VFMULVF_FLOAT(va, bb, vl);
            VSEV_FLOAT(a, va, vl);
            VSEV_FLOAT(pci, va, vl);
            a   += vl;
            pci += vl;
            for (k = i + 1; k < n; k ++){
                vc = VLEV_FLOAT(pcj + k * ldc, vl);
                vc = VFNMSACVF_FLOAT(vc, *(b + k), va, vl);
                VSEV_FLOAT(pcj + k * ldc, vc, vl);
            }
            pcj += vl;
        }
        b += n;
    }
}

#else

static inline void solve(BLASLONG m, BLASLONG n, FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc) {

    FLOAT bb1, bb2;

    FLOAT *pci, *pcj;

    int i, j, k;

    FLOAT_V_T va1, va2, vs1, vs2, vc1, vc2;

    size_t vl;

    for (i = 0; i < n; i++) {

        bb1 = *(b + i * 2 + 0);
        bb2 = *(b + i * 2 + 1);

        pci = c + i * ldc * 2;
        pcj = c;

        for (j = m; j > 0; j -= vl) {
            vl = VSETVL(j);
            VLSEG2_FLOAT(&va1, &va2, pci, vl);
#ifndef CONJ
            vs1 =   VFMULVF_FLOAT(va1, bb1, vl);
            vs1 = VFNMSACVF_FLOAT(vs1, bb2, va2, vl);
            vs2 =   VFMULVF_FLOAT(va1, bb2, vl);
            vs2 =  VFMACCVF_FLOAT(vs2, bb1, va2, vl);
#else
            vs1 =   VFMULVF_FLOAT(va1, bb1, vl);
            vs1 =  VFMACCVF_FLOAT(vs1, bb2, va2, vl);
            vs2 =   VFMULVF_FLOAT(va2, bb1, vl);
            vs2 = VFNMSACVF_FLOAT(vs2, bb2, va1, vl);
#endif
            VSSEG2_FLOAT(a, vs1, vs2, vl);
            VSSEG2_FLOAT(pci, vs1, vs2, vl);
            a += vl * 2;
            pci += vl * 2;

            for (k = i + 1; k < n; k ++){
                VLSEG2_FLOAT(&vc1, &vc2, pcj + k * ldc * 2, vl);
#ifndef CONJ
                vc1 =  VFMACCVF_FLOAT(vc1, *(b + k * 2 + 1), vs2, vl);
                vc1 = VFNMSACVF_FLOAT(vc1, *(b + k * 2 + 0), vs1, vl);
                vc2 = VFNMSACVF_FLOAT(vc2, *(b + k * 2 + 1), vs1, vl);
                vc2 = VFNMSACVF_FLOAT(vc2, *(b + k * 2 + 0), vs2, vl);
#else
                vc1 = VFNMSACVF_FLOAT(vc1, *(b + k * 2 + 0), vs1, vl);
                vc1 = VFNMSACVF_FLOAT(vc1, *(b + k * 2 + 1), vs2, vl);
                vc2 =  VFMACCVF_FLOAT(vc2, *(b + k * 2 + 1), vs1, vl);
                vc2 = VFNMSACVF_FLOAT(vc2, *(b + k * 2 + 0), vs2, vl);
#endif
                VSSEG2_FLOAT(pcj + k * ldc * 2, vc1, vc2, vl);
            }
            pcj += vl * 2;
        }
        b += n * 2;
    }
}

#endif


int CNAME(BLASLONG m, BLASLONG n, BLASLONG k, FLOAT dummy1,
#ifdef COMPLEX
	   FLOAT dummy2,
#endif
	   FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc, BLASLONG offset){

  FLOAT *aa, *cc;
  BLASLONG  kk;
  BLASLONG i, j;

  size_t vl = VSETVL_MAX;

    //fprintf(stderr, "%s , %s, m = %4ld  n = %4ld  k = %4ld offset = %4ld\n", __FILE__, __FUNCTION__, m, n, k, offset); // Debug


  j = (n >> GEMM_UNROLL_N_SHIFT);
  kk = -offset;

  while (j > 0) {

    aa = a;
    cc = c;

    i = vl;

    if (i <= m) {
      do {
	if (kk > 0) {
	  GEMM_KERNEL(vl, GEMM_UNROLL_N, kk, dm1,
#ifdef COMPLEX
		      ZERO,
#endif
		      aa, b, cc, ldc);
	}

	solve(vl, GEMM_UNROLL_N,
	      aa + kk * vl * COMPSIZE,
	      b  + kk * GEMM_UNROLL_N * COMPSIZE,
	      cc, ldc);

	aa += vl * k * COMPSIZE;
	cc += vl     * COMPSIZE;
	i += vl;
      } while (i <= m);
    }


    i = m % vl;
    if (i) {
      if (kk > 0) {
        GEMM_KERNEL(i, GEMM_UNROLL_N, kk, dm1,
#ifdef COMPLEX
            ZERO,
#endif
            aa, b, cc, ldc);
      }
      solve(i, GEMM_UNROLL_N,
          aa + kk * i             * COMPSIZE,
          b  + kk * GEMM_UNROLL_N * COMPSIZE,
          cc, ldc);

      aa += i * k * COMPSIZE;
      cc += i     * COMPSIZE;

    }

    kk += GEMM_UNROLL_N;
    b += GEMM_UNROLL_N * k   * COMPSIZE;
    c += GEMM_UNROLL_N * ldc * COMPSIZE;
    j --;
  }

  if (n & (GEMM_UNROLL_N - 1)) {

    j = (GEMM_UNROLL_N >> 1);
    while (j > 0) {
      if (n & j) {

	aa = a;
	cc = c;

  i = vl;

	while (i <= m) {
	  if (kk > 0) {
	    GEMM_KERNEL(vl, j, kk, dm1,
#ifdef COMPLEX
			ZERO,
#endif
			aa,
			b,
			cc,
			ldc);
	  }

	  solve(vl, j,
		aa + kk * vl * COMPSIZE,
		b  + kk * j             * COMPSIZE, cc, ldc);

	  aa += vl * k * COMPSIZE;
	  cc += vl     * COMPSIZE;
	  i += vl;
	}

  i = m % vl;
  if (i) {
	      if (kk > 0) {
		GEMM_KERNEL(i, j, kk, dm1,
#ifdef COMPLEX
			    ZERO,
#endif
			    aa,
			    b,
			    cc,
			    ldc);
	      }

	      solve(i, j,
		    aa + kk * i * COMPSIZE,
		    b  + kk * j * COMPSIZE, cc, ldc);

	      aa += i * k * COMPSIZE;
	      cc += i     * COMPSIZE;

  }

	b += j * k   * COMPSIZE;
	c += j * ldc * COMPSIZE;
	kk += j;
      }
      j >>= 1;
    }
  }

  return 0;
}
