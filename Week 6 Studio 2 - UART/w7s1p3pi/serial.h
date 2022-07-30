#ifndef __SERIAL__
#define __SERIAL__
#define MAX_BUFFER_LEN		1024

void startSerial(const char *portName, int baudRate, int byteSize, char parity, int stopBits, int maxAttempts);

int serialRead(char *buffer);
void serialWrite(char *buffer, int len);
#endif

