#include "honyar_common.h"

int32_t honyar_md5_start(md5_context_t *ctx)
{
    mbedtls_md5_init(ctx);
    mbedtls_md5_starts(ctx);
    return 0;
}

int32_t honyar_md5_update(md5_context_t *ctx, const uint8_t *input, uint32_t len)
{
    mbedtls_md5_update(ctx, input, len);
    return 0;
}

int32_t honyar_md5_finish(md5_context_t *ctx, uint8_t output[16])
{
    mbedtls_md5_finish(ctx, output);
    return 0;
}

int32_t honyar_md5(const uint8_t *input, uint32_t len, uint8_t output[16])
{
    mbedtls_md5(input, len, output);
    return 0;
}

