
/* 
 * Note: Bio is Ring Buffer, but it is not thread safe . 
 * 
 * A BIO (Basic I/O) abstraction layer provides an in-memory ring-buffer pipeline to connect mbedTLS with custom transport layers 
 without requiring direct socket attachments.  Primary Use CasesCustom Transport Security: Interfacing mbedTLS SSL/TLS operations
 with custom network layers (e.g., WebSockets, custom UDP, IPC, non-blocking asynchronous event loops) using TLS_BIO_net_send and 
 TLS_BIO_net_recv as mbedTLS callbacks.  In-Memory SSL Tunneling: Using paired memory BIOs (BIO_BIO) to route encrypted data back 
 and forth between two local components or through a proxy pipeline without touching actual network sockets.  
 
*/

#if USE_MBEDTLS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "net/bio.h"
#include "base/logger.h"
using namespace base;

/* Return the number of pending bytes in read and write buffers */
int TLS_BIO_ctrl_pending(BIO* bio) {
    if (bio == NULL) {
        SError << "[BIO ERROR] TLS_BIO_ctrl_pending: BIO context is NULL";
        return 0;
    }

    if (bio->type == BIO_MEMORY) {
        return bio->memLen < 0 ? 0 : bio->memLen;
    }

    /* FIXED: Use pair's occupied byte count directly to eliminate ambiguity */
    if (bio->type == BIO_BIO && bio->pair != NULL) {
        return bio->pair->occupied;
    }
    return 0;
}

int TLS_BIO_set_write_buf_size(BIO* bio, int size) {
    if (bio == NULL || bio->type != BIO_BIO || size <= 0) {
        SError << "[BIO ERROR] TLS_BIO_set_write_buf_size: Invalid argument (bio=" 
                  << (void*)bio << ", size=" << size << ")";
        return SSL_FAILURE;
    }

    /* if already in pair then do not change size */
    if (bio->pair != NULL) {
        SError << "[BIO ERROR] TLS_BIO_set_write_buf_size: Cannot resize existing BIO pair";
        return SSL_FAILURE;
    }

    bio->wrSz = size;

    if (bio->mem != NULL) {
        free(bio->mem);
    }

    bio->mem = (BYTE*)malloc((size_t)bio->wrSz);
    if (bio->mem == NULL) {
        SError << "[BIO ERROR] TLS_BIO_set_write_buf_size: Memory allocation failed for " 
                  << bio->wrSz << " bytes";
        return SSL_FAILURE;
    }
    bio->wrIdx = 0;
    bio->rdIdx = 0;
    bio->occupied = 0; /* FIXED: Initialize byte count */

    return SSL_SUCCESS;
}

int TLS_BIO_make_bio_pair(BIO* b1, BIO* b2) {
    if (b1 == NULL || b2 == NULL) {
        SError << "[BIO ERROR] TLS_BIO_make_bio_pair: One or both BIO pointers are NULL";
        return SSL_FAILURE;
    }

    /* both are expected to be of type BIO and not already paired */
    if (b1->type != BIO_BIO || b2->type != BIO_BIO || b1->pair != NULL || b2->pair != NULL) {
        SError << "[BIO ERROR] TLS_BIO_make_bio_pair: BIOs must be type BIO_BIO and unpaired";
        return SSL_FAILURE;
    }

    /* set default write size if not already set */
    if (b1->mem == NULL && TLS_BIO_set_write_buf_size(b1, SSL_BIO_SIZE) != SSL_SUCCESS) {
        SError << "[BIO ERROR] TLS_BIO_make_bio_pair: Failed to allocate b1 buffer";
        return SSL_FAILURE;
    }

    if (b2->mem == NULL && TLS_BIO_set_write_buf_size(b2, SSL_BIO_SIZE) != SSL_SUCCESS) {
        SError << "[BIO ERROR] TLS_BIO_make_bio_pair: Failed to allocate b2 buffer";
        return SSL_FAILURE;
    }

    b1->pair = b2;
    b2->pair = b1;

    return SSL_SUCCESS;
}

/* Does not advance read index pointer */
int TLS_BIO_nread0(BIO* bio, char** buf) {
    if (bio == NULL || buf == NULL) {
        SError << "[BIO ERROR] TLS_BIO_nread0: Invalid NULL parameters";
        return 0;
    }

    /* if paired read from pair */
    if (bio->pair != NULL) {
        BIO* pair = bio->pair;

        if (pair->occupied == 0) {
            return 0;
        }

        *buf = (char*)pair->mem + pair->rdIdx;
        
        /* FIXED: Determine contiguous readable region up to physical memory boundary */
        if (pair->wrIdx > pair->rdIdx) {
            return pair->wrIdx - pair->rdIdx;
        } else {
            return pair->wrSz - pair->rdIdx;
        }
    }
    return 0;
}

/* advances the read index safely */
int TLS_BIO_nread(BIO* bio, char** buf, int num) {
    int sz = SSL_BIO_UNSET;

    if (bio == NULL || buf == NULL || num < 0) {
        SError << "[BIO ERROR] TLS_BIO_nread: Invalid arguments (bio=" 
                  << (void*)bio << ", buf=" << (void*)buf << ", num=" << num << ")";
        return SSL_BIO_ERROR;
    }

    /* FIXED: Handle dangling pointer or uninitialized pair */
    if (bio->pair == NULL) {
        SError << "[BIO ERROR] TLS_BIO_nread: BIO pair is missing or freed";
        return SSL_BIO_ERROR;
    }

    BIO* pair = bio->pair;

    /* special case if asking to read 0 bytes */
    if (num == 0) {
        *buf = (char*)pair->mem + pair->rdIdx;
        return 0;
    }

    if (pair->occupied == 0) {
        return SSL_BIO_ERROR;
    }

    /* get amount able to read and set buffer pointer */
    sz = TLS_BIO_nread0(bio, buf);
    if (sz <= 0) {
        return SSL_BIO_ERROR;
    }

    if (num < sz) {
        sz = num;
    }

    /* FIXED: Wrap index predictably using modulo and update byte count */
    pair->rdIdx = (pair->rdIdx + sz) % pair->wrSz;
    pair->occupied -= sz;

    /* Reset indices to 0 when buffer is completely emptied */
    if (pair->occupied == 0) {
        pair->rdIdx = 0;
        pair->wrIdx = 0;
    }

    return sz;
}

int TLS_BIO_nwrite(BIO* bio, char** buf, int num) {
    int sz = SSL_BIO_UNSET;

    if (bio == NULL || buf == NULL || num < 0) {
        SError << "[BIO ERROR] TLS_BIO_nwrite: Invalid arguments (bio=" 
                  << (void*)bio << ", buf=" << (void*)buf << ", num=" << num << ")";
        return SSL_BIO_ERROR;
    }

    /* FIXED: Handle dangling pointer or uninitialized pair */
    if (bio->pair == NULL) {
        SError << "[BIO ERROR] TLS_BIO_nwrite: BIO pair is missing or freed";
        return SSL_BIO_ERROR;
    }

    if (num == 0) {
        *buf = (char*)bio->mem + bio->wrIdx;
        return 0;
    }

    /* FIXED: Unambiguous full check using byte counter */
    if (bio->occupied >= bio->wrSz) {
        SError << "[BIO ERROR] TLS_BIO_nwrite: Buffer full (occupied=" 
                  << bio->occupied << " >= wrSz=" << bio->wrSz << ")";
        return SSL_BIO_ERROR;
    }

    /* Calculate contiguous writable memory left in ring array */
    if (bio->wrIdx >= bio->rdIdx) {
        sz = bio->wrSz - bio->wrIdx;
        if (sz == 0 && bio->rdIdx > 0) {
            bio->wrIdx = 0;
            sz = bio->rdIdx;
        }
    } else {
        sz = bio->rdIdx - bio->wrIdx;
    }

    int space_left = bio->wrSz - bio->occupied;
    if (sz > space_left) {
        sz = space_left;
    }

    if (num < sz) {
        sz = num;
    }

    *buf = (char*)bio->mem + bio->wrIdx;
    
    /* FIXED: Wrap write index predictably using modulo and update byte count */
    bio->wrIdx = (bio->wrIdx + sz) % bio->wrSz;
    bio->occupied += sz;

    return sz;
}

int TLS_BIO_reset(BIO* bio) {
    if (bio == NULL) {
        SError << "[BIO ERROR] TLS_BIO_reset: BIO handle is NULL";
        return SSL_BIO_ERROR;
    }

    switch (bio->type) {
        case BIO_BIO:
            bio->rdIdx = 0;
            bio->wrIdx = 0;
            bio->occupied = 0; /* FIXED: Reset occupied counter */
            return 0;

        default:
            SError << "[BIO ERROR] TLS_BIO_reset: Unsupported BIO type " << bio->type;
            break;
    }

    return SSL_BIO_ERROR;
}

int TLS_BIO_read(BIO* bio, char* buf, int size) {
    if (bio == NULL || buf == NULL || size <= 0) {
        SError << "[BIO ERROR] TLS_BIO_read: Invalid arguments (bio=" 
                  << (void*)bio << ", buf=" << (void*)buf << ", size=" << size << ")";
        return SSL_BIO_ERROR;
    }

    char *pt = NULL;
    int sz = TLS_BIO_nread(bio, &pt, size);

    if (sz > 0) {
        memset((void*)buf, 0, (size_t)sz);
        memcpy((void*)buf, pt, (size_t)sz);
    }

    return sz;
}

int TLS_BIO_write(BIO* bio, const char* buf, int size) {
    if (bio == NULL || buf == NULL || size <= 0) {
        SError << "[BIO ERROR] TLS_BIO_write: Invalid arguments (bio=" 
                  << (void*)bio << ", buf=" << (void*)buf << ", size=" << size << ")";
        return SSL_BIO_ERROR;
    }

    char* data = NULL;
    int sz = TLS_BIO_nwrite(bio, &data, size);

    if (sz <= 0) {
        return sz;
    }

    memset(data, 0, (size_t)sz);
    memcpy(data, buf, (size_t)sz);
    return sz;
}

BIO* SSL_BIO_new(int type) {
    BIO* bio = (BIO*)malloc(sizeof(BIO));
    if (bio == NULL) {
        SError << "[BIO ERROR] SSL_BIO_new: Failed allocating BIO struct";
        return NULL;
    }

#ifdef _MSC_VER
    memset(bio, 0, sizeof(BIO));
#else
    bzero(bio, sizeof(BIO));
#endif

    bio->type = type;
    bio->mem = NULL;
    bio->prev = NULL;
    bio->next = NULL;
    bio->occupied = 0;

    return bio;
}

int TLS_BIO_free(BIO* bio) {
    if (bio) {
        if (bio->pair != NULL) {
            bio->pair->pair = NULL;
            bio->pair = NULL;
        }
        if (bio->mem) {
            free(bio->mem);
            bio->mem = NULL;
        }
        free(bio);
    }
    return 0;
}

int TLS_BIO_free_all(BIO* bio) {
    while (bio) {
        BIO* next = bio->next;
        TLS_BIO_free(bio);
        bio = next;
    }
    return 0;
}

int TLS_BIO_net_send(void *ctx, const unsigned char *buf, size_t len) {
    if (ctx == NULL || buf == NULL || len <= 0) {
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }

    BIO* bio = (BIO*)ctx;
    int sz = TLS_BIO_write(bio, (const char*)buf, len);

    if (sz <= 0) {
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    return sz;
}

int TLS_BIO_net_recv(void *ctx, unsigned char *buf, size_t len) {
    if (ctx == NULL || buf == NULL || len <= 0) {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    BIO* bio = (BIO*)ctx;
    int sz = TLS_BIO_read(bio, (char*)buf, len);

    if (sz <= 0) {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }
    return sz;
}

#endif
