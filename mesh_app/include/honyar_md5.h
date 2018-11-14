#ifndef INCLUDE_EG_MD5_H_
#define INCLUDE_EG_MD5_H_

#include "mbedtls/md5.h"
#include "c_types.h"

#define MD5_LEN  16

typedef mbedtls_md5_context md5_context_t;


int32_t honyar_md5_start(md5_context_t *ctx);

int32_t honyar_md5_update(md5_context_t *ctx, const uint8_t *input, uint32_t len);

int32_t honyar_md5_finish(md5_context_t *ctx, uint8_t output[16]);

int32_t honyar_md5(const uint8_t *input, uint32_t len, uint8_t output[16]);

#endif /* INCLUDE_EG_MD5_H_ */

