
#ifndef __JHL_CRYPTO_H__
#define __JHL_CRYPTO_H__

/**************************  AES ***********************************/
#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32
#define AES_BLOCK_SIZE		16

struct aes_ctx{
	int key_length;
	u_int32_t E[60];
	u_int32_t D[60];
};

int aes_set_key(struct aes_ctx *ctx_arg, const unsigned char *in_key, unsigned int key_len);
int aes_aes(struct aes_ctx *ctx, unsigned char *in, int in_len, unsigned char *out, int type);

/*************************** AES end *******************************/

/*************************** ECC ***********************************/
#define ECC_BN_ENC_LEN	23
#define ECC_BN_DEC_LEN	24
#define ECC_ENC_ALIGN	(ECC_BN_ENC_LEN*2)
#define ECC_DEC_ALIGN	(ECC_BN_DEC_LEN*2)

int ecc_enc(unsigned char *p_x, unsigned char *p_y, unsigned char *key,
				unsigned char *in_buf, int in_len, unsigned char *out_buf, int out_len);
int ecc_dec(unsigned char *R_x, unsigned char *R_y, unsigned char *Private,
						unsigned char *in_buf, int in_len, unsigned char *out_buf, int out_len);

/**************************** ECC end *********************************/

#endif

