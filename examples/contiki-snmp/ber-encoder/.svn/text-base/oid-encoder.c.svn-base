#include <stdio.h>
#include <string.h>

void main(int argc, char *argv[]) {
	char* src;
	if (argc == 2) {
		src = argv[1];
	} else {
		printf("error: wrong number of arguments\n\n");
		printf("the input format:\n");
		printf("./oid-encoder <oid>\n\n");
		printf("example:\n");
		printf("./oid-encoder 1.3.6.1.2.2.4\n\n");

		return;
	}

	int num[1000];
	int len = 0;
	int val = 0;
	int i;
	for (i = 0; i < strlen(src) + 1; i++) {
		if (i < strlen(src) && src[i] >= '0' && src[i] <= '9') {
			val = val * 10 + src[i] - '0';
		} else {
			num[len] = val;
			len++;			
			val = 0;
		}
	}

	unsigned char str[] = {0x13, 0x14};

	printf("{");
    val = num[0] * 40 + num[1];
	printf("0x%02x", val);

	for (i = 2; i < len; i++) {
		int value = num[i];
		int length = 0;
		if (value >= (268435456)) { // 2 ^ 28
			length = 5;
		} else if (value >= (2097152)) { // 2 ^ 21
			length = 4;
		} else if (value >= 16384) { // 2 ^ 14
			length = 3;
		} else if (value >= 128) { // 2 ^ 7
			length = 2;
		} else {
			length = 1;
		}

		int j = 0;
		for (j = length - 1; j >= 0; j--) {
		    if (j) {
				int p = ((value >> (7 * j)) & 0x7F) | 0x80;
		        printf(", 0x%02x", p);
		    } else {
		        int p = ((value >> (7 * j)) & 0x7F);
		        printf(", 0x%02x", p);
		    }
		}
	}
	printf("};\n");
}

