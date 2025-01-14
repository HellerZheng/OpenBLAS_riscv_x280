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
#define VSETVL(n) vsetvl_e32m8(n)
#define VSETVL_MAX vsetvlmax_e32m8()
#define FLOAT_V_T vfloat32m8_t
#define VLEV_FLOAT vle32_v_f32m8
#define VLSEV_FLOAT vlse32_v_f32m8
#define VSEV_FLOAT vse32_v_f32m8
#define VSSEV_FLOAT vsse32_v_f32m8
#define VFMVVF_FLOAT vfmv_v_f_f32m8
#else
#define VSETVL(n) vsetvl_e64m8(n)
#define VSETVL_MAX vsetvlmax_e64m8()
#define FLOAT_V_T vfloat64m8_t
#define VLEV_FLOAT vle64_v_f64m8
#define VLSEV_FLOAT vlse64_v_f64m8
#define VSEV_FLOAT vse64_v_f64m8
#define VSSEV_FLOAT vsse64_v_f64m8
#define VFMVVF_FLOAT vfmv_v_f_f64m8
#endif

int CNAME(BLASLONG n, BLASLONG dummy0, BLASLONG dummy1, FLOAT dummy3, FLOAT *x, BLASLONG inc_x, FLOAT *y, BLASLONG inc_y, FLOAT *dummy, BLASLONG dummy2)
{
    BLASLONG stride_x, stride_y;
    FLOAT_V_T vx, vy;

    if (n <= 0) return(0);

    if (inc_x == 0 && inc_y == 0) {
        if (n & 1) {
            FLOAT temp = x[0];
            x[0] = y[0];
            y[0] = temp;
        }
        else {
            return 0;
        }
    }
    else if(inc_x == 0) {
        FLOAT temp = x[0];
        x[0] = y[(n - 1) * inc_y];
        FLOAT* ptr = y + (n - 1) * inc_y;   // start from the last one
        stride_y = (0 - inc_y) * sizeof(FLOAT); // reverse
        BLASLONG m = n - 1;
        for (size_t vl; m > 0; m -= vl, ptr -= vl*inc_y) {
            vl = VSETVL(m);
            vy = VLSEV_FLOAT(ptr - 1, stride_y, vl);
            VSSEV_FLOAT(ptr, stride_y, vy, vl);
        }
        y[0] = temp;
    }
    else if(inc_y == 0) {
        FLOAT temp = y[0];
        y[0] = x[(n - 1) * inc_x];
        FLOAT* ptr = x + (n - 1) * inc_x;   // start from the last one
        stride_x = (0 - inc_x) * sizeof(FLOAT); // reverse
        BLASLONG m = n - 1;
        for (size_t vl; m > 0; m -= vl, ptr -= vl*inc_x) {
            vl = VSETVL(m);
            vx = VLSEV_FLOAT(ptr - 1, stride_x, vl);
            VSSEV_FLOAT(ptr, stride_x, vx, vl);
        }
        x[0] = temp;
    }
    else if(inc_x == 1 && inc_y == 1) {
        for (size_t vl; n > 0; n -= vl, x += vl, y += vl) {
            vl = VSETVL(n);

            vx = VLEV_FLOAT(x, vl);
            vy = VLEV_FLOAT(y, vl);
            VSEV_FLOAT(y, vx, vl);
            VSEV_FLOAT(x, vy, vl);
        }
  
    } else if (inc_y == 1) {
        stride_x = inc_x * sizeof(FLOAT);

        for (size_t vl; n > 0; n -= vl, x += vl*inc_x, y += vl) {
            vl = VSETVL(n);

            vx = VLSEV_FLOAT(x, stride_x, vl);
            vy = VLEV_FLOAT(y, vl);
            VSEV_FLOAT(y, vx, vl);
            VSSEV_FLOAT(x, stride_x, vy, vl);
        }
 
    } else if(inc_x == 1) {
        stride_y = inc_y * sizeof(FLOAT);

        for (size_t vl; n > 0; n -= vl, x += vl, y += vl*inc_y) {
            vl = VSETVL(n);

            vx = VLEV_FLOAT(x, vl);
            vy = VLSEV_FLOAT(y, stride_y, vl);
            VSSEV_FLOAT(y, stride_y, vx, vl);
            VSEV_FLOAT(x, vy, vl);
        }
 
    } else {
        stride_x = inc_x * sizeof(FLOAT);
        stride_y = inc_y * sizeof(FLOAT);

        for (size_t vl; n > 0; n -= vl, x += vl*inc_x, y += vl*inc_y) {
            vl = VSETVL(n);

            vx = VLSEV_FLOAT(x, stride_x, vl);
            vy = VLSEV_FLOAT(y, stride_y, vl);
            VSSEV_FLOAT(y, stride_y, vx, vl);
            VSSEV_FLOAT(x, stride_x, vy, vl);
        }
    }

    return(0);
}
