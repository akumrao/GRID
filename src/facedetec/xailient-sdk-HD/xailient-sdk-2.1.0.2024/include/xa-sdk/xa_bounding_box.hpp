/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_SDK_XA_BOUNDING_BOX_HPP_
#define XAILIENT_SDK_XA_BOUNDING_BOX_HPP_


namespace xailient {
namespace sdk {

/**
 * @brief Bounding box definition
 *
 */
struct BoundingBox {
    BoundingBox() = default;
    inline BoundingBox(int aXmin, int aYmin, int aXmax, int aYmax) :
        xmin(aXmin), ymin(aYmin), xmax(aXmax), ymax(aYmax) {}

    int xmin;
    int ymin;
    int xmax;
    int ymax;
};


}
}

#endif // XAILIENT_SDK_XA_BOUNDING_BOX_HPP_
