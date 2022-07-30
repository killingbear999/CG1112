#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "serial.h"

int main()
{
	char buffer[MAX_BUFFER_LEN];

	memset(buffer, 0, MAX_BUFFER_LEN);

	startSerial("/dev/ttyACM1", B115200, 7, 'E', 1, 5);

	if(fork() != 0)
	{
		while(1)
		{
			// Parent will read keyboard
			char ch = getchar();
			if(ch == '1' || ch == '2')
				serialWrite(&ch, sizeof(char));
		}

	} 
	else
	{
		while(1)
		{
			int n;
			n=serialRead(buffer);
			buffer[n]=0;
			printf("Received %d bytes: %s\n", n, buffer);
		}
	}
}


