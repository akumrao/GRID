
/* A BIO (Basic I/O) abstraction layer provides an in-memory ring-buffer pipeline to connect mbedTLS with custom transport layers 
 without requiring direct socket attachments.  Primary Use CasesCustom Transport Security: Interfacing mbedTLS SSL/TLS operations
 with custom network layers (e.g., WebSockets, custom UDP, IPC, non-blocking asynchronous event loops) using TLS_BIO_net_send and 
 TLS_BIO_net_recv as mbedTLS callbacks.  In-Memory SSL Tunneling: Using paired memory BIOs (BIO_BIO) to route encrypted data back 
 and forth between two local components or through a proxy pipeline without touching actual network sockets.  
 
*/

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
  int occupied; /* FIXED: Tracks exact byte count in buffer to resolve wrIdx == rdIdx ambiguity */
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
int TLS_BIO_set_write_buf_size(BIO* bio, int size);

int TLS_BIO_ctrl_pending(BIO* bio);
int TLS_BIO_nread0(BIO* bio, char** buf);
int TLS_BIO_nread(BIO* bio, char** buf, int num);
int TLS_BIO_nwrite(BIO* bio, char** buf, int num);
int TLS_BIO_read(BIO* bio, char* buf, int size);
int TLS_BIO_write(BIO* bio, const char* buf, int size);

int TLS_BIO_reset(BIO* bio);
int TLS_BIO_net_recv(void* ctx, unsigned char* buf, int len);
int TLS_BIO_net_send(void* ctx, const unsigned char* buf, int len);
int TLS_BIO_free_all(BIO* bio);
int TLS_BIO_free(BIO* bio);

#ifdef __cplusplus
};
#endif

#endif //CHAT_BIO_H
