//
// Created by Gemini on 2017/6/23.
//

#ifndef CHAT_BIO_H
#define CHAT_BIO_H

#include "mbedtls/ssl.h"




enum { /* ssl Constants */
       SSL_FAILURE = 0,
       SSL_SUCCESS = 1
};


#ifdef __cplusplus
extern "C" {
#endif



typedef unsigned char BYTE;

struct _BIO;
typedef struct _BIO BIO;

struct _BIO {
  BIO* prev;  /* previous in chain */
  BIO* next;  /* next in chain */
  BIO* pair;  /* BIO paired with */
  BYTE* mem;  /* memory buffer */
  int wrSz;   /* write buffer size (mem) */
  int wrIdx;  /* current index for write buffer */
  int rdIdx;  /* current read index */
  int readRq; /* read request */
  int memLen; /* memory buffer length */
  int type;   /* method type */
};

enum {
  SSL_BIO_ERROR = -1,
  SSL_BIO_UNSET = -2,
  SSL_BIO_SIZE = 17000 /* default BIO write size if not set */
};

enum BIO_TYPE {
  BIO_BUFFER = 1,
  BIO_SOCKET = 2,
  BIO_SSL = 3,
  BIO_MEMORY = 4,
  BIO_BIO = 5,
  BIO_FILE = 6
};

BIO* SSL_BIO_new(int type);
int TLS_BIO_make_bio_pair(BIO* b1, BIO* b2);

size_t TLS_BIO_ctrl_pending(BIO* bio);
int TLS_BIO_read(BIO* bio, const char* buf, size_t size);
int TLS_BIO_write(BIO* bio, const char* buf, size_t size);

int TLS_BIO_reset(BIO* bio);
int TLS_BIO_net_recv(void* ctx, unsigned char* buf, size_t len);
int TLS_BIO_net_send(void* ctx, const unsigned char* buf, size_t len);
int TLS_BIO_free_all(BIO* bio);
int TLS_BIO_free(BIO* bio);












#ifdef __cplusplus
};
#endif

#endif //CHAT_BIO_H
