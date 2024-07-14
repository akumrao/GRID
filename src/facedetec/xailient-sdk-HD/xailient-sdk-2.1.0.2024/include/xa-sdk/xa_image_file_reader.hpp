/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_SDK_XA_IMAGE_FILE_READER_DEFS_HPP_
#define XAILIENT_SDK_XA_IMAGE_FILE_READER_DEFS_HPP_

#include <cstdint>
#include <memory>

namespace xailient {
namespace sdk {

/**
 * @brief Struct representing an rgb image
 *
 */
struct RGBImage {
  std::shared_ptr<uint8_t> buffer;
  size_t width;
  size_t height;
};


/**
 * @brief Reads an rgb image from a file path. This module should be optional since we do not want to add this into the
 *        final release.
 *
 * @param path
 * @param result
 * @return true
 * @return false
 */
bool
read_rgb_from_file(const char* path, RGBImage& result);


}
}

#endif // XAILIENT_SDK_XA_LOGGING_DEFS_HPP_
