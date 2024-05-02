#ifndef FUNCTIONS
#define FUNCTIONS

void SignalHandler(int sig);

int Measurement(int** Values);

unsigned char* u_int_bytes(unsigned int value);

unsigned int pack_rgba(int r, int g, int b, int a);

void BMPcreator(int *Values, int NumValues);

int FindPID();

void SendViaFile(int *Values, int NumValues);

void ReceiveViaFile(int sig);

void SendViaSocket(int *Values, int NumValues);

void ReceiveViaSocket();

void stop_server(int sig);

#endif // FUNCTIONS
