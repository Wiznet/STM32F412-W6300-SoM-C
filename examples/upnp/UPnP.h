#ifndef __UPNP_H
#define __UPNP_H

#include <stdint.h>
#include "hyperterminal.h"
#include "socket.h"
#include "netutil.h"
#include "httpParser.h"

signed char SSDPProcess(SOCKET sockfd);
signed char GetDescriptionProcess(SOCKET sockfd);
signed char SetEventing(SOCKET sockfd);
void eventing_listener(SOCKET s);
signed short DeletePortProcess(SOCKET sockfd, const char* protocol, const unsigned int extertnal_port);
signed short AddPortProcess(SOCKET sockfd, const char* protocol, const unsigned int extertnal_port, const char* internal_ip, const unsigned int internal_port, const char* description);

signed char parseHTTP(const char* xml);
signed char parseSSDP(const char* xml);
signed char parseDescription(const char* xml);
void parseEventing(const char* xml);
signed short parseError(const char* xml);
signed short parseDeletePort(const char* xml);
signed short parseAddPort(const char* xml);

int ValidATOI(char* str, int base, int* ret);
void replacetochar(char * str, char oldchar, char newchar);
char C2D(unsigned char c);
char VerifyIPAddress_orig(char* src);
char VerifyIPAddress(char* src, unsigned char * ip);
unsigned char CheckDestInLocal(unsigned long destip);
void itoa2(unsigned short n, unsigned char* str, unsigned char len);

void data_process_count_handle(void);
#endif
