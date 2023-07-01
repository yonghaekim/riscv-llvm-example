#include <stdio.h>
#include <stdlib.h>

int main(void) {
	char *buf = (char *) malloc(20);
	sprintf(buf, "Hello, World!\n");
	printf("%s", buf);
}
