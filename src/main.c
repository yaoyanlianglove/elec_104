#include <stdio.h>
#include <string.h>
#include "tcp.h"
int main()
{
    printf("elec_104\n");
    Tcp_Listen("192.168.1.102", 2404);
    return 0;    
}

