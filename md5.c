#include <string.h>
#include "md5.h"

/* ==========================================================================
 * Function:    md5str ( instr )
 * Purpose: returns null-terminated character string containing
 *      md5 hash of instr (input string)
 * --------------------------------------------------------------------------
 * Arguments:   instr (I)   pointer to null-terminated char string
 *              containing input string whose md5 hash
 *              is desired
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to null-terminated 32-character
 *              md5 hash of instr
 * --------------------------------------------------------------------------
 * Notes:     o Other md5 library functions are included below.
 *      They're all taken from Christophe Devine's code,
 *      which (as of 04-Aug-2004) is available from
 *           http://www.cr0.net:8040/code/crypto/md5/
 *        o The P,F,S macros in the original code are replaced
 *      by four functions P1()...P4() to accommodate a problem
 *      with Compaq's vax/vms C compiler.
 * ======================================================================= */
#define GET_UINT32(n,b,i)                       \
  { (n) = ( (uint32_t) (b)[(i)    ]       )       \
        | ( (uint32_t) (b)[(i) + 1] <<  8 )       \
        | ( (uint32_t) (b)[(i) + 2] << 16 )       \
        | ( (uint32_t) (b)[(i) + 3] << 24 ); }
#define PUT_UINT32(n,b,i)                       \
  { (b)[(i)    ] = (uint8_t) ( (n)       );       \
    (b)[(i) + 1] = (uint8_t) ( (n) >>  8 );       \
    (b)[(i) + 2] = (uint8_t) ( (n) >> 16 );       \
    (b)[(i) + 3] = (uint8_t) ( (n) >> 24 ); }
/* --- P,S,F macros defined as functions --- */
static void P1(uint32_t *X, uint32_t *a, uint32_t b, uint32_t c, uint32_t d, int k, int s, uint32_t t)
{
    *a += (uint32_t)(d ^(b & (c ^ d))) + X[k] + t;
    *a  = ((*a << s) | ((*a & 0xFFFFFFFF) >> (32 - s))) + b;
    return;
}
static void P2(uint32_t *X, uint32_t *a, uint32_t b, uint32_t c, uint32_t d, int k, int s, uint32_t t)
{
    *a += (uint32_t)(c ^(d & (b ^ c))) + X[k] + t;
    *a  = ((*a << s) | ((*a & 0xFFFFFFFF) >> (32 - s))) + b;
    return;
}
static void P3(uint32_t *X, uint32_t *a, uint32_t b, uint32_t c, uint32_t d, int k, int s, uint32_t t)
{
    *a += (uint32_t)(b ^ c ^ d) + X[k] + t;
    *a  = ((*a << s) | ((*a & 0xFFFFFFFF) >> (32 - s))) + b;
    return;
}
static void P4(uint32_t *X, uint32_t *a, uint32_t b, uint32_t c, uint32_t d, int k, int s, uint32_t t)
{
    *a += (uint32_t)(c ^(b | ~d)) + X[k] + t;
    *a  = ((*a << s) | ((*a & 0xFFFFFFFF) >> (32 - s))) + b;
    return;
}

/* --- entry point (all md5 functions below by Christophe Devine) --- */
void md5_starts(md5_context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

static void md5_process(md5_context *ctx, uint8_t data[64])
{
    uint32_t X[16], A, B, C, D;
    GET_UINT32(X[0],  data,  0);
    GET_UINT32(X[1],  data,  4);
    GET_UINT32(X[2],  data,  8);
    GET_UINT32(X[3],  data, 12);
    GET_UINT32(X[4],  data, 16);
    GET_UINT32(X[5],  data, 20);
    GET_UINT32(X[6],  data, 24);
    GET_UINT32(X[7],  data, 28);
    GET_UINT32(X[8],  data, 32);
    GET_UINT32(X[9],  data, 36);
    GET_UINT32(X[10], data, 40);
    GET_UINT32(X[11], data, 44);
    GET_UINT32(X[12], data, 48);
    GET_UINT32(X[13], data, 52);
    GET_UINT32(X[14], data, 56);
    GET_UINT32(X[15], data, 60);
    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    P1(X, &A, B, C, D,  0,  7, 0xD76AA478);
    P1(X, &D, A, B, C,  1, 12, 0xE8C7B756);
    P1(X, &C, D, A, B,  2, 17, 0x242070DB);
    P1(X, &B, C, D, A,  3, 22, 0xC1BDCEEE);
    P1(X, &A, B, C, D,  4,  7, 0xF57C0FAF);
    P1(X, &D, A, B, C,  5, 12, 0x4787C62A);
    P1(X, &C, D, A, B,  6, 17, 0xA8304613);
    P1(X, &B, C, D, A,  7, 22, 0xFD469501);
    P1(X, &A, B, C, D,  8,  7, 0x698098D8);
    P1(X, &D, A, B, C,  9, 12, 0x8B44F7AF);
    P1(X, &C, D, A, B, 10, 17, 0xFFFF5BB1);
    P1(X, &B, C, D, A, 11, 22, 0x895CD7BE);
    P1(X, &A, B, C, D, 12,  7, 0x6B901122);
    P1(X, &D, A, B, C, 13, 12, 0xFD987193);
    P1(X, &C, D, A, B, 14, 17, 0xA679438E);
    P1(X, &B, C, D, A, 15, 22, 0x49B40821);
    P2(X, &A, B, C, D,  1,  5, 0xF61E2562);
    P2(X, &D, A, B, C,  6,  9, 0xC040B340);
    P2(X, &C, D, A, B, 11, 14, 0x265E5A51);
    P2(X, &B, C, D, A,  0, 20, 0xE9B6C7AA);
    P2(X, &A, B, C, D,  5,  5, 0xD62F105D);
    P2(X, &D, A, B, C, 10,  9, 0x02441453);
    P2(X, &C, D, A, B, 15, 14, 0xD8A1E681);
    P2(X, &B, C, D, A,  4, 20, 0xE7D3FBC8);
    P2(X, &A, B, C, D,  9,  5, 0x21E1CDE6);
    P2(X, &D, A, B, C, 14,  9, 0xC33707D6);
    P2(X, &C, D, A, B,  3, 14, 0xF4D50D87);
    P2(X, &B, C, D, A,  8, 20, 0x455A14ED);
    P2(X, &A, B, C, D, 13,  5, 0xA9E3E905);
    P2(X, &D, A, B, C,  2,  9, 0xFCEFA3F8);
    P2(X, &C, D, A, B,  7, 14, 0x676F02D9);
    P2(X, &B, C, D, A, 12, 20, 0x8D2A4C8A);
    P3(X, &A, B, C, D,  5,  4, 0xFFFA3942);
    P3(X, &D, A, B, C,  8, 11, 0x8771F681);
    P3(X, &C, D, A, B, 11, 16, 0x6D9D6122);
    P3(X, &B, C, D, A, 14, 23, 0xFDE5380C);
    P3(X, &A, B, C, D,  1,  4, 0xA4BEEA44);
    P3(X, &D, A, B, C,  4, 11, 0x4BDECFA9);
    P3(X, &C, D, A, B,  7, 16, 0xF6BB4B60);
    P3(X, &B, C, D, A, 10, 23, 0xBEBFBC70);
    P3(X, &A, B, C, D, 13,  4, 0x289B7EC6);
    P3(X, &D, A, B, C,  0, 11, 0xEAA127FA);
    P3(X, &C, D, A, B,  3, 16, 0xD4EF3085);
    P3(X, &B, C, D, A,  6, 23, 0x04881D05);
    P3(X, &A, B, C, D,  9,  4, 0xD9D4D039);
    P3(X, &D, A, B, C, 12, 11, 0xE6DB99E5);
    P3(X, &C, D, A, B, 15, 16, 0x1FA27CF8);
    P3(X, &B, C, D, A,  2, 23, 0xC4AC5665);
    P4(X, &A, B, C, D,  0,  6, 0xF4292244);
    P4(X, &D, A, B, C,  7, 10, 0x432AFF97);
    P4(X, &C, D, A, B, 14, 15, 0xAB9423A7);
    P4(X, &B, C, D, A,  5, 21, 0xFC93A039);
    P4(X, &A, B, C, D, 12,  6, 0x655B59C3);
    P4(X, &D, A, B, C,  3, 10, 0x8F0CCC92);
    P4(X, &C, D, A, B, 10, 15, 0xFFEFF47D);
    P4(X, &B, C, D, A,  1, 21, 0x85845DD1);
    P4(X, &A, B, C, D,  8,  6, 0x6FA87E4F);
    P4(X, &D, A, B, C, 15, 10, 0xFE2CE6E0);
    P4(X, &C, D, A, B,  6, 15, 0xA3014314);
    P4(X, &B, C, D, A, 13, 21, 0x4E0811A1);
    P4(X, &A, B, C, D,  4,  6, 0xF7537E82);
    P4(X, &D, A, B, C, 11, 10, 0xBD3AF235);
    P4(X, &C, D, A, B,  2, 15, 0x2AD7D2BB);
    P4(X, &B, C, D, A,  9, 21, 0xEB86D391);
    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
}

void md5_update(md5_context *ctx, uint8_t *input, uint32_t length)
{
    uint32_t left, fill;
    if (length < 1) return;
    left = ctx->total[0] & 0x3F;
    fill = 64 - left;
    ctx->total[0] += length;
    ctx->total[0] &= 0xFFFFFFFF;
    if (ctx->total[0] < length)
        ctx->total[1]++;
    if (left && length >= fill) {
        memcpy((void *)(ctx->buffer + left),
               (void *) input, fill);
        md5_process(ctx, ctx->buffer);
        length -= fill;
        input  += fill;
        left = 0;
    }
    while (length >= 64) {
        md5_process(ctx, input);
        length -= 64;
        input  += 64;
    }
    if (length >= 1)
        memcpy((void *)(ctx->buffer + left),
               (void *) input, length);
}

void md5_finish(md5_context *ctx, uint8_t digest[16])
{
    static uint8_t md5_padding[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint32_t last, padn;
    uint32_t high, low;
    uint8_t msglen[8];
    high = (ctx->total[0] >> 29) | (ctx->total[1] <<  3);
    low  = (ctx->total[0] <<  3);
    PUT_UINT32(low,  msglen, 0);
    PUT_UINT32(high, msglen, 4);
    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);
    md5_update(ctx, md5_padding, padn);
    md5_update(ctx, msglen, 8);
    PUT_UINT32(ctx->state[0], digest,  0);
    PUT_UINT32(ctx->state[1], digest,  4);
    PUT_UINT32(ctx->state[2], digest,  8);
    PUT_UINT32(ctx->state[3], digest, 12);
}
