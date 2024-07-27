/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_OBJECT_DETECTION_BIZCONTROLLER_DEFAULT_HPP_
#define XAILIENT_OBJECT_DETECTION_BIZCONTROLLER_DEFAULT_HPP_

#include <vector>
#include <xa-sdk/xa_bounding_box.hpp>

namespace xailient {
namespace sdk {

/**
 * @brief Bizcontroller configuration parameters
 *
 */
struct BizcontrollerConfig {
    // Duration in seconds before considering the object is lost and remove from tracked
    // object list
    // Units: seconds
    float objtrackLostDurationSecs;
    // This parameter is for testing mainly, currently it defines the maxium distance in
    // pixels that an bounding should be match with objects.
    float objtrackMaxCost;
    // The number frames we have to see a bounding box to consider it as a valid face.
    // The purpose of this is to help minimize false positive face detections. For example,
    // an FP face is detected for one frame then disappears. We want to ignore such "detections".
    // Units: frames
    // Range: 1..N.
    int objtrackFramesToBeValid;
};


/**
 * @brief The different detected object classes / types
 *
 */
enum class DetectedObjectClass {
    DLI_FACE = 0,
    DLI_PERSON,

    // unknown
    DLI_UNKNOWN = 1025
};

/**
 * @brief Converts to string
 *
 * @param objClass
 * @return constexpr const char*
 */
static constexpr const char* toString(DetectedObjectClass objClass)
{
    switch (objClass) {
        case DetectedObjectClass::DLI_FACE: return "FACE";
        case DetectedObjectClass::DLI_PERSON: return "PERSON";
        case DetectedObjectClass::DLI_UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

/**
 * @brief A detected object
 *
 */
struct DetectedObject {
    // the detected bounding box
    BoundingBox bbox;
    // the confidence
    float confidence;
    // the object label / class
    DetectedObjectClass label;
    // the unique object tracking id associated to this class
    unsigned int id;
};

/**
 * @brief The result of detected objects
 *
 */
struct BizcontrollerOutput {
    std::vector<DetectedObject> detectedObjects;
};

}
}

#endif // XAILIENT_OBJECT_DETECTION_BIZCONTROLLER_DEFAULT_HPP_
