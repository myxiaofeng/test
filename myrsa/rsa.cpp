#include <stdio.h>
#include <openssl/evp.h>
//#include <crypto/evp/evp_locl.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string.h>
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")


#define RSA_KEY_LENGTH 1024
static const char rnd_seed[] = "string to make the random number generator initialized";
 
#define PRIVATE_KEY_FILE "/tmp/avit.data.tmp1"
#define PUBLIC_KEY_FILE  "/tmp/avit.data.tmp2"
 
#define RSA_PRIKEY_PSW "123"
 




int generate_key_files(const char *pub_keyfile, const char *pri_keyfile, 
					   const unsigned char *passwd, int passwd_len)
{
	RSA *rsa = RSA_new();
	int ret = 0;
	BIGNUM *bne = BN_new();
	RAND_seed(rnd_seed, sizeof(rnd_seed));
	ret = BN_set_word(bne,RSA_F4);
	ret = RSA_generate_key_ex(rsa,RSA_KEY_LENGTH,bne,NULL);
	if(ret != 1)
	{
		printf("RSA_generate_key error!\n");
		return -1;
	}
 
	// ��ʼ���ɹ�Կ�ļ�
	BIO *bp = BIO_new(BIO_s_file());
	if(NULL == bp)
	{
		printf("generate_key bio file new error!\n");
		return -1;
	}
 
	if(BIO_write_filename(bp, (void *)pub_keyfile) <= 0)
	{
		printf("BIO_write_filename error!\n");
		return -1;
	}
 
	if(PEM_write_bio_RSAPublicKey(bp, rsa) != 1)
	{
		printf("PEM_write_bio_RSAPublicKey error!\n");
		return -1;
	}
	
	// ��Կ�ļ����ɳɹ����ͷ���Դ
	printf("Create public key ok!\n");
	BIO_free_all(bp);
 
	// ����˽Կ�ļ�
	bp = BIO_new_file(pri_keyfile, "w+");
        if(NULL == bp)
	{
		printf("generate_key bio file new error2!\n");
		return -1;
	}
 
	if(PEM_write_bio_RSAPrivateKey(bp, rsa,
		EVP_des_ede3_ofb(), (unsigned char *)passwd, 
		passwd_len, NULL, NULL) != 1)
	{
		printf("PEM_write_bio_RSAPublicKey error!\n");
		return -1;
	}
 
	// �ͷ���Դ
	printf("Create private key ok!\n");
	BIO_free_all(bp);
	RSA_free(rsa);
	BN_free(bne);
	return 0;
}

	
// �򿪹�Կ�ļ�������EVP_PKEY�ṹ��ָ��
EVP_PKEY* open_public_key(const char *keyfile)
{
	EVP_PKEY* key = NULL;
	RSA *rsa = NULL;
 
	OpenSSL_add_all_algorithms();
	BIO *bp = BIO_new(BIO_s_file());;
	BIO_read_filename(bp, keyfile);
	if(NULL == bp)
	{
		printf("open_public_key bio file new error!\n");
		return NULL;
	}
 
	rsa = PEM_read_bio_RSAPublicKey(bp, NULL, NULL, NULL);
	if(rsa == NULL)
	{
		printf("open_public_key failed to PEM_read_bio_RSAPublicKey!\n");
		BIO_free(bp);
		RSA_free(rsa);
 
		return NULL;
	}
 
	printf("open_public_key success to PEM_read_bio_RSAPublicKey!\n");
	key = EVP_PKEY_new();
	if(NULL == key)
	{
		printf("open_public_key EVP_PKEY_new failed\n");
		RSA_free(rsa);
 
		return NULL;
	}
 
	EVP_PKEY_assign_RSA(key, rsa);
	return key;
}


// ��˽Կ�ļ�������EVP_PKEY�ṹ��ָ��
EVP_PKEY* open_private_key(const char *keyfile, const unsigned char *passwd)
{
	EVP_PKEY* key = NULL;
	RSA *rsa = RSA_new();
	OpenSSL_add_all_algorithms();
	BIO *bp = NULL;
	bp = BIO_new_file(keyfile, "rb"); 
	if(NULL == bp)
	{
		printf("open_private_key bio file new error!\n");
 
		return NULL;
	}
 
	rsa = PEM_read_bio_RSAPrivateKey(bp, &rsa, NULL, (void *)passwd);
	if(rsa == NULL)
	{
		printf("open_private_key failed to PEM_read_bio_RSAPrivateKey!\n");
		BIO_free(bp);
		RSA_free(rsa);
 
		return NULL;
	}
 
	printf("open_private_key success to PEM_read_bio_RSAPrivateKey!\n");
	key = EVP_PKEY_new();
	if(NULL == key)
	{
		printf("open_private_key EVP_PKEY_new failed\n");
		RSA_free(rsa);
 
		return NULL;
	}
 
	EVP_PKEY_assign_RSA(key, rsa);
	return key;
}


// ʹ����Կ���ܣ����ַ�װ��ʽֻ���ù�Կ���ܣ�˽Կ���ܣ�����key�����ǹ�Կ
int rsa_key_encrypt(EVP_PKEY *key, const unsigned char *orig_data, size_t orig_data_len, 
					unsigned char *enc_data, size_t &enc_data_len)
{
	EVP_PKEY_CTX *ctx = NULL;
	OpenSSL_add_all_ciphers();
 
	ctx = EVP_PKEY_CTX_new(key, NULL);
	if(NULL == ctx)
	{
		printf("ras_pubkey_encryptfailed to open ctx.\n");
		EVP_PKEY_free(key);
		return -1;
	}
 
	if(EVP_PKEY_encrypt_init(ctx) <= 0)
	{
		printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt_init.\n");
		EVP_PKEY_free(key);
		return -1;
	}
 
	if(EVP_PKEY_encrypt(ctx,
		enc_data,
		&enc_data_len,
		orig_data,
		orig_data_len) <= 0)
	{
		printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt.\n");
		EVP_PKEY_CTX_free(ctx);
		EVP_PKEY_free(key);
 
		return -1;
	}
 
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(key);
 
	return 0;
}

// ʹ����Կ���ܣ����ַ�װ��ʽֻ���ù�Կ���ܣ�˽Կ���ܣ�����key������˽Կ
int rsa_key_decrypt(EVP_PKEY *key, const unsigned char *enc_data, size_t enc_data_len, 
					unsigned char *orig_data, size_t &orig_data_len, const unsigned char *passwd)
{
	EVP_PKEY_CTX *ctx = NULL;
	OpenSSL_add_all_ciphers();
 
	ctx = EVP_PKEY_CTX_new(key, NULL);
	if(NULL == ctx)
	{
		printf("ras_prikey_decryptfailed to open ctx.\n");
		EVP_PKEY_free(key);
		return -1;
	}
 
	if(EVP_PKEY_decrypt_init(ctx) <= 0)
	{
		printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt_init.\n");
		EVP_PKEY_free(key);
		return -1;
	}
 
	if(EVP_PKEY_decrypt(ctx,
		orig_data,
		&orig_data_len,
		enc_data,
		enc_data_len) <= 0)
	{
		printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt.\n");
		EVP_PKEY_CTX_free(ctx);
		EVP_PKEY_free(key);
 
		return -1;
	}
 
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(key);
	return 0;
}

int main(int argc, char **argv)
{
	char origin_text[] = "hello world!";
	char enc_text[512] = "";
	char dec_text[512] = "";
	size_t enc_len = 512;
	size_t dec_len = 512;
 
	// ���ɹ�Կ��˽Կ�ļ�
	generate_key_files(PUBLIC_KEY_FILE, PRIVATE_KEY_FILE, (const unsigned char *)RSA_PRIKEY_PSW, strlen(RSA_PRIKEY_PSW));
 
	EVP_PKEY *pub_key = open_public_key(PUBLIC_KEY_FILE);
	EVP_PKEY *pri_key = open_private_key(PRIVATE_KEY_FILE, (const unsigned char *)RSA_PRIKEY_PSW);
	
	rsa_key_encrypt(pub_key, (const unsigned char *)&origin_text, sizeof(origin_text), (unsigned char *)enc_text, enc_len);
	rsa_key_decrypt(pri_key, (const unsigned char *)enc_text, enc_len, 
		(unsigned char *)dec_text, dec_len, (const unsigned char *)RSA_PRIKEY_PSW);
 
	return 0;
}
