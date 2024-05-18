
#include "t31Video.h"
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "sample-common.h"

#include <json/json.hpp>
using json = nlohmann::json;
#include "json/configuration.h"
#include <xailient-fi/sdk_json_interface.h>



#include "base/logger.h"
using namespace base;
using namespace base::cnfg;



extern struct chn_conf chn[];

 T31Video::~T31Video() {

  join();
  T31Exit();

}



void T31Video::run() {
    

    int ret = 0;
    int i = 0;
    FILE *fp;
    IMPFrameInfo *frame;

    fp = fopen("/tmp/snap1.rgba", "wb");
    if(fp == NULL) {
        SError<<"file open error ";
        return -1;
    }

    while (!stopped()) {
        /* Snap RGBA */
        ret = IMP_FrameSource_SetFrameDepth(3, 1);
        if (ret < 0) {
            SError<<"IMP_FrameSource_SetFrameDepth failed";
            return -1;
        }

        ret = IMP_FrameSource_GetFrame(3, &frame);
        if (ret < 0) {
            SError<<"IMP_FrameSource_GetFrame failed";
            return -1;
        }

        STrace << "Frame size "  <<  frame->size << " width"  << frame->width << " height "  <<   frame->height <<  " format "  << frame->pixfmt;
        
        //XAProcess( frame->virAddr, frame->width , frame->height) ;
        if (80 == i) {
            fwrite((void *)frame->virAddr, frame->size, 1, fp);
            fclose(fp);
        }
        IMP_FrameSource_ReleaseFrame(3, frame);
        if (ret < 0) {
            SError<<"IMP_FrameSource_ReleaseFrame failed";
            return -1;
        }
        ret = IMP_FrameSource_SetFrameDepth(3, 0);
        if (ret < 0) {
            SError<<"IMP_FrameSource_SetFrameDepth failed";
            return -1;
        }

        ++i;
    }

    fclose(fp);
    /* end */
    return ;
}

int T31Video::T31Init()
{
    int i, ret;

    /* Step.1 System init */
    ret = sample_system_init();
    if (ret < 0) {
        SError<<"IMP_System_Init() failed";
        return -1;
    }

    chn[3].enable = 0;
    chn[2].enable = 1;
    ret = sample_framesource_init();
    if (ret < 0) {
        SError<<"FrameSource init failed";
        return -1;
    }

    ret = sample_framesource_ext_rgba_init();
    if (ret < 0) {
        SError<<"FrameSource init ext rgba failed";
        return -1;
    }

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_Encoder_CreateGroup(chn[i].index);
            if (ret < 0) {
                SError<<"IMP_Encoder_CreateGroup(%d) error: " <<  i ;
                return -1;
            }
        }
    }

    /* Step.3 Encoder init */
    ret = sample_encoder_init();
    if (ret < 0) {
        SError<<"Encoder init failed";
        return -1;
    }

    /* Step.4 Bind */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                SError<<"Bind FrameSource channel%d and Encoder failed: " << i ;
                return -1;
            }
        }
    }

    /* Step.5 Stream On */
    ret = sample_framesource_streamon();
    if (ret < 0) {
        SError<<"ImpStreamOn failed";
        return -1;
    }
    ret = sample_framesource_ext_rgba_streamon();
    if (ret < 0) {
        SError<<"ImpStreamOn ext rgba failed";
        return -1;
    }


    /* Step.6 Get stream */

#if 1
    // ret = sample_snap_fs_rgba();
    // if (ret < 0) {
    //     SError<<"Get FS RGBA failed";
    //     return -1;
    // }

     //start();
#endif

    // ret = sample_get_video_stream();  // For hh264 stream
    // if (ret < 0) {
    //     SError<<"Get H264 stream failed";
    //     return -1;
    // }


    return 0;
}


int T31Video::T31Exit()
{
     int ret;
    /* Exit sequence as follow */

    /* Step.a Stream Off */
    ret = sample_framesource_streamoff();
    if (ret < 0) {
        SError<<"FrameSource StreamOff failed";
        return -1;
    }

    ret = sample_framesource_ext_rgba_streamoff();
    if (ret < 0) {
        SError<<"FrameSource StreamOff  ext rgba failed";
        return -1;
    }

    /* Step.b UnBind */
    for (int i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                SError<<"UnBind FrameSource channel%d and Encoder failed: "  << i ;
                return -1;
            }
        }
    }

    /* Step.c Encoder exit */
    ret = sample_encoder_exit();
    if (ret < 0) {
        SError<<"Encoder exit failed";
        return -1;
    }

    /* Step.d FrameSource exit */
    ret = sample_framesource_exit();
    if (ret < 0) {
        SError<<"FrameSource exit failed";
        return -1;
    }
    ret = sample_framesource_ext_rgba_exit();
    if (ret < 0) {
        SError<<"FrameSource exit rgba exit failed";
        return -1;
    }

    /* Step.e System exit */
    ret = sample_system_exit();
    if (ret < 0) {
        SError<<"sample_system_exit() failed";
        return -1;
    }
}






int T31Video::XAInit()
{
    cnfg::Configuration config;

    config.load("./config.json");

    std::string xaconfig = config.root.dump();

    xa_fi_error_t returnValue;

    const char* path_to_vision_cell =
        "/mnt/libxailient-fi-vcell.so";                    // For shared lib
    returnValue = xa_sdk_initialize(path_to_vision_cell);  // For shared lib

    // returnValue = xa_sdk_initialize(); // For static lib

    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_initialize";

        return -1;
    }

    const char* configuration = xaconfig.c_str();

    STrace << "config json: " << configuration;

    // xa_sdk_update_identities
    // xa_sdk_update_identity_image
    returnValue = xa_sdk_configure(configuration);
    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_configure";

        return -1;
    }

    returnValue = xa_sdk_is_face_recognition_enabled();
    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_configure";

        return -1;
    }

   return 0;
}

int T31Video::XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  )
{

    xa_fi_image_t image;
    image.width = w;
    image.height = h;

    image.pixel_format =
        XA_FI_COLOR_RGB888;  // signifies the buffer data format

   // uint8_t* buffer_containing_raw_rgb_data = new uint8_t [image.width*3*image.height];

    //memset(buffer_containing_raw_rgb_data, 0, image.width*3*image.height);


    image.buff =  buffer_containing_raw_rgb_data;  // note this is in RGB order, otherwise
                                         // colors will be swapped

    xa_fi_error_t returnValue;

    xa_sdk_process_image_outputs* process_image_outputs;

    if (1) {
        returnValue = xa_sdk_process_image(&image, &process_image_outputs);

        if (returnValue == XA_ERR_NONE) {
            for (int index = 0;
                 index < process_image_outputs->number_of_json_blobs; ++index) {
                xa_sdk_json_blob_t blob = process_image_outputs->blobs[index];

                if (blob.blob_descriptor == XA_FACE_TRACK_EVENT) {
                    //<
                    // send blob -> json to Face Track Event endpoint >
                    STrace << "json to Face Track Event endpoint: " <<  blob.json;

                } else if (blob.blob_descriptor == XA_ACCURACY_MONITOR) {
                    //<
                    // send blob -> json to Accuracy Monitor endpoint >
                    STrace << "send blob -> json to Accuracy Monitor endpoint: " <<  blob.json;
                } else {
                    SError << "Not a possible state";
                }
            }
        } else {
            SError << "Error at process_image_outputs";
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

}