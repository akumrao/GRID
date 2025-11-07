/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#include "base/async.h"



using std::cout;
using std::cerr;
using std::endl;


namespace base {

    
        
    Async::Async() 
    {
     
    }
     
     
    int Async::queueWork(WorkCallback work_cb, AfterWorkCallback after_work_cb) {
        // Create a new request object
        // Use std::unique_ptr for memory management in the callbacks
        auto req = new uv_work_t();
        
        // Use a std::pair or struct to hold the C++ callbacks and manage memory of 'req'
        // std::pair is used here for simplicity.
        auto callbacks = new std::pair<WorkCallback, AfterWorkCallback>{std::move(work_cb), std::move(after_work_cb)};

        // Assign the callbacks data to the uv_work_t's data pointer
        req->data = callbacks;

        // Queue the work using static C-style function wrappers
        return uv_queue_work(Application::uvGetLoop(), req, Async::workCallbackStatic, Async::afterWorkCallbackStatic);
    } 
     
     

} // namespace base


