


#ifndef STUN_Agent_H
#define STUN_Agent_H

#include <Attribute.h>
#include <Types.h>

#define STUN_TRANSACTION_ID_SIZE 12
namespace stun {

  class Agent {
  public:
    Agent();
    ~Agent();
    bool getInterfaces();


  public:
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    //uint32_t transaction[3];
    uint8_t transaction_id[STUN_TRANSACTION_ID_SIZE];
    std::vector<Attribute*> attributes;
    std::vector<uint8_t> buffer;
  };

} /* namespace stun */

#endif
