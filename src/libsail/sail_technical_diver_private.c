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

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static sail_status_t check_io_arguments(struct sail_io *io,
                                        const struct sail_codec_info *codec_info,
                                        void **state) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(codec_info);
    SAIL_CHECK_PTR(state);

    return SAIL_OK;
}

static sail_status_t allowed_write_compression(const struct sail_write_features *write_features,
                                               enum SailCompression compression) {

    SAIL_CHECK_PTR(write_features);

    for (unsigned i = 0; i < write_features->compressions_length; i++) {
        if (write_features->compressions[i] == compression) {
            return SAIL_OK;
        }
    }

    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
}

/*
 * Public functions.
 */

sail_status_t start_reading_io_with_options(struct sail_io *io, bool own_io,
                                           const struct sail_codec_info *codec_info,
                                           const struct sail_read_options *read_options, void **state) {

    SAIL_TRY_OR_CLEANUP(check_io_arguments(io, codec_info, state),
                        /* cleanup */ if (own_io) sail_destroy_io(io));

    *state = NULL;

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct hidden_state), &ptr),
                        /* cleanup */ if (own_io) sail_destroy_io(io));
    struct hidden_state *state_of_mind = ptr;

    state_of_mind->io            = io;
    state_of_mind->own_io        = own_io;
    state_of_mind->write_options = NULL;
    state_of_mind->state         = NULL;
    state_of_mind->codec_info   = codec_info;
    state_of_mind->codec        = NULL;

    SAIL_TRY_OR_CLEANUP(load_codec_by_codec_info(state_of_mind->codec_info, &state_of_mind->codec),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (read_options == NULL) {
        struct sail_read_options *read_options_local = NULL;

        SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(state_of_mind->codec_info->read_features, &read_options_local),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
        SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v6->read_init(state_of_mind->io, read_options_local, &state_of_mind->state),
                            /* cleanup */ sail_destroy_read_options(read_options_local),
                                          state_of_mind->codec->v6->read_finish(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
        sail_destroy_read_options(read_options_local);
    } else {
        SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v6->read_init(state_of_mind->io, read_options, &state_of_mind->state),
                            /* cleanup */ state_of_mind->codec->v6->read_finish(&state_of_mind->state, state_of_mind->io),
                                          destroy_hidden_state(state_of_mind));
    }

    *state = state_of_mind;

    return SAIL_OK;
}

sail_status_t start_writing_io_with_options(struct sail_io *io, bool own_io,
                                           const struct sail_codec_info *codec_info,
                                           const struct sail_write_options *write_options, void **state) {

    SAIL_TRY_OR_CLEANUP(check_io_arguments(io, codec_info, state),
                        /* cleanup */ if (own_io) sail_destroy_io(io));

    *state = NULL;

    /*
     * When write options is not NULL, we need to check if we can actually output the requested compression.
     * When write options is NULL, we use the default compression which is always acceptable.
     */
    if (write_options != NULL) {
        SAIL_TRY_OR_CLEANUP(allowed_write_compression(codec_info->write_features, write_options->compression),
                            /* cleanup */ if (own_io) sail_destroy_io(io));
    }

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct hidden_state), &ptr),
                        /* cleanup */ if (own_io) sail_destroy_io(io));
    struct hidden_state *state_of_mind = ptr;

    state_of_mind->io            = io;
    state_of_mind->own_io        = own_io;
    state_of_mind->write_options = NULL;
    state_of_mind->state         = NULL;
    state_of_mind->codec_info    = codec_info;
    state_of_mind->codec         = NULL;

    SAIL_TRY_OR_CLEANUP(load_codec_by_codec_info(state_of_mind->codec_info, &state_of_mind->codec),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (write_options == NULL) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_write_options_from_features(state_of_mind->codec_info->write_features, &state_of_mind->write_options),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
    } else {
        SAIL_TRY_OR_CLEANUP(sail_copy_write_options(write_options, &state_of_mind->write_options),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v6->write_init(state_of_mind->io, state_of_mind->write_options, &state_of_mind->state),
                        /* cleanup */ state_of_mind->codec->v6->write_finish(&state_of_mind->state, state_of_mind->io),
                                      destroy_hidden_state(state_of_mind));

    *state = state_of_mind;

    return SAIL_OK;
}
