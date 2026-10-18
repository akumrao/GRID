/*

  STUN
  ----

  Experimental code that parses stun. 

  References:
  ----------
  - libjingle: https://gist.github.com/roxlu/511544ea07610f70a900
  - own test code: https://gist.github.com/roxlu/57fe6c590f6267e0a209
  - C++, each attribute has its own class: https://github.com/husman/Development-Sample-Side-Projects/blob/713b052afd2344e95b7e1b33405713460d1b2d95/Computer_Science/My_Hobby_Projects/Live_Streaming/P2P_Live_Streaming/3rdparty/StunUsernameAttribute.h
  - C++, each clean example, a couple of generic attribute values: https://github.com/sourcey/libsourcey/blob/f126fdaa26fdcc3f44ebd2d70f2915ee0b6a1b23/src/stun/tests/stuntests.cpp
  - Stun attribute values: http://www.iana.org/assignments/stun-parameters/stun-parameters.xhtml
  - Some good info on the attributes and their meaning: http://www.3cx.com/blog/voip-howto/stun-details/
  - Test data for message-integrity checks: http://tools.ietf.org/html/rfc5769
 */

#ifndef STUN_READER_H
#define STUN_READER_H



#include <stdint.h>
#include <vector>
#include <string>
#include <Types.h>
#include <MessageStun.h>

namespace stun {

#define STUN_IS_RESPONSE(msg_class) (msg_class & 0x0100)

    class Reader {
    public:
        Reader();
        int process(uint8_t* data, uint32_t nbytes, MessageStun* msg); /* parses the incoming data and fills msg if the data contains a valid stun message, if so it returns 0, when other data is passed into this function it will return 1, on error it returns -1 */

    private:

        int stun_read_attr(const void *data, size_t size, MessageStun *msg, uint8_t *begin, uint8_t *attr_begin, uint32_t *security_bits);
  

    public:


        bool stun_check_integrity(void *buf, size_t size, const MessageStun *msg, const char *password);
  

  
    };

} /* namespace stun */

#endif
