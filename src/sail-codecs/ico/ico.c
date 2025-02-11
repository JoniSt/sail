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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "common/bmp/bmp.h"

#include "helpers.h"

#define SAIL_ICO_TYPE_ICO 1
#define SAIL_ICO_TYPE_CUR 2

/*
 * Codec-specific state.
 */
struct ico_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    struct SailIcoHeader ico_header;
    struct SailIcoDirEntry *ico_dir_entries;
    unsigned current_frame;

    void *common_bmp_state;
};

static sail_status_t alloc_ico_state(struct ico_state **ico_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct ico_state), &ptr));
    *ico_state = ptr;

    (*ico_state)->read_options  = NULL;
    (*ico_state)->write_options = NULL;

    (*ico_state)->ico_dir_entries  = NULL;
    (*ico_state)->current_frame    = 0;
    (*ico_state)->common_bmp_state = NULL;

    return SAIL_OK;
}

static void destroy_ico_state(struct ico_state *ico_state) {

    if (ico_state == NULL) {
        return;
    }

    sail_destroy_read_options(ico_state->read_options);
    sail_destroy_write_options(ico_state->write_options);

    sail_free(ico_state->ico_dir_entries);

    sail_free(ico_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v6_ico(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(read_options);

    /* Allocate a new state. */
    struct ico_state *ico_state;
    SAIL_TRY(alloc_ico_state(&ico_state));
    *state = ico_state;

    SAIL_TRY(ico_private_read_header(io, &ico_state->ico_header));

    if (ico_state->ico_header.images_count == 0) {
        SAIL_LOG_ERROR("ICO: No images found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Check the image type. */
    switch (ico_state->ico_header.type) {
        case SAIL_ICO_TYPE_ICO:
        case SAIL_ICO_TYPE_CUR: break;
        default: {
            SAIL_LOG_ERROR("ICO: Invalid image type %u", ico_state->ico_header.type);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct SailIcoDirEntry) * ico_state->ico_header.images_count, &ptr));
    ico_state->ico_dir_entries = ptr;

    for (unsigned i = 0; i < ico_state->ico_header.images_count; i++) {
        SAIL_TRY(ico_private_read_dir_entry(io, &ico_state->ico_dir_entries[i]));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v6_ico(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct ico_state *ico_state = (struct ico_state *)state;

    /* Skip non-BMP images. */
    enum SailIcoImageType ico_image_type;

    do {
        if (ico_state->current_frame >= ico_state->ico_header.images_count) {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
        }

        SAIL_TRY(io->seek(io->stream, (long)ico_state->ico_dir_entries[ico_state->current_frame++].image_offset, SEEK_SET));

        /* Check the image is not PNG. */
        SAIL_TRY(ico_private_probe_image_type(io, &ico_image_type));
    } while (ico_image_type != SAIL_ICO_IMAGE_BMP);

    /* Continue to reading BMP. */
    SAIL_TRY(bmp_private_read_init(io, ico_state->read_options, &ico_state->common_bmp_state, SAIL_NO_BMP_FLAGS));
    SAIL_TRY(bmp_private_read_seek_next_frame(ico_state->common_bmp_state, io, image));

    /*
     * The contained image is twice the height declared in the directory.
     * The second half is a mask. We need just the image.
     */
    (*image)->height /= 2;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v6_ico(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct ico_state *ico_state = (struct ico_state *)state;

    SAIL_TRY(bmp_private_read_frame(ico_state->common_bmp_state, io, image));
    SAIL_TRY(bmp_private_read_finish(&ico_state->common_bmp_state, io));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v6_ico(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct ico_state *ico_state = (struct ico_state *)(*state);

    *state = NULL;

    if (ico_state->common_bmp_state != NULL) {
        SAIL_TRY_OR_CLEANUP(bmp_private_read_finish(&ico_state->common_bmp_state, io),
                            /* cleanup */ destroy_ico_state(ico_state));
    }

    destroy_ico_state(ico_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v6_ico(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v6_ico(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v6_ico(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v6_ico(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
