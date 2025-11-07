/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#include "base/application.h"

#include <assert.h>
#include <iostream>


//using std::cout;
//using std::cerr;
//using std::endl;


namespace base {

//   #include <uv.h>
//#include <functional>
//#include <iostream>
//#include <memory>

class AsyncWorker {
public:
    // Define the types for the work and after callbacks
    using WorkCallback = std::function<void()>;
    using AfterWorkCallback = std::function<void(int status)>;

    AsyncWorker() ;

    // Method to queue work with custom callbacks
    int queueWork(WorkCallback work_cb, AfterWorkCallback after_work_cb) ;

private:
 

    // Static C-style callback for the work in the thread pool
    static void workCallbackStatic(uv_work_t* req) {
        auto callbacks = static_cast<std::pair<WorkCallback, AfterWorkCallback>*>(req->data);
        if (callbacks && callbacks->first) {
            // Execute the user-defined work callback
            callbacks->first();
        }
    }

    // Static C-style callback for the main thread after work is done
    static void afterWorkCallbackStatic(uv_work_t* req, int status) {
        auto callbacks = static_cast<std::pair<WorkCallback, AfterWorkCallback>*>(req->data);
        if (callbacks && callbacks->second) {
            // Execute the user-defined after-work callback
            callbacks->second(status);
        }

        // Clean up the memory allocated for the callbacks and the request
        delete callbacks;
        delete req;
    }
};


} // namespace base


