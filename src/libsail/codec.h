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

#ifndef SAIL_CODEC_H
#define SAIL_CODEC_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "layout/v6_pointers.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail/layout/v6_pointers.h>
#endif

struct sail_codec_info;
struct sail_codec_layout_v6;

struct sail_read_features;
struct sail_read_options;
struct sail_write_features;
struct sail_write_options;
struct sail_image;
struct sail_io;

/*
 * SAIL codec.
 */
struct sail_codec {

    /* Layout version. */
    int layout;

    /* System-specific library handle. */
    void *handle;

    /* Codec interface. */
    struct sail_codec_layout_v6 *v6;
};

typedef struct sail_codec sail_codec_t;

/*
 * Loads the specified codec by its info and saves its handle and exported interfaces into
 * the specified codec instance.
 * The assigned codec MUST be destroyed later with sail_destroy_codec().
 *
 * Returns SAIL_OK on success.
 */
SAIL_HIDDEN sail_status_t alloc_and_load_codec(const struct sail_codec_info *codec_info, struct sail_codec **codec);

/*
 * Destroys the specified codec and all its internal memory buffers.
 * Does nothing if the codec is NULL.
 */
SAIL_HIDDEN void destroy_codec(struct sail_codec *codec);

#endif
