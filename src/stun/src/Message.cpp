#include <zlib.h>
#include <stdio.h>
#include <Message.h>
#include <Utils.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  Message::Message(uint16_t type)
    :type(type)
    ,length(0)
    ,cookie(0x2112a442)
  {
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

  bool Message::find(MessageIntegrity** result) {
    return find<MessageIntegrity>(STUN_ATTR_MESSAGE_INTEGRITY, result);
  }

  bool Message::find(XorMappedAddress** result) {
    return find<XorMappedAddress>(STUN_ATTR_XOR_MAPPED_ADDRESS, result);
  }

  bool Message::find(Fingerprint** result) {
    return find<Fingerprint>(STUN_ATTR_FINGERPRINT, result);
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
