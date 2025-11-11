/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "base/logger.h"
#include "base/application.h"
#include "base/tty.h"

#include <assert.h>
#include <iostream>


using std::cout;
using std::cerr;
using std::endl;


namespace base {

    void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        buf->base = (char*) malloc(suggested_size);
        buf->len = suggested_size;
    }

    void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
        if (nread > 0) {
            // Process the received character(s)
           // printf("Received: %s\n", buf->base);

            TTY *idler = (TTY*) stream->data;

            char c = buf->base[0];

            if (c == 3) {

                if (idler->cbfun)
                idler->cbfun(nullptr, 0 );
                   
                if (buf->base) {
                    free(buf->base);
                }
                uv_read_stop(stream);
                uv_close((uv_handle_t*) stream, NULL);
                
                return;
            }


            if (idler->cbfun)
                idler->cbfun(buf->base, nread );

            // If you want to stop reading after one character:
            // uv_read_stop(stream);
            // uv_close((uv_handle_t*) stream, NULL);
        } else if (nread < 0) {
            if (nread != UV_EOF) {
                fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
            }
            uv_close((uv_handle_t*) stream, NULL);
        }
        // Free the buffer if it was allocated by the allocator callback
        if (buf->base) {
            free(buf->base);
        }
    }



    TTY::TTY(std::function<void(char *, int) > cbfun) : cbfun(cbfun) {
        start();
    }

    TTY::TTY() {
    }

    void TTY::start() {

        uv_tty_init(Application::uvGetLoop(), &tty, STDIN_FILENO, 1); // 1 for readable
        uv_tty_set_mode(&tty, UV_TTY_MODE_RAW); // Set raw mode for unbuffered input

        tty.data = this;

        uv_read_start((uv_stream_t*) & tty, alloc_buffer, on_read);


        printf("Press a key (Ctrl+C to exit):\n");

    }


    int TTY::getWinSize(int *width, int *height) {

        return uv_tty_get_winsize(&tty, width, height);
    }

    TTY::~TTY() {
        stop();
    }

    //    void TTY::run() {
    //       
    //    }

    void TTY::stop() {
        uv_read_stop((uv_stream_t*) &tty);
        uv_close((uv_handle_t*) &tty, NULL);
        uv_tty_reset_mode(); // Reset TTY mode before exiting
    }


} // namespace base


