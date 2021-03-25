/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef SAIL_PIXEL_H
#define SAIL_PIXEL_H

#include <stdint.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A structure representing a pixel with 3 8-bit components. Typically, it's RGB.
 */
struct sail_pixel3_uint8
{
    uint8_t component1;
    uint8_t component2;
    uint8_t component3;
};

typedef struct sail_pixel3_uint8 sail_rgb8_t;
typedef struct sail_pixel3_uint8 sail_bgr8_t;
typedef struct sail_pixel3_uint8 sail_ycbcr8_t;
typedef struct sail_pixel3_uint8 sail_cie_lab8_t;

/*
 * A structure representing a pixel with 4 8-bit components. Typically, it's RGBA.
 */
struct sail_pixel4_uint8
{
    uint8_t component1;
    uint8_t component2;
    uint8_t component3;
    uint8_t component4;
};

typedef struct sail_pixel4_uint8 sail_rgbx8_t;
typedef struct sail_pixel4_uint8 sail_bgrx8_t;
typedef struct sail_pixel4_uint8 sail_xrgb8_t;
typedef struct sail_pixel4_uint8 sail_xbgr8_t;
typedef struct sail_pixel4_uint8 sail_rgba8_t;
typedef struct sail_pixel4_uint8 sail_bgra8_t;
typedef struct sail_pixel4_uint8 sail_argb8_t;
typedef struct sail_pixel4_uint8 sail_abgr8_t;
typedef struct sail_pixel4_uint8 sail_cmyk8_t;
typedef struct sail_pixel4_uint8 sail_ycck8_t;

/*
 * Reads a sail_pixel3_uint8 pixel byte by byte from the I/O stream.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_read_pixel3_uint8(struct sail_io *io, struct sail_pixel3_uint8 *pixel);

/*
 * Reads a sail_pixel4_uint8 pixel byte by byte from the I/O stream.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_read_pixel4_uint8(struct sail_io *io, struct sail_pixel4_uint8 *pixel);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
