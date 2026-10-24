/*

  stun::Writer
  -------------
  
  This will fill the `buffer` member with the binary representation
  of the values in the MessageStun. It will also compute the message integrity
  and fingerprint CRC32 value when they're found in the attributes of the 
  MessageStun that is passed into Writer::writeMessage().

 */
#ifndef STUN_WRITER_H
#define STUN_WRITER_H

#include <stdint.h>
#include <vector>
#include <string>
#include <MessageStun.h>
#include <Types.h>

namespace stun {

    class Writer {
    public:
        int writeMessage(MessageStun* msg);
        int writeMessage(MessageStun* msg, const char * messageIntegrityPassword); /* When you call this, we assume that the message contains a MessageIntegrity attribute. We calculate the hmac-sha and rewrite our internal buffer. This also checks for a Fingerprint attribute; and computers + writes this crc32-value. */

    private:
//        void writeErrorCode(ErrorIce* p);
//        void writeUseCandidate(Attribute* p);
//        void writeAttribute(Attribute* attr, MessageStun* msg);
//        void writeUsername(Username* u);
//        void writeSoftware(Software* s);
//        void writePriority(Priority* p);
//        void writeIceControlled(IceControlled* ic);
//        void writeIceControlling(IceControlling* ic);
//        void writeMessageIntegrity(MessageIntegrity* integ);
//        void writeFingerprint(Fingerprint* fp);
//        void writeXorMappedAddress(XorMappedAddress* xma, MessageStun* msg);
        
        // TURN and Credential attribute writers
//#if TURN
//        void writeRealm(const char* realm);
//        void writeNonce(const char* nonce);
//        void writePasswordAlgorithm(uint16_t algorithm);
//        void writePasswordAlgorithms(const uint8_t* value, size_t size);
//        void writeUserhash(const uint8_t* userhash);
//        void writeChannelNumber(uint16_t channel);
//        void writeLifetime(uint32_t lifetime);
//        void writeXorRelayedAddress(addr_record_t* relayed, MessageStun* msg);
//        void writeTurnData(const char* data, size_t data_size);
//        void writeRequestedTransport(uint8_t protocol);
//        void writeDontFragment();
//        void writeReservationToken(uint64_t token);
//        void writeEvenPort(bool even_port, bool next_port);
//#endif
//        void writeU8(uint8_t v);
//        void writeU16(uint16_t v);
//        void writeU32(uint32_t v);
//        void writeU64(uint64_t v);
//        void writeBytes(uint8_t* buf, uint32_t nbytes);
//        void writeString(StringValue v);
//        void rewriteU16(size_t index, uint16_t v);
//        void rewriteU32(size_t index, uint32_t v);

    public:
        std::vector<uint8_t> buffer;
        
        int stun_write(void *buf, size_t size, const MessageStun *msg, const char *password) ;
        int stun_write_header(void *buf, size_t size, stun_class_t class1,  stun_method_t method, const uint8_t *transaction_id);
    };

} /* namespace stun */
#endif
