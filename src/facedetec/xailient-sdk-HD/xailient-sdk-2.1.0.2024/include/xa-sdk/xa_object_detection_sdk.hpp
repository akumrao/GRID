/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_OBJECT_DETECTION_SDK_HPP_
#define XAILIENT_OBJECT_DETECTION_SDK_HPP_

//#include <xa-sdk/xa_version_compatibility.hpp>
#include <xa-sdk/xa_error_defs.hpp>
#include <xa-sdk/xa_logging_defs.hpp>
#include <xa-sdk/xa_pipeline.hpp>
#include <xa-sdk/xa_bizcontroller.hpp>

namespace xailient {
namespace sdk {

struct SdkInitData {
    // the log callback type, can be null if not needed
    LogCallbackType logFunction;
};

/**
 * @brief Configuration of the sdk
 *
 */
struct Config {
    BizcontrollerConfig bizcontroller;
    PipelineConfig pipeline;
};

/**
 * @brief Initializes the object detection sdk
 *
 * @param data  The sdk initialization data if any
 * @return ErrorCode
 */
ErrorCode
xa_sdk_initialize(const SdkInitData& data);

/**
 * @brief Return the current configuration
 *
 * @return Config
 */
Config
xa_sdk_get_config();


/**
 * @brief Configure the sdk
 *
 * @param configure
 * @return ErrorCode
 */
ErrorCode
xa_sdk_configure(const Config& configure);


/**
 * @brief Run inference on an image and return the list of detected objects
 *
 * @param image
 * @param output
 * @return ErrorCode
 */
ErrorCode
xa_sdk_process_image(const Image& image, BizcontrollerOutput& output);


/**
 * @brief Uninit the sdk disabling all functionalities
 *
 * @return ErrorCode
 */
ErrorCode
xa_sdk_uninitialize();



}
}

#endif // XAILIENT_OBJECT_DETECTION_SDK_HPP_
