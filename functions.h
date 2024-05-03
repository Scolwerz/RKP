#ifndef FUNCTIONS
#define FUNCTIONS

void SignalHandler(int sig);

int Measurement(int** Values);

unsigned char* u_int_bytes_little(unsigned int value);

unsigned char* u_int_bytes_big(unsigned int value);

unsigned int pack_rgba(int r, int g, int b, int a);

void BMPcreator(int *Values, int NumValues);

int FindPID();

void SendViaFile(int *Values, int NumValues);

void ReceiveViaFile(int sig);

void SendViaSocket(int *Values, int NumValues);

void ReceiveViaSocket();

#endif // FUNCTIONS
