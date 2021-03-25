/* digest.c
 *
 * Copyright (C) 2019-2021 wolfSSL Inc.
 *
 * This file is part of wolfEngine.
 *
 * wolfEngine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfengine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#include "wolfengine.h"

#if defined(WE_HAVE_SHA256) && defined(WE_SHA256_DIRECT)

/*
 * SHA-256
 */

/**
 * Initialize the SHA-256 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha256_init(EVP_MD_CTX *ctx)
{
    int ret = 1;

    WOLFENGINE_ENTER("we_sha256_init");

    ret = wc_InitSha256((wc_Sha256*)EVP_MD_CTX_md_data(ctx));
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_InitSha256", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha256_init", ret);

    return ret;
}

/**
 * Digest some more data with SHA-256 using wolfSSL.
 *
 * @param  ctx   [in]  EVP digest context of operation.
 * @param  data  [in]  More data to digest with SHA-256.
 * @param  len   [in]  Length of data to digest.
 * @return  1 on success and 0 on failure.
 */
static int we_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t len)
{
    int ret = 1;

    WOLFENGINE_ENTER("we_sha256_update");

    ret = wc_Sha256Update((wc_Sha256*)EVP_MD_CTX_md_data(ctx),
                          (const byte*)data, (word32)len);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_Sha256Update", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha256_update", ret);

    return ret;
}

/**
 * Finalize the SHA-256 digest operation.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @param  md   [in]  SHA-256 digest of data.
 * @return  1 on success and 0 on failure.
 */
static int we_sha256_final(EVP_MD_CTX *ctx, unsigned char *md)
{
    int ret = 1;

    WOLFENGINE_ENTER("we_sha256_final");

    ret = wc_Sha256Final((wc_Sha256*)EVP_MD_CTX_md_data(ctx), (byte*)md);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_Sha256Final", ret);
        ret = 0;
    } else {
        WOLFENGINE_MSG("SHA-256 Digest");
        WOLFENGINE_BUFFER(md, WC_SHA256_DIGEST_SIZE);
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha256_final", ret);

    return ret;
}

/**
 * Cleanup the SHA-256 digest object.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 for success.
 */
static int we_sha256_cleanup(EVP_MD_CTX *ctx)
{
    WOLFENGINE_ENTER("we_sha256_cleanup");

    wc_Sha256Free((wc_Sha256*)EVP_MD_CTX_md_data(ctx));

    WOLFENGINE_LEAVE("we_sha256_cleanup");
    return 1;
}

/** EVP digest method - SHA-256 using wolfSSL for the implementation. */
EVP_MD *we_sha256_md = NULL;

/**
 * Initialize the global SHA-256 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha256_meth()
{
    int ret;

    WOLFENGINE_ENTER("we_init_sha256_meth");

    ret = (we_sha256_md = EVP_MD_meth_new(NID_sha256, EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha256_md, we_sha256_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_update(we_sha256_md, we_sha256_update);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_final(we_sha256_md, we_sha256_final);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_cleanup(we_sha256_md, we_sha256_cleanup);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha256_md, WC_SHA256_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_app_datasize(we_sha256_md, sizeof(wc_Sha256));
    }

    if ((ret != 1) && (we_sha256_md != NULL)) {
        EVP_MD_meth_free(we_sha256_md);
    }

    WOLFENGINE_LEAVE("we_init_sha256_meth", ret);

    return ret;
};

#endif /* WE_HAVE_SHA256 && WE_SHA256_DIRECT */

#ifdef WE_USE_HASH

/**
 * Data required to complete an AES-GCM encrypt/decrypt operation.
 */
typedef struct we_Digest
{
    wc_HashAlg       hash;
    enum wc_HashType hashType;
} we_Digest;

#ifdef WE_HAVE_SHA256
/**
 * Initialize the SHA-256 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha256_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha256_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA256;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha256_init", ret);

    return ret;
}
#endif

#ifdef WE_HAVE_SHA384
/**
 * Initialize the SHA-384 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha384_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha384_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA384;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha384_init", ret);

    return ret;
}
#endif

#ifdef WE_HAVE_SHA512
/**
 * Initialize the SHA-512 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha512_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha512_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA512;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha512_init", ret);

    return ret;
}
#endif

#ifdef WE_HAVE_SHA3_224
/**
 * Initialize the SHA3-224 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha3_224_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha3_224_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA3_224;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha3_224_init", ret);

    return ret;
}
#endif

#ifdef WE_HAVE_SHA3_256
/**
 * Initialize the SHA3-256 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha3_256_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha3_256_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA3_256;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha3_256_init", ret);

    return ret;
}
#endif

#ifdef WE_HAVE_SHA3_384
/**
 * Initialize the SHA3-384 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha3_384_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha3_384_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA3_384;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha3_384_init", ret);

    return ret;
}
#endif

#ifdef WE_HAVE_SHA3_512
/**
 * Initialize the SHA3-512 digest operation using wolfSSL.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 on success and 0 on failure.
 */
static int we_sha3_512_init(EVP_MD_CTX *ctx)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_sha3_512_init");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);
    digest->hashType = WC_HASH_TYPE_SHA3_512;

    ret = wc_HashInit(&digest->hash, digest->hashType);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashInit", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_sha3_512_init", ret);

    return ret;
}
#endif

/**
 * Digest some more data using wolfSSL.
 *
 * @param  ctx   [in]  EVP digest context of operation.
 * @param  data  [in]  More data to digest with SHA-256.
 * @param  len   [in]  Length of data to digest.
 * @return  1 on success and 0 on failure.
 */
static int we_digest_update(EVP_MD_CTX *ctx, const void *data, size_t len)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_digest_update");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);

    ret = wc_HashUpdate(&digest->hash, digest->hashType, (const byte*)data,
                         (word32)len);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashUpdate", ret);
        ret = 0;
    } else {
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_digest_update", ret);

    return ret;
}

/**
 * Finalize the digest operation.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @param  md   [in]  SHA-256 digest of data.
 * @return  1 on success and 0 on failure.
 */
static int we_digest_final(EVP_MD_CTX *ctx, unsigned char *md)
{
    int ret = 1;
    we_Digest *digest;

    WOLFENGINE_ENTER("we_digest_final");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);

    ret = wc_HashFinal(&digest->hash, digest->hashType, (byte*)md);
    if (ret != 0) {
        WOLFENGINE_ERROR_FUNC("wc_HashFinal", ret);
        ret = 0;
    } else {
        WOLFENGINE_MSG("Message Digest");
        WOLFENGINE_BUFFER(md, wc_HashGetDigestSize(digest->hashType));
        ret = 1;
    }

    WOLFENGINE_LEAVE("we_digest_final", ret);

    return ret;
}

/**
 * Cleanup the digest object.
 *
 * @param  ctx  [in]  EVP digest context of operation.
 * @return  1 for success.
 */
static int we_digest_cleanup(EVP_MD_CTX *ctx)
{
    int ret = 1;
#if !defined(HAVE_FIPS_VERSION) || HAVE_FIPS_VERSION >= 2
    we_Digest *digest;

    WOLFENGINE_ENTER("we_digest_cleanup");

    digest = (we_Digest *)EVP_MD_CTX_md_data(ctx);

    if (digest != NULL) {
        ret = wc_HashFree(&digest->hash, digest->hashType);
        if (ret != 0) {
            WOLFENGINE_ERROR_FUNC("wc_HashFree", ret);
            ret = 0;
        } else {
            ret = 1;
        }
    }
#else
    WOLFENGINE_ENTER("we_digest_cleanup");

    (void)ctx;
    ret = 1; 
#endif

    WOLFENGINE_LEAVE("we_digest_cleanup", ret);
    return ret;
}

/**
 * Initialize the EVP digest method.
 *
 * @param  method  [in]  EVP digest method to modify.
 * @return  1 on success else failure.
 */
static int we_init_digest_meth(EVP_MD *method)
{
    int ret;

    WOLFENGINE_ENTER("we_init_digest_meth");

    ret = EVP_MD_meth_set_update(method, we_digest_update);
    if (ret == 1) {
        ret = EVP_MD_meth_set_final(method, we_digest_final);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_cleanup(method, we_digest_cleanup);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_app_datasize(method, sizeof(we_Digest));
    }

#ifdef WE_HAVE_EVP_PKEY
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (ret == 1) {
        const int *nids;
        int cnt;

        cnt = we_pkey_get_nids(&nids);
        XMEMCPY(method->required_pkey_type, nids, cnt);
        method->flags |= EVP_MD_FLAG_PKEY_METHOD_SIGNATURE;
    }
#endif
#endif

    WOLFENGINE_LEAVE("we_init_digest_meth", ret);

    return ret;
}

#ifdef WE_HAVE_SHA256
/** EVP digest method - SHA-256 using wolfSSL for the implementation. */
EVP_MD *we_sha256_md = NULL;

/**
 * Initialize the global SHA-256 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha256_meth()
{
    int ret;

    WOLFENGINE_ENTER("we_init_sha256_meth");

    ret = (we_sha256_md = EVP_MD_meth_new(NID_sha256, EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha256_md, we_sha256_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha256_md, WC_SHA256_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha256_md);
    }

    if ((ret != 1) && (we_sha256_md != NULL)) {
        EVP_MD_meth_free(we_sha256_md);
    }

    WOLFENGINE_LEAVE("we_init_sha256_meth", ret);

    return ret;
};
#endif

#ifdef WE_HAVE_SHA384
/** EVP digest method - SHA-384 using wolfSSL for the implementation. */
EVP_MD *we_sha384_md = NULL;

/**
 * Initialize the global SHA-384 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha384_meth()
{
    int ret;

    WOLFENGINE_ENTER("we_init_sha384_meth");

    ret = (we_sha384_md = EVP_MD_meth_new(NID_sha384, EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha384_md, we_sha384_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha384_md, WC_SHA384_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha384_md);
    }

    if ((ret != 1) && (we_sha384_md != NULL)) {
        EVP_MD_meth_free(we_sha384_md);
    }

    WOLFENGINE_LEAVE("we_init_sha384_meth", ret);

    return ret;
};
#endif

#ifdef WE_HAVE_SHA512
/** EVP digest method - SHA-512 using wolfSSL for the implementation. */
EVP_MD *we_sha512_md = NULL;

/**
 * Initialize the global SHA-512 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha512_meth()
{
    int ret = 1;

    WOLFENGINE_ENTER("we_init_sha512_meth");

    ret = (we_sha512_md = EVP_MD_meth_new(NID_sha512, EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha512_md, we_sha512_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha512_md, WC_SHA512_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha512_md);
    }

    if ((ret != 1) && (we_sha512_md != NULL)) {
        EVP_MD_meth_free(we_sha512_md);
    }

    WOLFENGINE_LEAVE("we_init_sha512_meth", ret);

    return ret;
};
#endif

#ifdef WE_HAVE_SHA3_224
/** EVP digest method - SHA3-224 using wolfSSL for the implementation. */
EVP_MD *we_sha3_224_md = NULL;

/**
 * Initialize the global SHA3-224 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha3_224_meth()
{
    int ret = 1;

    WOLFENGINE_ENTER("we_init_sha3_224_meth");

    ret = (we_sha3_224_md = EVP_MD_meth_new(NID_sha3_224,
                                            EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha3_224_md, we_sha3_224_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha3_224_md,
                                          WC_SHA3_224_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha3_224_md);
    }

    if ((ret != 1) && (we_sha3_224_md != NULL)) {
        EVP_MD_meth_free(we_sha3_224_md);
    }

    WOLFENGINE_LEAVE("we_init_sha3_224_meth", ret);

    return ret;
};
#endif

#ifdef WE_HAVE_SHA3_256
/** EVP digest method - SHA3-256 using wolfSSL for the implementation. */
EVP_MD *we_sha3_256_md = NULL;

/**
 * Initialize the global SHA3-256 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha3_256_meth()
{
    int ret = 1;

    WOLFENGINE_ENTER("we_init_sha3_256_meth");

    ret = (we_sha3_256_md = EVP_MD_meth_new(NID_sha3_256,
                                            EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha3_256_md, we_sha3_256_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha3_256_md,
                                          WC_SHA3_256_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha3_256_md);
    }

    if ((ret != 1) && (we_sha3_256_md != NULL)) {
        EVP_MD_meth_free(we_sha3_256_md);
    }

    WOLFENGINE_LEAVE("we_init_sha3_256_meth", ret);

    return ret;
};
#endif

#ifdef WE_HAVE_SHA3_384
/** EVP digest method - SHA3-384 using wolfSSL for the implementation. */
EVP_MD *we_sha3_384_md = NULL;

/**
 * Initialize the global SHA3-384 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha3_384_meth()
{
    int ret = 1;

    WOLFENGINE_ENTER("we_init_sha3_384_meth");

    ret = (we_sha3_384_md = EVP_MD_meth_new(NID_sha3_384,
                                            EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha3_384_md, we_sha3_384_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha3_384_md,
                                          WC_SHA3_384_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha3_384_md);
    }

    if ((ret != 1) && (we_sha3_384_md != NULL)) {
        EVP_MD_meth_free(we_sha3_384_md);
    }

    WOLFENGINE_LEAVE("we_init_sha3_384_meth", ret);

    return ret;
};
#endif

#ifdef WE_HAVE_SHA3_512
/** EVP digest method - SHA3-512 using wolfSSL for the implementation. */
EVP_MD *we_sha3_512_md = NULL;

/**
 * Initialize the global SHA3-512 EVP digest method.
 *
 * @return  1 on success else failure.
 */
int we_init_sha3_512_meth()
{
    int ret = 1;

    WOLFENGINE_ENTER("we_init_sha3_512_meth");

    ret = (we_sha3_512_md = EVP_MD_meth_new(NID_sha3_512,
                                            EVP_PKEY_NONE)) != NULL;
    if (ret == 1) {
        ret = EVP_MD_meth_set_init(we_sha3_512_md, we_sha3_512_init);
    }
    if (ret == 1) {
        ret = EVP_MD_meth_set_result_size(we_sha3_512_md,
                                          WC_SHA3_512_DIGEST_SIZE);
    }
    if (ret == 1) {
        ret = we_init_digest_meth(we_sha3_512_md);
    }

    if ((ret != 1) && (we_sha3_512_md != NULL)) {
        EVP_MD_meth_free(we_sha3_512_md);
    }

    WOLFENGINE_LEAVE("we_init_sha3_512_meth", ret);

    return ret;
};
#endif

#endif /* WE_USE_HASH */


