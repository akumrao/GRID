//
// Created by Gemini on 2017/6/23.
//

#ifndef BIO_UDP_HANDSHAKE
#define BIO_UDP_HANDSHAKE

    static void mark_bitmap(uint8_t *bitmap, uint32_t offset, uint32_t len) {
        for (uint32_t i = offset; i < offset + len; i++) {
            bitmap[i / 8] |= (1 << (i % 8));
        }
    }

    static int is_bitmap_complete(const uint8_t *bitmap, uint32_t total_len) {
        for (uint32_t i = 0; i < total_len; i++) {
            if (!(bitmap[i / 8] & (1 << (i % 8)))) return 0;
        }
        return 1;
    }



#define DTLS_RECORD_HEADER_LEN      13
#define DTLS_HANDSHAKE_HEADER_LEN   12
#define MAX_DTLS_RECV_BUFFER        16384

    /* --- Data Structures --- */

    typedef struct {
        uint16_t message_seq{0};
        uint32_t total_length{0};
        ;
        uint32_t assembled_length{0};
        ;
        uint8_t *reassembly_buf{nullptr};
        uint8_t *bitmap{nullptr};
    } dtls_msg_reassembler_t;

    dtls_msg_reassembler_t reassembler;
    uint8_t bio_in_buf[MAX_DTLS_RECV_BUFFER];

    static int process_incoming_packet_v3(dtls_msg_reassembler_t *reassembler,
            const uint8_t *packet, size_t packet_len,
            uint8_t *out_buf, size_t *out_len) {
        if (packet_len < DTLS_RECORD_HEADER_LEN) return -1;

        uint8_t content_type = packet[0];
        uint16_t epoch = (packet[3] << 8) | packet[4];

        // Intercept Handshake (22) in Epoch 0 (Plaintext Handshake Records)
        if (content_type == 22 && epoch == 0) {
            size_t hs_offset = DTLS_RECORD_HEADER_LEN;
            if (hs_offset + DTLS_HANDSHAKE_HEADER_LEN > packet_len) return -1;

            uint8_t msg_type = packet[hs_offset];
            uint32_t length = (packet[hs_offset + 1] << 16) | (packet[hs_offset + 2] << 8) | packet[hs_offset + 3];
            uint16_t msg_seq = (packet[hs_offset + 4] << 8) | packet[hs_offset + 5];
            uint32_t frag_offset = (packet[hs_offset + 6] << 16) | (packet[hs_offset + 7] << 8) | packet[hs_offset + 8];
            uint32_t frag_length = (packet[hs_offset + 9] << 16) | (packet[hs_offset + 10] << 8) | packet[hs_offset + 11];

            // Isolate Handshake Messages that are actually fragmented (typically ClientHello [1])
            if (length > frag_length) {
                if (length > MAX_DTLS_RECV_BUFFER - (DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN)) return -1;

                // Allocate buffering structures on the very first fragment discovery
                if (reassembler->reassembly_buf == NULL) {
                    reassembler->total_length = length;
                    reassembler->message_seq = msg_seq;
                    reassembler->reassembly_buf = (uint8_t *) calloc(1, length);
                    reassembler->bitmap = (uint8_t *) calloc(1, (length + 7) / 8);
                    if (!reassembler->reassembly_buf || !reassembler->bitmap) return -1;
                }

                // Boundary validation guards against malformed fragments
                if (frag_offset + frag_length > length ||
                        hs_offset + DTLS_HANDSHAKE_HEADER_LEN + frag_length > packet_len) {
                    return -1;
                }

                // Safely write incoming payload chunk straight to its relative target block position
                const uint8_t *frag_payload = packet + hs_offset + DTLS_HANDSHAKE_HEADER_LEN;
                memcpy(reassembler->reassembly_buf + frag_offset, frag_payload, frag_length);
                mark_bitmap(reassembler->bitmap, frag_offset, frag_length);

                // Re-evaluate if all holes are filled
                if (is_bitmap_complete(reassembler->bitmap, reassembler->total_length)) {
                    size_t total_hs_msg_len = DTLS_HANDSHAKE_HEADER_LEN + reassembler->total_length;
                    size_t total_record_len = DTLS_RECORD_HEADER_LEN + total_hs_msg_len;

                    if (*out_len < total_record_len) return -1;

                    // Synthesize a fresh, completely unfragmented DTLS Record Header Base
                    memcpy(out_buf, packet, DTLS_RECORD_HEADER_LEN);
                    out_buf[11] = (total_hs_msg_len >> 8) & 0xFF;
                    out_buf[12] = total_hs_msg_len & 0xFF;

                    // Synthesize Complete Handshake Header Frame
                    uint8_t *out_hs = out_buf + DTLS_RECORD_HEADER_LEN;
                    out_hs[0] = msg_type;
                    out_hs[1] = (length >> 16) & 0xFF;
                    out_hs[2] = (length >> 8) & 0xFF;
                    out_hs[3] = length & 0xFF;
                    out_hs[4] = (msg_seq >> 8) & 0xFF;
                    out_hs[5] = msg_seq & 0xFF;
                    out_hs[6] = 0;
                    out_hs[7] = 0;
                    out_hs[8] = 0; // fragment_offset = 0
                    out_hs[9] = out_hs[1];
                    out_hs[10] = out_hs[2];
                    out_hs[11] = out_hs[3]; // fragment_length = total_length

                    // Flush collected linear handshake payload sequence right behind the header
                    memcpy(out_hs + DTLS_HANDSHAKE_HEADER_LEN, reassembler->reassembly_buf, reassembler->total_length);
                    *out_len = total_record_len;

                    // Release local allocation memory structures for this message stream
                    free(reassembler->reassembly_buf);
                    reassembler->reassembly_buf = NULL;
                    free(reassembler->bitmap);
                    reassembler->bitmap = NULL;
                    return 1; // Completed full message reconstruction
                }
                return 0; // Intercepted and cached successfully, awaiting more fragments
            }
        }

        // Pass-through processing lane for unfragmented packets or encrypted epochs
//        if (*out_len < packet_len) return -1;
//        memcpy(out_buf, packet, packet_len);
//        *out_len = packet_len;
        return 2;
    }

#endif //BIO_UDP_HANDSHAKE
