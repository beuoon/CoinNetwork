#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <sys/time.h>
#include <limits.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>


using namespace std;

struct url_data {
size_t size;
char *data;
};

#define _API_HOST "https://api.bithumb.com"
#define _API_KEY "api connect key"
#define _API_SECRET "api secret key"

/* {{{ base64 tables */
static const char base64_table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

static const char base64_pad = '=';

static const short base64_reverse_table[256] = {
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
	-2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
	-2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};
/* }}} */

unsigned char *base64_encode(const unsigned char *str, int length, int *ret_length) /* {{{ */
{
	const unsigned char *current = str;
	unsigned char *p;
	unsigned char *result;

	if (length < 0) {
		if (ret_length != NULL) {
			*ret_length = 0;
		}
		return NULL;
	}
	
	//result = (unsigned char *) safe_emalloc((length + 2) / 3, 4 * sizeof(char), 1);
	result = (unsigned char *) malloc(((length + 2) / 3) * (4 * sizeof(char)) + 1);//**;
	p = result;

	while (length > 2) { /* keep going until we have less than 24 bits */
		*p++ = base64_table[current[0] >> 2];
		*p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		*p++ = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		*p++ = base64_table[current[2] & 0x3f];

		current += 3;
		length -= 3; /* we just handle 3 octets of data */
	}

	/* now deal with the tail end of things */
	if (length != 0) {
		*p++ = base64_table[current[0] >> 2];
		if (length > 1) {
			*p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			*p++ = base64_table[(current[1] & 0x0f) << 2];
			*p++ = base64_pad;
		} else {
			*p++ = base64_table[(current[0] & 0x03) << 4];
			*p++ = base64_pad;
			*p++ = base64_pad;
		}
	}

	if (ret_length != NULL) {
		*ret_length = (int)(p - result);
	}

	*p = '\0';

	return result;
}

char chr(int ascii)
{
   return((char)ascii);
}

int ord(char chr)
{
   return((int)chr);
}


char *url_encode(char *str) 
{
	char *encstr, buf[2+1];
	unsigned char c;
	int i, j;

	if(str == NULL) return NULL;
	if((encstr = (char *)malloc((strlen(str) * 3) + 1)) == NULL) 
			return NULL;

	for(i = j = 0; str[i]; i++) 
	{
		c = (unsigned char)str[i];
		if((c >= '0') && (c <= '9')) encstr[j++] = c;
		else if((c >= 'A') && (c <= 'Z')) encstr[j++] = c;
		else if((c >= 'a') && (c <= 'z')) encstr[j++] = c;
		else if((c == '@') || (c == '.') || (c == '=') || (c == '\\')
				|| (c == '-') || (c == '_') || (c == ':') || (c == '&') ) 
			encstr[j++] = c;
		else 
		{
			sprintf(buf, "%02X", c);
			encstr[j++] = '%';
			encstr[j++] = buf[0];
			encstr[j++] = buf[1];
		}
	}
	encstr[j] = '\0';

	return encstr;
}

void hash_bin2hex(char *out, const unsigned char *in, int in_len)
{
	static const char hexits[17] = "0123456789abcdef";
	int i;

	for(i = 0; i < in_len; i++) {
		out[i * 2]       = hexits[in[i] >> 4];
		out[(i * 2) + 1] = hexits[in[i] &  0x0F];
	}
}

void hash_sha512(char *hash, const unsigned char *in, const char *secret)
{
	HMAC_CTX *ctx = HMAC_CTX_new();
	
	unsigned int len = 128;
	char *out;
	out = (char *)malloc( sizeof(char) * len );

	// using sha512
	HMAC_Init_ex( ctx, secret, strlen(secret), EVP_sha512(), NULL );
	HMAC_Update( ctx, in, strlen((char*)in) );
	HMAC_Final( ctx, (unsigned char*)out, &len );
	HMAC_CTX_free(ctx);

	hash_bin2hex(hash,(unsigned char*)out, 128/2);
}

size_t write_data( void *ptr, size_t size, size_t nmemb, struct url_data *data )
{
    size_t index = data->size;
    size_t n = ( size * nmemb );
    char *tmp;

    data->size += ( size * nmemb );

#ifdef DEBUG
	fprintf( stderr, "data at %p size=%ld nmemb=%ld\n", ptr,
		size, nmemb );
#endif

	tmp = (char *)realloc( data->data, data->size + 1); // + 1 for \n

	if ( tmp )
	{
		data->data = tmp;
	}
	else
	{
		if ( data->data )
		{
			free( data->data );
		}
		fprintf( stderr, "Failed to allocate memory.\n" );
		return 0;
	}

	memcpy( ( data->data + index ), ptr, n );
	data->data[ data->size ] = '\0';

	return size * nmemb;
}

const char *api_request(const char *endpoint, const char *post_data)
{

	char url[strlen(_API_HOST) + strlen(endpoint) + 1];
	sprintf(url, "%s%s", _API_HOST, endpoint);
	
	//printf("url : [%s]\n", url);

	struct timeval time;
	gettimeofday( &time, NULL );
	long mt = ( (unsigned long long)time.tv_sec * 1000000 ) + time.tv_usec;
	char mtStr[ 128 ];
	
	sprintf( mtStr, "%lu", mt );

	char strPost[strlen("encpoint=") + strlen(endpoint) + strlen("&") + strlen(post_data)];
	char strData[strlen(endpoint) + strlen("encpoint=") + strlen(endpoint) + strlen("&") + strlen(post_data) + 2 + 1];
	char *strEncode;
	strcpy(strPost, "endpoint=");
	strcat(strPost, endpoint);
	strcat(strPost, "&");
	strcat(strPost, post_data);
	
	strEncode = url_encode(strPost);
	void *pData = strEncode;

	char parChar = chr(1);
	sprintf(strData, "%s%c%s%c%s", endpoint, parChar, strEncode, parChar, mtStr);

	//printf("strData : %s \n", strData);

	char *hash_str;
	unsigned int len = 128;

	hash_str = (char *)malloc( sizeof(char) * len );
	
	hash_sha512(hash_str, (unsigned char*)&strData, (char*)&_API_SECRET);

	//printf("%s \n", hash_str);

	unsigned char *base64_str;
	int ret_length;
	base64_str = base64_encode((unsigned char*)hash_str, strlen(hash_str), &ret_length);
	
	//printf("%s\n", base64_str);

	
	CURL *curl;
	
	struct url_data data;
	data.size = 0;
	data.data = (char*)malloc( 4096 );
	if ( NULL == data.data )
	{
		fprintf( stderr, "failed to allocate memory.\n" );
		return NULL;
	}

	data.data[0] = '\0';

	CURLcode res;
	
	const char *header1 = "Api-Key: ";
	size_t PrefixR = strlen(header1);
	char keyBuffer[PrefixR + 2 * strlen(_API_KEY) + 1];
	strcpy( keyBuffer, header1);
	strcat( keyBuffer, _API_KEY);

	//printf( "%s\n", keyBuffer );

	const char *header2 = "Api-Sign:";
	size_t PrefixL = strlen(header2);
	char resBuffer[PrefixL + 2 * strlen((char*)base64_str) + 1];
	strcpy( resBuffer, header2);
	strcat( resBuffer, (char*)base64_str);

	//printf( "%s\n", resBuffer );

	const char *header3 = "Api-Nonce:";
	size_t PrefixN = strlen(header3);
	char nonceBuffer[PrefixN + 2 * strlen(mtStr) + 1];
	strcpy( nonceBuffer, header3);
	strcat( nonceBuffer, mtStr);

	//printf( "%s\n", nonceBuffer );

	char clientTypeBuffer[strlen("api-client-type:1") + 1];
	strcpy( clientTypeBuffer, "api-client-type:1");

	const char *headerArray[4];
	headerArray[0] = keyBuffer;
	headerArray[1] = resBuffer;
	headerArray[2] = nonceBuffer;
	headerArray[3] = clientTypeBuffer;
	
	struct curl_slist *cslist= NULL;
	cslist = curl_slist_append(cslist, headerArray[0]);
	cslist = curl_slist_append(cslist, headerArray[1]);
	cslist = curl_slist_append(cslist, headerArray[2]);
	cslist = curl_slist_append(cslist, headerArray[3]);

	curl = curl_easy_init();
	if ( curl )
	{
		curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible XCoin API C Client)");
		curl_easy_setopt( curl, CURLOPT_URL, url);
		curl_easy_setopt( curl, CURLOPT_POST, 1 ); 
		curl_easy_setopt( curl, CURLOPT_POSTFIELDS, pData ); 
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, cslist );
		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_data );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, &data );
		curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
		res = curl_easy_perform( curl );
		if ( res != CURLE_OK )
		{
			fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return NULL;
		}
		curl_slist_free_all( cslist );
		curl_easy_cleanup( curl );
	}
	return data.data;

}
