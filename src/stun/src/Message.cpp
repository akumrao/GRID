#include <zlib.h>
#include <stdio.h>
#include <Message.h>
#include <Utils.h>
#include <cstring>


	/*
		    STUN Attributes

		    After the STUN header are zero or more attributes.  Each attribute
		    MUST be TLV encoded, with a 16-bit type, 16-bit length, and value.
		    Each STUN attribute MUST end on a 32-bit boundary.  As mentioned
		    above, all fields in an attribute are transmitted most significant
		    bit first.

		        0                   1                   2                   3
		        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		       |         Type                  |            Length             |
		       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		       |                         Value (variable)                ....
		       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       */

namespace stun {

  /* --------------------------------------------------------------------- */

  Message::Message(uint16_t type)
    :type(type)
    ,length(0)
    ,cookie(0x2112a442)
  {
      memset(&credentials ,'\0', sizeof(stun_credentials_t)) ;
  }

    
  Message::Message(stun_class_t msg_class, stun_method_t msg_method)
    : type( (uint16_t) msg_class | (uint16_t)msg_method )
    ,length(0)
    ,cookie(0x2112a442)
  {
      memset(&credentials ,'\0', sizeof(stun_credentials_t)) ;
  }

  Message::~Message() {
    std::vector<Attribute*>::iterator it = attributes.begin();
    while (it != attributes.end()) {
      delete *it;
      it = attributes.erase(it);
    }
  }

  void Message::addAttribute(Attribute* attr) {
    attributes.push_back(attr);
  }

  void Message::copyTransactionID(Message* from) {

    if (!from) {
      printf("Error: tryign to copy a transaction ID, but the message is invalid in stun::Message.\n");
      return;
    }

//    transaction[0] = from->transaction[0];
//    transaction[1] = from->transaction[1];
//    transaction[2] = from->transaction[2];
  }

  void Message::setTransactionID() {
//    transaction[0] = a;
//    transaction[1] = b;
//    transaction[2] = c;
    random_bytes(transaction_id, STUN_TRANSACTION_ID_SIZE);
    printf("Message::Set msg  transactionids: ");
       for (int k = 0; k < STUN_TRANSACTION_ID_SIZE; ++k) {
         printf("%02X ", transaction_id[k]);
    }
    printf("\n");

  }

   void Message::setTransactionID( uint8_t *transactionid) {
        memcpy(transaction_id, transactionid, STUN_TRANSACTION_ID_SIZE);
   }

  
  bool Message::find(MessageIntegrity** result) {
    return (find<MessageIntegrity>(STUN_ATTR_MESSAGE_INTEGRITY, result) ||  find<MessageIntegrity>(STUN_ATTR_MESSAGE_INTEGRITY_SHA256, result)) ;
  }

  bool Message::find(XorMappedAddress** result) {
    return find<XorMappedAddress>(STUN_ATTR_XOR_MAPPED_ADDRESS, result);
  }

  bool Message::find(Fingerprint** result) {
    return find<Fingerprint>(STUN_ATTR_FINGERPRINT, result);
  }
  
  bool Message::find(Priority** result) {
    return find<Priority>(STUN_ATTR_PRIORITY, result);
  }

  bool Message::hasAttribute(AttributeType atype) {
    for (size_t i = 0; i < attributes.size(); ++i) {
      if (attributes[i]->type == atype) {
        return true;
      }
    }
    return false;
  }



} /* namespace stun */
