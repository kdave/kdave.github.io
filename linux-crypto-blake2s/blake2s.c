/*
   BLAKE2 reference source code package - reference C implementations

   Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.  You may use this under the
   terms of the CC0, the OpenSSL Licence, or the Apache Public License 2.0, at
   your option.  The terms of these licenses can be found at:

   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
   - OpenSSL license   : https://www.openssl.org/source/license.html
   - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0

   More information about the BLAKE2 hash function can be found at
   https://blake2.net.
*/

#ifdef __KERNEL__
#include <asm/unaligned.h>
#include <crypto/internal/hash.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define printk	printf
#define KERN_DEBUG

#endif

#include "blake2.h"
#include "blake2-impl.h"

static const u32 blake2s_IV[8] =
{
  0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
  0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static const u8 blake2s_sigma[10][16] =
{
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
  { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 } ,
  { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 } ,
  {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 } ,
  {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 } ,
  {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 } ,
  { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 } ,
  { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 } ,
  {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 } ,
  { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 } ,
};

static void blake2s_set_lastnode( blake2s_state *S )
{
  S->f[1] = (u32)-1;
}

/* Some helper functions, not necessarily useful */
static int blake2s_is_lastblock( const blake2s_state *S )
{
  return S->f[0] != 0;
}

static void blake2s_set_lastblock( blake2s_state *S )
{
  if( S->last_node ) blake2s_set_lastnode( S );

  S->f[0] = (u32)-1;
}

static void blake2s_increment_counter( blake2s_state *S, const u32 inc )
{
  S->t[0] += inc;
  S->t[1] += ( S->t[0] < inc );
}

static void blake2s_init0( blake2s_state *S )
{
  size_t i;
  memset( S, 0, sizeof( blake2s_state ) );

  for( i = 0; i < 8; ++i ) S->h[i] = blake2s_IV[i];
}

/* init2 xors IV with input parameter block */
int blake2s_init_param( blake2s_state *S, const blake2s_param *P )
{
  const unsigned char *p = ( const unsigned char * )( P );
  size_t i;

  blake2s_init0( S );

  /* IV XOR ParamBlock */
  for( i = 0; i < 8; ++i )
    S->h[i] ^= load32( &p[i * 4] );

  S->outlen = P->digest_length;
  return 0;
}


/* Sequential blake2s initialization */
int blake2s_init( blake2s_state *S, size_t outlen )
{
  blake2s_param P[1];

  /* Move interval verification here? */
  if ( ( !outlen ) || ( outlen > BLAKE2S_OUTBYTES ) ) return -1;

  P->digest_length = (u8)outlen;
  P->key_length    = 0;
  P->fanout        = 1;
  P->depth         = 1;
  store32( &P->leaf_length, 0 );
  store32( &P->node_offset, 0 );
  store16( &P->xof_length, 0 );
  P->node_depth    = 0;
  P->inner_length  = 0;
  /* memset(P->reserved, 0, sizeof(P->reserved) ); */
  memset( P->salt,     0, sizeof( P->salt ) );
  memset( P->personal, 0, sizeof( P->personal ) );
  return blake2s_init_param( S, P );
}

int blake2s_init_key( blake2s_state *S, size_t outlen, const void *key, size_t keylen )
{
  blake2s_param P[1];

  if ( ( !outlen ) || ( outlen > BLAKE2S_OUTBYTES ) ) return -1;

  if ( !key || !keylen || keylen > BLAKE2S_KEYBYTES ) return -1;

  P->digest_length = (u8)outlen;
  P->key_length    = (u8)keylen;
  P->fanout        = 1;
  P->depth         = 1;
  store32( &P->leaf_length, 0 );
  store32( &P->node_offset, 0 );
  store16( &P->xof_length, 0 );
  P->node_depth    = 0;
  P->inner_length  = 0;
  /* memset(P->reserved, 0, sizeof(P->reserved) ); */
  memset( P->salt,     0, sizeof( P->salt ) );
  memset( P->personal, 0, sizeof( P->personal ) );

  if( blake2s_init_param( S, P ) < 0 ) return -1;

  {
    u8 block[BLAKE2S_BLOCKBYTES];
    memset( block, 0, BLAKE2S_BLOCKBYTES );
    memcpy( block, key, keylen );
    blake2s_update( S, block, BLAKE2S_BLOCKBYTES );
    secure_zero_memory( block, BLAKE2S_BLOCKBYTES ); /* Burn the key from stack */
  }
  return 0;
}

#define G(r,i,a,b,c,d)                      \
  do {                                      \
    a = a + b + m[blake2s_sigma[r][2*i+0]]; \
    d = rotr32(d ^ a, 16);                  \
    c = c + d;                              \
    b = rotr32(b ^ c, 12);                  \
    a = a + b + m[blake2s_sigma[r][2*i+1]]; \
    d = rotr32(d ^ a, 8);                   \
    c = c + d;                              \
    b = rotr32(b ^ c, 7);                   \
  } while(0)

#define ROUND(r)                    \
  do {                              \
    G(r,0,v[ 0],v[ 4],v[ 8],v[12]); \
    G(r,1,v[ 1],v[ 5],v[ 9],v[13]); \
    G(r,2,v[ 2],v[ 6],v[10],v[14]); \
    G(r,3,v[ 3],v[ 7],v[11],v[15]); \
    G(r,4,v[ 0],v[ 5],v[10],v[15]); \
    G(r,5,v[ 1],v[ 6],v[11],v[12]); \
    G(r,6,v[ 2],v[ 7],v[ 8],v[13]); \
    G(r,7,v[ 3],v[ 4],v[ 9],v[14]); \
  } while(0)

static void blake2s_compress( blake2s_state *S, const u8 in[BLAKE2S_BLOCKBYTES] )
{
  u32 m[16];
  u32 v[16];
  size_t i;

  for( i = 0; i < 16; ++i ) {
    m[i] = load32( in + i * sizeof( m[i] ) );
  }

  for( i = 0; i < 8; ++i ) {
    v[i] = S->h[i];
  }

  v[ 8] = blake2s_IV[0];
  v[ 9] = blake2s_IV[1];
  v[10] = blake2s_IV[2];
  v[11] = blake2s_IV[3];
  v[12] = S->t[0] ^ blake2s_IV[4];
  v[13] = S->t[1] ^ blake2s_IV[5];
  v[14] = S->f[0] ^ blake2s_IV[6];
  v[15] = S->f[1] ^ blake2s_IV[7];

  ROUND( 0 );
  ROUND( 1 );
  ROUND( 2 );
  ROUND( 3 );
  ROUND( 4 );
  ROUND( 5 );
  ROUND( 6 );
  ROUND( 7 );
  ROUND( 8 );
  ROUND( 9 );

  for( i = 0; i < 8; ++i ) {
    S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];
  }
}

#undef G
#undef ROUND

int blake2s_update( blake2s_state *S, const void *pin, size_t inlen )
{
  const unsigned char * in = (const unsigned char *)pin;
  if( inlen > 0 )
  {
    size_t left = S->buflen;
    size_t fill = BLAKE2S_BLOCKBYTES - left;
    if( inlen > fill )
    {
      S->buflen = 0;
      memcpy( S->buf + left, in, fill ); /* Fill buffer */
      blake2s_increment_counter( S, BLAKE2S_BLOCKBYTES );
      blake2s_compress( S, S->buf ); /* Compress */
      in += fill; inlen -= fill;
      while(inlen > BLAKE2S_BLOCKBYTES) {
        blake2s_increment_counter(S, BLAKE2S_BLOCKBYTES);
        blake2s_compress( S, in );
        in += BLAKE2S_BLOCKBYTES;
        inlen -= BLAKE2S_BLOCKBYTES;
      }
    }
    memcpy( S->buf + S->buflen, in, inlen );
    S->buflen += inlen;
  }
  return 0;
}

int blake2s_final( blake2s_state *S, void *out, size_t outlen )
{
  u8 buffer[BLAKE2S_OUTBYTES] = {0};
  size_t i;

  if( out == NULL || outlen < S->outlen )
    return -1;

  if( blake2s_is_lastblock( S ) )
    return -1;

  blake2s_increment_counter( S, ( u32 )S->buflen );
  blake2s_set_lastblock( S );
  memset( S->buf + S->buflen, 0, BLAKE2S_BLOCKBYTES - S->buflen ); /* Padding */
  blake2s_compress( S, S->buf );

  for( i = 0; i < 8; ++i ) /* Output full hash to temp buffer */
    store32( buffer + sizeof( S->h[i] ) * i, S->h[i] );

  memcpy( out, buffer, outlen );
  secure_zero_memory(buffer, sizeof(buffer));
  return 0;
}

int blake2s( void *out, size_t outlen, const void *in, size_t inlen, const void *key, size_t keylen )
{
  blake2s_state S[1];

  /* Verify parameters */
  if ( NULL == in && inlen > 0 ) return -1;

  if ( NULL == out ) return -1;

  if ( NULL == key && keylen > 0) return -1;

  if( !outlen || outlen > BLAKE2S_OUTBYTES ) return -1;

  if( keylen > BLAKE2S_KEYBYTES ) return -1;

  if( keylen > 0 )
  {
    if( blake2s_init_key( S, outlen, key, keylen ) < 0 ) return -1;
  }
  else
  {
    if( blake2s_init( S, outlen ) < 0 ) return -1;
  }

  blake2s_update( S, ( const u8 * )in, inlen );
  blake2s_final( S, out, outlen );
  return 0;
}

/* crypto API glue code */

struct chksum_desc_ctx {
	blake2s_state S[1];
};

struct chksum_ctx {
	u8 key[BLAKE2S_KEYBYTES];
};

#ifdef __KERNEL__

static int chksum_init(struct shash_desc *desc)
{
	struct chksum_ctx *mctx = crypto_shash_ctx(desc->tfm);
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);
	int ret;

	printk(KERN_DEBUG "BLAKE2s: init\n");
	ret = blake2s_init_key(ctx->S, BLAKE2S_OUTBYTES, mctx->key, BLAKE2S_KEYBYTES);
	if (ret)
		return -EINVAL;

	return 0;
}

static int chksum_setkey(struct crypto_shash *tfm, const u8 *key,
			 unsigned int keylen)
{
	struct chksum_ctx *mctx = crypto_shash_ctx(tfm);

	printk(KERN_DEBUG "BLAKE2s: setkey: keylen=%d\n", keylen);
	if (keylen != BLAKE2S_KEYBYTES) {
		crypto_shash_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	memcpy(mctx->key, key, BLAKE2S_KEYBYTES);
	return 0;
}

static int chksum_update(struct shash_desc *desc, const u8 *data,
			 unsigned int length)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);
	int ret;

	printk(KERN_DEBUG "BLAKE2s: update: len=%d\n", length);
	ret = blake2s_update(ctx->S, data, length);
	if (ret)
		return -EINVAL;
	return 0;
}

static int chksum_final(struct shash_desc *desc, u8 *out)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);
	int ret;

	printk(KERN_DEBUG "BLAKE2s: final\n");
	ret = blake2s_final(ctx->S, out, BLAKE2S_OUTBYTES);
	if (ret)
		return -EINVAL;
	return 0;
}

static int chksum_finup(struct shash_desc *desc, const u8 *data,
			unsigned int len, u8 *out)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);
	int ret;

	printk(KERN_DEBUG "BLAKE2s: finup: len=%d\n", len);
	ret = blake2s_update(ctx->S, data, len);
	if (ret)
		return -EINVAL;
	ret = blake2s_final(ctx->S, out, BLAKE2S_OUTBYTES);
	if (ret)
		return -EINVAL;

	return 0;
}

static int blake2s_cra_init(struct crypto_tfm *tfm)
{
	struct chksum_ctx *mctx = crypto_tfm_ctx(tfm);
	int i;

	printk(KERN_DEBUG "BLAKE2s: cra_init");
	for (i = 0; i <BLAKE2S_KEYBYTES; i++)
		mctx->key[i] = (u8)i;

	return 0;
}

static struct shash_alg alg = {
	.digestsize	=	BLAKE2S_OUTBYTES,
	.setkey		=	chksum_setkey,
	.init		=	chksum_init,
	.update		=	chksum_update,
	.final		=	chksum_final,
	.finup		=	chksum_finup,
	.descsize	=	sizeof(struct chksum_desc_ctx),
	.base		=	{
		.cra_name		=	"blake2s",
		.cra_driver_name	=	"blake2s-generic",
		.cra_priority		=	100,
		.cra_flags		=	CRYPTO_ALG_OPTIONAL_KEY,
		.cra_blocksize		=	1,
		.cra_ctxsize		=	sizeof(struct chksum_ctx),
		.cra_module		=	THIS_MODULE,
		.cra_init		=	blake2s_cra_init,
	}
};

static int __init blake2s_mod_init(void)
{
	printk(KERN_DEBUG "BLAKE2s: blake2s loaded\n");
	return crypto_register_shash(&alg);
}

static void __exit blake2s_mod_fini(void)
{
	crypto_unregister_shash(&alg);
	printk(KERN_DEBUG "BLAKE2s: blake2s unloaded\n");
}

subsys_initcall(blake2s_mod_init);
module_exit(blake2s_mod_fini);

MODULE_AUTHOR("kdave");
MODULE_DESCRIPTION("BLAKE2s reference implementation");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CRYPTO("blake2s");
MODULE_ALIAS_CRYPTO("blake2s-generic");

#else

int main() {
	char bigarray[4096];
	struct chksum_ctx mctx[1];
	struct chksum_desc_ctx ctx[1];
	u8 result[BLAKE2S_OUTBYTES];;
	int i;
	int length;
	char *data;
	int ret;
	int defaultkey = 1;

	if (defaultkey) {
		for (i = 0; i < BLAKE2S_KEYBYTES; i++)
			mctx->key[i] = (u8)i;
		printf("KEY___: ");
		for (i = 0; i < BLAKE2S_KEYBYTES; i++)
			printf("%02hhx", (u8)mctx->key[i]);
		printf("\n");
		/* Default key:
		{
		    "hash": "blake2s",
		    "in": "",
		    "key": "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
		    "out": "48a8997da407876b3d79c0d92325ad3b89cbb754d86ab71aee047ad345fd2c49"
		},
		 */
		ret = blake2s_init_key(ctx->S, BLAKE2S_OUTBYTES, mctx->key, BLAKE2S_KEYBYTES);
		if (ret) {
			printf("ERROR: init %d\n", ret);
			return 1;
		}
	} else {
		printf("KEY___: empty\n");
		/* No key:
		 * {
			"hash": "blake2s",
			"in": "",
			"key": "",
			"out": "69217a3079908094e11121d042354a7c1f55b6482ca1a51e1b250dfd1ed0eef9"
		},
		 */
		ret = blake2s_init(ctx->S, BLAKE2S_OUTBYTES);
		if (ret) {
			printf("ERROR: init %d\n", ret);
			return 1;
		}
	}

	data = bigarray;
	length = 0;
	ret = blake2s_update(ctx->S, data, length);
	if (ret) {
		printf("ERROR: update %d\n", ret);
		return 1;
	}

	ret = blake2s_final(ctx->S, result, BLAKE2S_OUTBYTES);
	if (ret) {
		printf("ERROR: final %d\n", ret);
		return 1;
	}

	printf("OUTPUT: ");
	for (i = 0; i < BLAKE2S_OUTBYTES; i++)
		printf("%02hhx", (u8)result[i]);
	printf("\n");

	return 0;
}

#endif		/* __KERNEL__ */
