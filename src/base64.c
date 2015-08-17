/**
 * Base64 encoding implementation
 * @see http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
 */

#include <stdlib.h>
#include "global.h"
#include "base64.h"

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static int mod_table[] = {0, 2, 1};

void base64_encode(const uint8_t *in, uint8_t *out, uint16_t input_length) {
	uint32_t i,j;
	for(i=0, j=0; i<input_length;) {
		uint32_t octet_a = i < input_length ? (unsigned char)in[i++] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char)in[i++] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char)in[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		out[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		out[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		out[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		out[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for(i=0; i<mod_table[input_length % 3]; i++)
		out[BASE64LEN(input_length) - 1 - i] = '=';

	out[BASE64LEN(input_length)] = '\0';
}
