//
// Created by Newton on 2020/6/7.
//

#ifndef SUPERDUPERSERVER_SUPERDUPERLIB_H
#define SUPERDUPERSERVER_SUPERDUPERLIB_H


#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#define socklen_t int
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define closesocket close
#endif

#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#define MAX_UDP_LTH 512
#endif //SUPERDUPERSERVER_SUPERDUPERLIB_H
