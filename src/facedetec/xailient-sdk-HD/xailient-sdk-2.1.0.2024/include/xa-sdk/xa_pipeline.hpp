/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_OBJECT_DETECTION_PIPELINE_JZDL_HPP_
#define XAILIENT_OBJECT_DETECTION_PIPELINE_JZDL_HPP_

#include <cstdint>
#include <cstdlib>

namespace xailient {
namespace sdk {

/**
 * @brief The image type, should be RGB format
 *
 */
struct Image {
  uint8_t* buff;
  size_t width;
  size_t height;
};


/**
 * @brief Pipeline configuration, if any
 *
 */
struct PipelineConfig {
    float scoreThreshold;
    float nmsThreshold;
    // we want to keep bboxes that have a area bigger than X% of the original image
    // if we take 1% seems to small but it is not actually, if we divide the image in 10 by 10, 1 cell is 1%
    // Range: [0.0 ... 1.0]
    float minAcceptableBBoxArea;
    // Min, max aspect ratios range (width / height) of bounding box that will be considered detections.
    // Detected bboxes outside of these values will discarded as they are likely false positives.
    float minAcceptableBBoxAspectRatio;
    float maxAcceptableBBoxAspectRatio;
};


}
}

#endif // XAILIENT_OBJECT_DETECTION_PIPELINE_JZDL_HPP_
