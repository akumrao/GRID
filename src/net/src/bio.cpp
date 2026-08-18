#if USE_MBEDTLS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "net/bio.h"

/* Return the number of pending bytes in read and write buffers */
int TLS_BIO_ctrl_pending(BIO* bio) {
    if (bio == NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_ctrl_pending: BIO context is NULL\n");
        return 0;
    }

    if (bio->type == BIO_MEMORY) {
        return bio->memLen < 0 ? 0 : bio->memLen;
    }

    /* type BIO_BIO then check paired buffer */
    if (bio->type == BIO_BIO && bio->pair != NULL) {
        BIO* pair = bio->pair;

        if (pair->wrIdx > 0 && pair->wrIdx <= pair->rdIdx) {
            /* in wrap around state where beginning of buffer is being overwritten */
            return pair->wrSz - pair->rdIdx + pair->wrIdx;
        } else {
            /* simple case where has not wrapped around */
            return pair->wrIdx - pair->rdIdx;
        }
    }
    return 0;
}

int TLS_BIO_set_write_buf_size(BIO* bio, int size) {
    if (bio == NULL || bio->type != BIO_BIO || size <= 0) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_set_write_buf_size: Invalid argument (bio=%p, size=%d)\n", (void*)bio, size);
        return SSL_FAILURE;
    }

    /* if already in pair then do not change size */
    if (bio->pair != NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_set_write_buf_size: Cannot resize existing BIO pair\n");
        return SSL_FAILURE;
    }

    bio->wrSz = size;

    if (bio->mem != NULL) {
        free(bio->mem);
    }

    bio->mem = (BYTE*)malloc((size_t)bio->wrSz);
    if (bio->mem == NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_set_write_buf_size: Memory allocation failed for %d bytes\n", bio->wrSz);
        return SSL_FAILURE;
    }
    bio->wrIdx = 0;
    bio->rdIdx = 0;

    return SSL_SUCCESS;
}

int TLS_BIO_make_bio_pair(BIO* b1, BIO* b2) {
    if (b1 == NULL || b2 == NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_make_bio_pair: One or both BIO pointers are NULL\n");
        return SSL_FAILURE;
    }

    /* both are expected to be of type BIO and not already paired */
    if (b1->type != BIO_BIO || b2->type != BIO_BIO || b1->pair != NULL || b2->pair != NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_make_bio_pair: BIOs must be type BIO_BIO and unpaired\n");
        return SSL_FAILURE;
    }

    /* set default write size if not already set */
    if (b1->mem == NULL && TLS_BIO_set_write_buf_size(b1, SSL_BIO_SIZE) != SSL_SUCCESS) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_make_bio_pair: Failed to allocate b1 buffer\n");
        return SSL_FAILURE;
    }

    if (b2->mem == NULL && TLS_BIO_set_write_buf_size(b2, SSL_BIO_SIZE) != SSL_SUCCESS) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_make_bio_pair: Failed to allocate b2 buffer\n");
        return SSL_FAILURE;
    }

    b1->pair = b2;
    b2->pair = b1;

    return SSL_SUCCESS;
}

/* Does not advance read index pointer */
int TLS_BIO_nread0(BIO* bio, char** buf) {
    if (bio == NULL || buf == NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_nread0: Invalid NULL parameters\n");
        return 0;
    }

    /* if paired read from pair */
    if (bio->pair != NULL) {
        BIO* pair = bio->pair;

        /* case where have wrapped around write buffer */
        *buf = (char*)pair->mem + pair->rdIdx;
        if (pair->wrIdx > 0 && pair->rdIdx >= pair->wrIdx) {
            return pair->wrSz - pair->rdIdx;
        } else {
            return pair->wrIdx - pair->rdIdx;
        }
    }
    return 0;
}

/* advances the read index safely */
int TLS_BIO_nread(BIO* bio, char** buf, int num) {
    int sz = SSL_BIO_UNSET;

    if (bio == NULL || buf == NULL || num < 0) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_nread: Invalid arguments (bio=%p, buf=%p, num=%d)\n", (void*)bio, (void*)buf, num);
        return SSL_BIO_ERROR;
    }

    if (bio->pair != NULL) {
        /* special case if asking to read 0 bytes */
        if (num == 0) {
            *buf = (char*)bio->pair->mem + bio->pair->rdIdx;
            return 0;
        }

        /* get amount able to read and set buffer pointer */
        sz = TLS_BIO_nread0(bio, buf);
        if (sz <= 0) {
            return SSL_BIO_ERROR;
        }

        if (num < sz) {
            sz = num;
        }
        bio->pair->rdIdx += sz;

        /* check if have read to the end of the buffer and need to reset */
        if (bio->pair->rdIdx == bio->pair->wrSz) {
            bio->pair->rdIdx = 0;
            if (bio->pair->wrIdx == bio->pair->wrSz) {
                bio->pair->wrIdx = 0;
            }
        }

        /* check if read up to write index, if so then reset indexs */
        if (bio->pair->rdIdx == bio->pair->wrIdx) {
            bio->pair->rdIdx = 0;
            bio->pair->wrIdx = 0;
        }
    } else {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_nread: BIO pair is missing\n");
        return SSL_BIO_ERROR;
    }

    return sz;
}

int TLS_BIO_nwrite(BIO* bio, char** buf, int num) {
    int sz = SSL_BIO_UNSET;

    if (bio == NULL || buf == NULL || num < 0) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_nwrite: Invalid arguments (bio=%p, buf=%p, num=%d)\n", (void*)bio, (void*)buf, num);
        return SSL_BIO_ERROR;
    }

    if (bio->pair != NULL) {
        if (num == 0) {
            *buf = (char*)bio->mem + bio->wrIdx;
            return 0;
        }

        if (bio->wrIdx < bio->rdIdx) {
            /* wrapped around write mode */
            sz = bio->rdIdx - bio->wrIdx;
        } else if (bio->rdIdx > 0 && bio->wrIdx == bio->rdIdx) {
            fprintf(stderr, "[BIO ERROR] TLS_BIO_nwrite: Buffer full (wrIdx=%d == rdIdx=%d)\n", bio->wrIdx, bio->rdIdx);
            return SSL_BIO_ERROR; /* no room to write */
        } else {
            /* write index is past read index so write to end of buffer */
            sz = bio->wrSz - bio->wrIdx;

            if (sz <= 0) {
                if (bio->rdIdx == 0) {
                    fprintf(stderr, "[BIO ERROR] TLS_BIO_nwrite: Write buffer completely filled\n");
                    return SSL_BIO_ERROR;
                }

                bio->wrIdx = 0;

                if (bio->rdIdx > 0) {
                    sz = bio->rdIdx;
                } else {
                    sz = bio->wrSz;
                }
            }
        }

        if (num < sz) {
            sz = num;
        }
        *buf = (char*)bio->mem + bio->wrIdx;
        bio->wrIdx += sz;

        /* wrap write index if at memory end and read index has advanced */
        if (bio->wrIdx == bio->wrSz && bio->rdIdx > 0) {
            bio->wrIdx = 0;
        }
    } else {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_nwrite: BIO pair is missing\n");
        return SSL_BIO_ERROR;
    }

    return sz;
}

int TLS_BIO_reset(BIO* bio) {
    if (bio == NULL) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_reset: BIO handle is NULL\n");
        return SSL_BIO_ERROR;
    }

    switch (bio->type) {
        case BIO_BIO:
            bio->rdIdx = 0;
            bio->wrIdx = 0;
            return 0;

        default:
            fprintf(stderr, "[BIO ERROR] TLS_BIO_reset: Unsupported BIO type %d\n", bio->type);
            break;
    }

    return SSL_BIO_ERROR;
}

int TLS_BIO_read(BIO* bio, char* buf, int size) {
    if (bio == NULL || buf == NULL || size <= 0) {
        fprintf(stderr, "[BIO ERROR] TLS_BIO_read: Invalid arguments (bio=%p, buf=%p, size=%d)\n", (void*)bio, (void*)buf, size);
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
        fprintf(stderr, "[BIO ERROR] TLS_BIO_write: Invalid arguments (bio=%p, buf=%p, size=%d)\n", (void*)bio, (void*)buf, size);
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
        fprintf(stderr, "[BIO ERROR] SSL_BIO_new: Failed allocating BIO struct\n");
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

    return bio;
}

int TLS_BIO_free(BIO* bio) {
    if (bio) {
        if (bio->pair != NULL) {
            bio->pair->pair = NULL;
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

int TLS_BIO_net_send(void* ctx, const unsigned char* buf, int len) {
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

int TLS_BIO_net_recv(void* ctx, unsigned char* buf, int len) {
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