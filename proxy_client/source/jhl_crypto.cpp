

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "jhl_crypto.h"


#define __le32	u_int32_t

typedef u_int8_t 	_u8;
typedef u_int32_t	_u32;
typedef u_int64_t 	_u64;

typedef u_int8_t 	__u8;
typedef u_int32_t	__u32;
typedef u_int64_t 	__u64;

static union
{
  int i;
  char c[sizeof(int)];
}CPU_BIG_LITTLE={1};



#define swab32(x) \
({ \
	__u32 __x = (x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})

#if 0 //JHL_SYS_TYPE == SYS_TYPE_CM || JHL_SYS_TYPE == SYS_TYPE_RA
//#define cpu_to_le32(x) swab32(x)
//#define le32_to_cpu(x) swab32(x)
static inline __u32 cpu_to_le32(__u32 x)
{
	if(CPU_BIG_LITTLE.c[0] == 0)
		return swab32(x);
	else 
		return (__u32)(x);
}
#define le32_to_cpu cpu_to_le32

#else
#define cpu_to_le32(x) (__u32)(x)
#define le32_to_cpu(x) (__u32)(x)
#endif


static inline _u8 byte(const _u32 x, const unsigned n)
{
	return x >> (n << 3);
}

#define E_KEY ctx->E
#define D_KEY ctx->D

static _u8 pow_tab[256];
static _u8 log_tab[256];
static _u8 sbx_tab[256];
static _u8 isb_tab[256];
static _u32 rco_tab[10];
static _u32 ft_tab[4][256];
static _u32 it_tab[4][256];

static _u32 fl_tab[4][256];
static _u32 il_tab[4][256];

static inline _u8 f_mult (_u8 a, _u8 b)
{
	_u8 aa = log_tab[a], cc = aa + log_tab[b];

	return pow_tab[cc + (cc < aa ? 1 : 0)];
}

static inline _u32 rol32(_u32 word, unsigned int shift)
{
	return (word << shift) | (word >> (32 - shift));
}

static inline _u32 ror32(_u32 word, unsigned int shift)
{
	return (word >> shift) | (word << (32 - shift));
}

#define ff_mult(a,b)    (a && b ? f_mult(a, b) : 0)

#define f_rn(bo, bi, n, k)					\
    bo[n] =  ft_tab[0][byte(bi[n],0)] ^				\
             ft_tab[1][byte(bi[(n + 1) & 3],1)] ^		\
             ft_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             ft_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rn(bo, bi, n, k)					\
    bo[n] =  it_tab[0][byte(bi[n],0)] ^				\
             it_tab[1][byte(bi[(n + 3) & 3],1)] ^		\
             it_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             it_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

#define ls_box(x)				\
    ( fl_tab[0][byte(x, 0)] ^			\
      fl_tab[1][byte(x, 1)] ^			\
      fl_tab[2][byte(x, 2)] ^			\
      fl_tab[3][byte(x, 3)] )

#define f_rl(bo, bi, n, k)					\
    bo[n] =  fl_tab[0][byte(bi[n],0)] ^				\
             fl_tab[1][byte(bi[(n + 1) & 3],1)] ^		\
             fl_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             fl_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rl(bo, bi, n, k)					\
    bo[n] =  il_tab[0][byte(bi[n],0)] ^				\
             il_tab[1][byte(bi[(n + 3) & 3],1)] ^		\
             il_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             il_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

static void gen_tabs (void)
{
	_u32 i, t;
	_u8 p, q;

	/* log and power tables for GF(2**8) finite field with
	   0x011b as modular polynomial - the simplest primitive
	   root is 0x03, used here to generate the tables */

	for (i = 0, p = 1; i < 256; ++i) {
		pow_tab[i] = (_u8) p;
		log_tab[p] = (_u8) i;

		p ^= (p << 1) ^ (p & 0x80 ? 0x01b : 0);
	}

	log_tab[1] = 0;

	for (i = 0, p = 1; i < 10; ++i) {
		rco_tab[i] = p;

		p = (p << 1) ^ (p & 0x80 ? 0x01b : 0);
	}

	for (i = 0; i < 256; ++i) {
		p = (i ? pow_tab[255 - log_tab[i]] : 0);
		q = ((p >> 7) | (p << 1)) ^ ((p >> 6) | (p << 2));
		p ^= 0x63 ^ q ^ ((q >> 6) | (q << 2));
		sbx_tab[i] = p;
		isb_tab[p] = (_u8) i;
	}

	for (i = 0; i < 256; ++i) {
		p = sbx_tab[i];

		t = p;
		fl_tab[0][i] = t;
		fl_tab[1][i] = rol32(t, 8);
		fl_tab[2][i] = rol32(t, 16);
		fl_tab[3][i] = rol32(t, 24);

		t = ((_u32) ff_mult (2, p)) |
		    ((_u32) p << 8) |
		    ((_u32) p << 16) | ((_u32) ff_mult (3, p) << 24);

		ft_tab[0][i] = t;
		ft_tab[1][i] = rol32(t, 8);
		ft_tab[2][i] = rol32(t, 16);
		ft_tab[3][i] = rol32(t, 24);

		p = isb_tab[i];

		t = p;
		il_tab[0][i] = t;
		il_tab[1][i] = rol32(t, 8);
		il_tab[2][i] = rol32(t, 16);
		il_tab[3][i] = rol32(t, 24);

		t = ((_u32) ff_mult (14, p)) |
		    ((_u32) ff_mult (9, p) << 8) |
		    ((_u32) ff_mult (13, p) << 16) |
		    ((_u32) ff_mult (11, p) << 24);

		it_tab[0][i] = t;
		it_tab[1][i] = rol32(t, 8);
		it_tab[2][i] = rol32(t, 16);
		it_tab[3][i] = rol32(t, 24);
	}
}

#define star_x(x) (((x) & 0x7f7f7f7f) << 1) ^ ((((x) & 0x80808080) >> 7) * 0x1b)

#define imix_col(y,x)       \
    u   = star_x(x);        \
    v   = star_x(u);        \
    w   = star_x(v);        \
    t   = w ^ (x);          \
   (y)  = u ^ v ^ w;        \
   (y) ^= ror32(u ^ t,  8) ^ \
          ror32(v ^ t, 16) ^ \
          ror32(t,24)

/* initialise the key schedule from the user supplied key */

#define loop4(i)                                    \
{   t = ror32(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= E_KEY[4 * i];     E_KEY[4 * i + 4] = t;    \
    t ^= E_KEY[4 * i + 1]; E_KEY[4 * i + 5] = t;    \
    t ^= E_KEY[4 * i + 2]; E_KEY[4 * i + 6] = t;    \
    t ^= E_KEY[4 * i + 3]; E_KEY[4 * i + 7] = t;    \
}

#define loop6(i)                                    \
{   t = ror32(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= E_KEY[6 * i];     E_KEY[6 * i + 6] = t;    \
    t ^= E_KEY[6 * i + 1]; E_KEY[6 * i + 7] = t;    \
    t ^= E_KEY[6 * i + 2]; E_KEY[6 * i + 8] = t;    \
    t ^= E_KEY[6 * i + 3]; E_KEY[6 * i + 9] = t;    \
    t ^= E_KEY[6 * i + 4]; E_KEY[6 * i + 10] = t;   \
    t ^= E_KEY[6 * i + 5]; E_KEY[6 * i + 11] = t;   \
}

#define loop8(i)                                    \
{   t = ror32(t,  8); ; t = ls_box(t) ^ rco_tab[i];  \
    t ^= E_KEY[8 * i];     E_KEY[8 * i + 8] = t;    \
    t ^= E_KEY[8 * i + 1]; E_KEY[8 * i + 9] = t;    \
    t ^= E_KEY[8 * i + 2]; E_KEY[8 * i + 10] = t;   \
    t ^= E_KEY[8 * i + 3]; E_KEY[8 * i + 11] = t;   \
    t  = E_KEY[8 * i + 4] ^ ls_box(t);    \
    E_KEY[8 * i + 12] = t;                \
    t ^= E_KEY[8 * i + 5]; E_KEY[8 * i + 13] = t;   \
    t ^= E_KEY[8 * i + 6]; E_KEY[8 * i + 14] = t;   \
    t ^= E_KEY[8 * i + 7]; E_KEY[8 * i + 15] = t;   \
}

int tabs_inited = 0;
int aes_set_key(struct aes_ctx *ctx_arg, const unsigned char *in_key, unsigned int key_len)
{
	struct aes_ctx *ctx = ctx_arg;
	const __le32 *key = (const __le32 *)in_key;
	_u32 i, t, u, v, w;

	if (key_len != 16 && key_len != 24 && key_len != 32) {
		return -1;
	}

	if(!tabs_inited)
	{
		gen_tabs();
		tabs_inited = 1;
	}
	
	ctx->key_length = key_len;

	E_KEY[0] = le32_to_cpu(key[0]);
	E_KEY[1] = le32_to_cpu(key[1]);
	E_KEY[2] = le32_to_cpu(key[2]);
	E_KEY[3] = le32_to_cpu(key[3]);

	switch (key_len) {
	case 16:
		t = E_KEY[3];
		for (i = 0; i < 10; ++i)
			loop4 (i);
		break;

	case 24:
		E_KEY[4] = le32_to_cpu(key[4]);
		t = E_KEY[5] = le32_to_cpu(key[5]);
		for (i = 0; i < 8; ++i)
			loop6 (i);
		break;

	case 32:
		E_KEY[4] = le32_to_cpu(key[4]);
		E_KEY[5] = le32_to_cpu(key[5]);
		E_KEY[6] = le32_to_cpu(key[6]);
		t = E_KEY[7] = le32_to_cpu(key[7]);
		for (i = 0; i < 7; ++i)
			loop8 (i);
		break;
	}

	D_KEY[0] = E_KEY[0];
	D_KEY[1] = E_KEY[1];
	D_KEY[2] = E_KEY[2];
	D_KEY[3] = E_KEY[3];

	for (i = 4; i < key_len + 24; ++i) {
		imix_col (D_KEY[i], E_KEY[i]);
	}

	return 0;
}

/* encrypt a block of text */

#define f_nround(bo, bi, k) \
    f_rn(bo, bi, 0, k);     \
    f_rn(bo, bi, 1, k);     \
    f_rn(bo, bi, 2, k);     \
    f_rn(bo, bi, 3, k);     \
    k += 4

#define f_lround(bo, bi, k) \
    f_rl(bo, bi, 0, k);     \
    f_rl(bo, bi, 1, k);     \
    f_rl(bo, bi, 2, k);     \
    f_rl(bo, bi, 3, k)

void aes_encrypt(struct aes_ctx *ctx_arg, unsigned char *out, const unsigned char *in)
{
	const struct aes_ctx *ctx = ctx_arg;
	const __le32 *src = (const __le32 *)in;
	__le32 *dst = (__le32 *)out;
	_u32 b0[4], b1[4];
	const _u32 *kp = E_KEY + 4;

	b0[0] = le32_to_cpu(src[0]) ^ E_KEY[0];
	b0[1] = le32_to_cpu(src[1]) ^ E_KEY[1];
	b0[2] = le32_to_cpu(src[2]) ^ E_KEY[2];
	b0[3] = le32_to_cpu(src[3]) ^ E_KEY[3];

	if (ctx->key_length > 24) {
		f_nround (b1, b0, kp);
		f_nround (b0, b1, kp);
	}

	if (ctx->key_length > 16) {
		f_nround (b1, b0, kp);
		f_nround (b0, b1, kp);
	}

	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_lround (b0, b1, kp);

	dst[0] = cpu_to_le32(b0[0]);
	dst[1] = cpu_to_le32(b0[1]);
	dst[2] = cpu_to_le32(b0[2]);
	dst[3] = cpu_to_le32(b0[3]);
}

/* decrypt a block of text */

#define i_nround(bo, bi, k) \
    i_rn(bo, bi, 0, k);     \
    i_rn(bo, bi, 1, k);     \
    i_rn(bo, bi, 2, k);     \
    i_rn(bo, bi, 3, k);     \
    k -= 4

#define i_lround(bo, bi, k) \
    i_rl(bo, bi, 0, k);     \
    i_rl(bo, bi, 1, k);     \
    i_rl(bo, bi, 2, k);     \
    i_rl(bo, bi, 3, k)

void aes_decrypt(struct aes_ctx *ctx_arg, unsigned char *out, const unsigned char *in)
{
	const struct aes_ctx *ctx = ctx_arg;
	const __le32 *src = (const __le32 *)in;
	__le32 *dst = (__le32 *)out;
	_u32 b0[4], b1[4];
	const int key_len = ctx->key_length;
	const _u32 *kp = D_KEY + key_len + 20;

	b0[0] = le32_to_cpu(src[0]) ^ E_KEY[key_len + 24];
	b0[1] = le32_to_cpu(src[1]) ^ E_KEY[key_len + 25];
	b0[2] = le32_to_cpu(src[2]) ^ E_KEY[key_len + 26];
	b0[3] = le32_to_cpu(src[3]) ^ E_KEY[key_len + 27];

	if (key_len > 24) {
		i_nround (b1, b0, kp);
		i_nround (b0, b1, kp);
	}

	if (key_len > 16) {
		i_nround (b1, b0, kp);
		i_nround (b0, b1, kp);
	}

	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_lround (b0, b1, kp);

	dst[0] = cpu_to_le32(b0[0]);
	dst[1] = cpu_to_le32(b0[1]);
	dst[2] = cpu_to_le32(b0[2]);
	dst[3] = cpu_to_le32(b0[3]);
}

/********************
* type:   0: encrypt; 1: decrypt
* out buf len >= (in_len + 15) / 16 * 16 
*********************/
int aes_aes(struct aes_ctx *ctx, unsigned char *in, int in_len, unsigned char *out, int type)
{
	int len;
	unsigned char buf[AES_BLOCK_SIZE];
	int out_len =0;
	
	while(in_len > 0)
	{
		memset(buf, 0, sizeof(buf));
		len = (in_len > AES_BLOCK_SIZE) ? AES_BLOCK_SIZE : in_len;
		memcpy(buf, in, len);

		if(type)
			aes_decrypt(ctx, out, buf);
		else
			aes_encrypt(ctx, out, buf);
		
		in_len -= len;
		out += AES_BLOCK_SIZE;
		in += len;
		out_len += AES_BLOCK_SIZE;
	}

	return out_len;
}


