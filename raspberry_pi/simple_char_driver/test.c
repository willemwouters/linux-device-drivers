// String reversal done by device driver

#include<stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h> // for memset and strlen

int main(int argc ,char *argv[])
{

	assert(argc > 1);
	char buf[100] ;
	char i = 0;
	memset(buf, 0, 100);
	printf("Input: %s\n", argv[1]);

	int fp = open("/dev/myDev", O_RDWR);
	
	write(fp, argv[1], strlen(argv[1]));

	while(read(fp, &buf[i++], 1));

	printf("Reversed by the driver: %s\n" ,buf);

}
