#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int build_listener(const char * port);
int build_sender(const char * hostname, const char * port);
#endif