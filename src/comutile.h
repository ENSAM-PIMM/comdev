#pragma once

#if defined(OSTYPEmexw64) 
#define WIN64 1
#endif
#if defined(OSTYPEmexw32) 
#define WIN32 1
#endif
#if defined(OSTYPEmexa64) 
#define linux 1
#endif
#if defined(OSTYPEmexmaci64) 
#define linux 1
#endif

#if defined(WIN32)||defined(WIN64) /* si vous etes sous Windows */
#include <winsock2.h>
#define socklen_t int 
#elif defined (linux) /* si vous êtes sous Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#ifndef SOCKET
 typedef int SOCKET; 
#endif
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#else /* sinon vous êtes sur une plateforme non supportée */
#error not defined for this platform
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(MKL_ILP64)
#define MKL_INT long long
#else
#define MKL_INT int
#endif

#include <iostream>
using namespace std;

//#pragma comment(lib, "ws2_32.lib")

void recv_data_bloc(int sock,char* & data,size_t size);
void send_data_bloc(int sock,char* data,size_t size);

void recv_sparse_matrix_from_client(int sock,int& mtype,int& m,int& n,int& complex,
                                    MKL_INT*& ia,MKL_INT*& ja,double*& a);
void send_sparse_matrix_to_serv(int sock,int mtype,int m, int n,int complex,
                                size_t* ia,size_t* ja,double* a,double* ima);

void recv_matrix_from_client(int sock,int& m,int& n,int& complex,double*& a);
void send_matrix_to_serv(int sock,int m,int n,int complex,double* a,double* ima);

void recv_matrix_from_serv(int sock,int& m,int& n,int& complex,double*& a,double*& ima);
void send_matrix_to_client(int sock,int m,int n,int complex,double* a);

void send_str(int sock,string* p_str);
void recv_str(int sock,string* p_str);

void send_int_data(int sock,int size,int* data);
void recv_int_data(int sock,int& size,int*& data);

void send_float_data(int sock,int size,double* data);
void recv_float_data(int sock,int& size,double*& data);



