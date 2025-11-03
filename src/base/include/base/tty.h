/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef base_TTY_H
#define base_TTY_H



#include "base/base.h"
#include <functional>
#include "uv.h"

namespace base {

    ///

    class TTY {
    public:
        /// Create the idler with the given event loop.
        TTY();

        /// Create and start the idler with the given callback.
        TTY(std::function<void(char*, int) > cbfun);

        std::function<void( char*, int) > cbfun;

        /// Start the idler with the given callback function.
        virtual void start();

        virtual ~TTY();

        virtual void stop();
        // virtual void run();

    protected:

        uv_tty_t tty;
    };


}


#endif // SCY_Idler_H

