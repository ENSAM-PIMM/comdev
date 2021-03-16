/*
load k
ofact('spfmex','FactBuild','ks=spfmex_utils(''servfact'',k,ks,[1 1 size(k,1)/32  0.01*size(k,1)]);');
kd=ofact(k);b=ones(size(k,1),2);q=kd\b; disp(norm(k*q-b)/norm(b));q=kd\b; norm(k*q-b)/norm(b)

ofact('spfmex','FactBuild','ks=spfmex_utils(''servfact'',k,ks,[1 1 size(k,1)/32  0.01*size(k,1)]);');
model=femesh('test ubeam');model=fe_case(model,'fixdof','base','z==0')
def=fe_eig(model,[6 5 0]);

[m,k,mdof]=fe_mk(model);
kd=ofact(k);b=ones(size(k,1),2);q=kd\b; disp(norm(k*q-b)/norm(b));q=kd\b; norm(k*q-b)/norm(b)

cd c:/dis_sdt/tcpip;webget_eb%('localhost:8888')

webserver_demo;

fid=fopen('c:/tmp/zzz','w+');fwrite(fid,0:10,'float64');fseek(fid,0,-1);a=fread(fid,'int8');fclose(fid);reshape(a,8,11)'

*/

#define myMexFunction pnet 
#define AutoTimeOut 300
#define VERSION "Version  2.0.5  2003-09-16"
    
/******* GENERAL DEFINES *********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifdef ForPetsc
#include "petscmat.h"
#else
#include "spfact.c"
#endif

/*  ----------------------------------------------------------------------
GENERAL IMPLEMENTATION OF USED MATLAB COMMANDS
    ---------------------------------------------------------------------- */

/*
 http://scilabsoft.inria.fr/doc/internals/node8.html#SECTION00032100000000000000
*/

typedef int mxArray;

#define mexPrintf printf
void mexErrMsgTxt(
    const char	*error_msg)	/* string with error message */
{printf(error_msg);}

int myMexErr( char* BUFF)
{ 
  printf("%s",BUFF);  
  return(1);
}
#define mexWarnMsgTxt printf


/*  ----------------------------------------------------------------------
END GENERAL IMPLEMENTATION OF USED MATLAB COMMANDS
    ---------------------------------------------------------------------- */


/******* WINDOWS ONLY DEFINES *********/
#ifdef WIN32
#define IFWINDOWS(dothis) dothis 
#define IFUNIX(dothis)
#define NeedSwapInt 0
#define NeedSwapDouble 0
/* #include <windows.h> */
#include <winsock2.h>
#define close(s) closesocket(s)
#define nonblockingsocket(s) {unsigned long ctl = 1;ioctlsocket( s, FIONBIO, &ctl );}
#define s_errno WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#define usleep(a) Sleep((a)/1000)
#define MSG_NOSIGNAL 0

/******* NON WINDOWS DEFINES *********/
#else
#define IFWINDOWS(dothis) 
#define IFUNIX(dothis) dothis
/* allow for unix/intel platforms */
#if defined(OSTYPEmexglx)  
#define NeedSwapInt 0
#define NeedSwapDouble 0
#else
#define NeedSwapInt 4
#define NeedSwapDouble 8
#endif
#define mxMalloc malloc
#define mxFree free

#include <errno.h>
#define s_errno errno 
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <fcntl.h>
#define nonblockingsocket(s)  fcntl(s,F_SETFL,O_NONBLOCK)
#endif

#ifndef INADDR_NONE
#define INADDR_NONE (-1)
#endif

/* Do this hack cause SIGPIPE that kills matlab on any platform???*/
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif


/********** DEFINES related to pnet own functionality *****************/
/*   Set debuging on/off   */
#define debug_view_con_status(X)  

#define MAX_CON         100       /* Maximum number of simultanius tcpip connections.*/
#define NAMEBUFF_LEN    100
#define MINBUFFSIZE     1000      /* readbuffer will shrink to this size if datalength is smaller. */
#define CBNAMELEN       30

#define CON_FREE         -1       /* Find a new free struct in struct array */

#define BACKLOG           50       /* How many pending connections queue will hold */

#define double_inf            HUGE_VAL
#define DEFAULT_WRITETIMEOUT  double_inf
#define DEFAULT_READTIMEOUT   double_inf
#define DEFAULT_INPUT_SIZE    50000
#define DEFAULT_USLEEP        100

/* Different status of a con_info struct handles a file descriptor    */
#define STATUS_FREE       -1
#define STATUS_NOCONNECT   0    /* Disconnected pipe that is note closed */
#define STATUS_TCP_SOCKET  1 
#define STATUS_IO_OK       5    /* Used for IS_... test */
#define STATUS_UDP_CLIENT  6
#define STATUS_UDP_SERVER  8
#define STATUS_CONNECT     10   /* Used for IS_... test */
#define STATUS_TCP_CLIENT  11
#define STATUS_TCP_SERVER  12
#define STATUS_UDP_CLIENT_CONNECT 18
#define STATUS_UDP_SERVER_CONNECT 19

#define IS_STATUS_IO_OK(x) ((x)>=STATUS_IO_OK)
#define IS_STATUS_CONNECTED(x) ((x)>=STATUS_CONNECT)
#define IS_STATUS_UDP_NO_CON(x) ((x)==STATUS_UDP_CLIENT || (x)==STATUS_UDP_SERVER)
#define IS_STATUS_TCP_CONNECTED(x) ((x)==STATUS_TCP_CLIENT || (x)==STATUS_TCP_SERVER)

typedef struct
{
    char *ptr;       /* Pointer to buffert. */
    int len;         /* Length of allocated buffert. */
    int pos;         /* Length used of buffer for data storage.*/
} io_buff;

/* Structure that hold all information about a tcpip connection. */
typedef struct
{
    int fid;
    double writetimeout;
    double readtimeout;
    struct sockaddr_in remote_addr;
    io_buff write;
    io_buff read;
    int status;       /* STATUS_... FREE, NOCONNECT, SERVER, CLIENT ... */
    char callback[CBNAMELEN+1];
} con_info;


/* Some global variables */
int        gret_args=0;         /* Global variable that holds number of matlab return argumens returned */
int            gnlhs;           /* number of expected outputs */
mxArray       **gplhs;          /* array of pointers to output arguments */
int            gnrhs;           /* number of inputs */
const mxArray  **gprhs;         /* array of pointers to input arguments */

/* Global list with static length of connections structures holding info about current connection */
con_info con[MAX_CON];
int con_index=0;                   /* Current index possition for list of handlers */
unsigned long mex_call_counter=0;  /* Counter that counts how many calls that have been done to pnet */

/***********************************************************************/
void Print_Start_Message(){
    mexPrintf("\n===============================================================================\n"

"SDT Factored Matrix Server\n"
	      "Copyright SDTools, 2003-2004 All Rights Reserved\n"
"\n\n$Revision: 1.12 $  $Date: 2006/03/31 15:46:45 $\n"
"\n===============================================================================\n"
	      "Using pnet MEX-file for the tcp/udp/ip-toolbox Compiled @ "
	      __DATE__ " " __TIME__  "\n"
	      VERSION "\n"
	      "Copyright (C) Peter Rydesäter, Sweden, et al. , 1998 - 2003\n"
	      "GNU General Public License, se license.txt for full license notis.\n"
	      "You are allowed to (dynamicaly) link this file with non-free code. \n\n"
	      "   http://www.rydesater.com \n\n"
	      "===============================================================================\n\n"
	      );
}

/***********************************************************************/
int myoptstrcmp(const char *s1,const char *s2)
{
    int val;
    while( (val= toupper(*s1) - toupper(*s2))==0 ){
	if(*s1==0 || *s2==0) return 0;
	s1++;
	s2++;
	while(*s1=='_') s1++;
	while(*s2=='_') s2++;
    }
    return val;
}

/********************************************************************/
void SwapInt(unsigned int *i1, int N) {

 unsigned int i2, j1, mask = 65280, mask2=16711680; /* FF00 FF0000 */

 for (j1=0;j1<N;j1++)
  { i2=i1[j1];
    i1[j1]=(i2<<24) | (i2>>24) | ((i2 & mask)<<8) | ((i2 & mask2)>>8);
  }
}
/********************************************************************/
void SwapDouble(char *ptr, int N) {

  int j1;
#ifndef SWAPDATA
#define SWAPDATA(a,b) { a^=b; b^=a; a^=b; }
#endif

 for (j1=0;j1<N*8;j1+=8)  {
   SWAPDATA(ptr[j1],ptr[j1+7]) SWAPDATA(ptr[j1+1],ptr[j1+6]) SWAPDATA(ptr[j1+2],ptr[j1+5]) SWAPDATA(ptr[j1+3],ptr[j1+4]) ;
  }
}

/********************************************************************/
void PrintChar(char *i1, int N) {

  int j1, i2;
  for (j1=0;j1<N;j1++) {i2=i1[j1];if (i2<0) {i2=i2+256;} printf("(%2i)",i2);
  if ((int)((j1+1)/8)*8==j1+1) printf("\n");
  }

}

/********************************************************************/
/* A "wrapper" function for memory allocation. Most for debuging /tracing purpose */
void *myrealloc(char *ptr,int newsize)
{
    ptr=realloc(ptr,newsize);
    if(ptr==NULL && newsize>0)
	mexErrMsgTxt("Internal out of memory!");
    return ptr;
}
/********************************************************************/
/* A "wrapper" function for memory allocation. Most for debuging /tracing purpose */

void *myCalloc(int len,int eltsize)
{
  char *ptr;
    ptr=calloc(len,eltsize);
    if(ptr==NULL && len>0)
	mexErrMsgTxt("Internal out of memory!");
    return ptr;
}

/********************************************************************/
/* A "wrapper" function for memory allocation. Most for debuging /tracing purpose */
void newbuffsize(io_buff *buff,int newsize)
{

    if(newsize==-1){
	free(buff->ptr);
	buff->ptr=NULL;
	return;
    }
    if(newsize<buff->pos)
	newsize=buff->pos;
    if(newsize<256)
	newsize=256;
    if(newsize>buff->len){   /* Grow.... */
	buff->ptr=myrealloc(buff->ptr,newsize*2);
	buff->len=newsize*2;
    }else if(newsize*4 < buff->len){ /* Decrease... */
	buff->ptr=myrealloc(buff->ptr,newsize*2);
	buff->len=newsize*2;
    }
}


/* Windows implementation of perror() function */
#ifdef WIN32
/********************************************************************/
void perror(const char *context )
{
    int wsa_err;
    wsa_err = WSAGetLastError();
    mexPrintf( "[%s]: WSA error: ", context );
    switch ( wsa_err )
    {
    case WSANOTINITIALISED: mexPrintf( "WSANOTINITIALISED\n" ); break;
    case WSAENETDOWN:       mexPrintf( "WSAENETDOWN      \n" ); break;
    case WSAEADDRINUSE:     mexPrintf( "WSAEADDRINUSE    \n" ); break;
    case WSAEACCES:         mexPrintf( "WSAEACCES        \n" ); break;
    case WSAEINTR:          mexPrintf( "WSAEINTR         \n" ); break;
    case WSAEINPROGRESS:    mexPrintf( "WSAEINPROGRESS   \n" ); break;
    case WSAEALREADY:       mexPrintf( "WSAEALREADY      \n" ); break;
    case WSAEADDRNOTAVAIL:  mexPrintf( "WSAEADDRNOTAVAIL \n" ); break;
    case WSAEAFNOSUPPORT:   mexPrintf( "WSAEAFNOSUPPORT  \n" ); break;
    case WSAEFAULT:         mexPrintf( "WSAEFAULT        \n" ); break;
    case WSAENETRESET:      mexPrintf( "WSAENETRESET     \n" ); break;
    case WSAENOBUFS:        mexPrintf( "WSAENOBUFS       \n" ); break;
    case WSAENOTSOCK:       mexPrintf( "WSAENOTSOCK      \n" ); break;
    case WSAEOPNOTSUPP:     mexPrintf( "WSAEOPNOTSUPP    \n" ); break;
    case WSAESHUTDOWN:      mexPrintf( "WSAESHUTDOWN     \n" ); break;
    case WSAEWOULDBLOCK:    mexPrintf( "WSAEWOULDBLOCK   \n" ); break;
    case WSAEMSGSIZE:       mexPrintf( "WSAEMSGSIZE      \n" ); break;
    case WSAEHOSTUNREACH:   mexPrintf( "WSAEHOSTUNREACH  \n" ); break;
    case WSAEINVAL:         mexPrintf( "WSAEINVAL        \n" ); break;

    case WSAECONNREFUSED:   mexPrintf( "WSAECONNREFUSED  \n" ); break;
    case WSAECONNABORTED:   mexPrintf( "WSAECONNABORTED  \n" ); break;
    case WSAECONNRESET:     mexPrintf( "WSAECONNRESET    \n" ); break;
    case WSAEISCONN:        mexPrintf( "WSAEISCONN       \n" ); break;
    case WSAENOTCONN:       mexPrintf( "WSAENOTCONN      \n" ); break;
    case WSAETIMEDOUT:      mexPrintf( "WSAETIMEDOUT     \n" ); break;
    default:                mexPrintf( "Unknown(%d) error!\n", wsa_err ); break;
    }
    return;
}
#endif

/********************************************************************/
/*Makes byte swapping, or not depending on the mode argument        */
void byteswapdata(char *ptr,const int elements,const int elementsize,int mode)
{
  /* MODE=0 Do nothing, MODE=1 Swap, MODE=2 network byte order, MODE=3 Intel byte order. */
#ifndef SWAPDATA
#define SWAPDATA(a,b) { a^=b; b^=a; a^=b; }
#endif

    const int ordertest=1;
    const char *is_intel_order=(const char *)(&ordertest);
    if(elementsize<2) return;
    if(is_intel_order[0]==1 && mode==2)   mode=1;
    if(is_intel_order[0]==0 && mode==3)   mode=1;
    if(mode==1){
	int e;
	switch(elementsize){
	case 2: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+1]) break;
	case 3: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+2])  break;
	case 4: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+3]) SWAPDATA(ptr[e+1],ptr[e+2])  break;
	case 5: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+4]) SWAPDATA(ptr[e+1],ptr[e+3])  break;
	case 6: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+5]) SWAPDATA(ptr[e+1],ptr[e+4]) SWAPDATA(ptr[e+2],ptr[e+3])  break;
	case 7: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+6]) SWAPDATA(ptr[e+1],ptr[e+5]) SWAPDATA(ptr[e+2],ptr[e+4])  break;
	case 8: for(e=0;e<elements*elementsize;e+=elementsize)
	    SWAPDATA(ptr[e],ptr[e+7]) SWAPDATA(ptr[e+1],ptr[e+6]) SWAPDATA(ptr[e+2],ptr[e+5]) SWAPDATA(ptr[e+3],ptr[e+4])  break;
	}
    }
}


/********************************************************************/
/* DEBUGING FUNCTION */
void __debug_view_con_status(char *str)
{
    int a;
    mexPrintf("%s\n",str);
    for(a=0;a<5;a++)
    {
	mexPrintf("[%02d] FID:%02d STATUS:%02d WRT.POS:%d RD.POS:%d ",a,con[a].fid,con[a].status,con[a].write.pos,con[a].read.pos);
	if(con[a].read.ptr)
	    mexPrintf("RD: %02x %02x %02x %02x %02x %02x %02x %02x ",
		      (int)con[a].read.ptr[0],(int)con[a].read.ptr[1],(int)con[a].read.ptr[2],(int)con[a].read.ptr[3],
		      (int)con[a].read.ptr[4],(int)con[a].read.ptr[5],(int)con[a].read.ptr[6],(int)con[a].read.ptr[7]);
	if(con[a].write.ptr)
	    mexPrintf("WR: %02x %02x %02x %02x %02x %02x %02x %02x ",
		      (int)con[a].write.ptr[0],(int)con[a].write.ptr[1],(int)con[a].write.ptr[2],(int)con[a].write.ptr[3],
		      (int)con[a].write.ptr[4],(int)con[a].write.ptr[5],(int)con[a].write.ptr[6],(int)con[a].write.ptr[7]);
	if(a==con_index)
	    mexPrintf("<--\n");
	else
	    mexPrintf("\n");
    }
    mexPrintf("--------------------\n");
}

/********************************************************************/
/* Portable time function using matlabs NOW                         */
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>

double my_now(){
  time_t ltime;
  double sec;
   char tmpbuf[128], ampm[] = "AM";
 
    time( &ltime );
    sec = (double)(ltime);
    return(sec);
}

/*******************************************************************************/
/* Checks that given index is valid index and set current index, "con_index"   */
/* to that point. If index is CON_FREE (-1) then is search done for a free one */
/* Returns 1 On success. 0 on error                                            */
int move_con(int idx)
{
    if(idx>=MAX_CON)
	mexErrMsgTxt("Invalid value of handler!\n");
    if(idx>=0)
    {
	con_index=idx;
	if(con[con_index].status==STATUS_FREE)
	    mexErrMsgTxt("No valid handler! already closed?");
	return 1;
    }
    debug_view_con_status("START MOVE");
    if(idx==CON_FREE)    /* Move con_index until it find a free non used struct */
    {
	for(con_index=0;con_index<MAX_CON;con_index++)
	{
	    debug_view_con_status("STEP MOVE");
	    if(con[con_index].status==STATUS_FREE)
		return 1;
	}
	mexErrMsgTxt("To many open connection! Forgot to close old connections?");
    }
    if(idx<0)
	mexErrMsgTxt("Unvalid value of handler!");
    return 0;
}



/********************************************************************/
/* Copys a matlab Char array to a char * string buffer              */
int InputArray2Buff(const char* in1,io_buff *buff)
{
    int len, swap=2, i1;

    len = strlen(in1);
    newbuffsize(buff,len);
    for(i1=0;i1<len;i1++)
	    buff->ptr[buff->pos++]=in1[i1];
    return len;
}





/**********************************************************************/
int ipv4_lookup(const char *hostname,int port)
{
    struct in_addr addr;    /* my address information */
    /* Try IP address */
    addr.s_addr=inet_addr(hostname);
    if (addr.s_addr==INADDR_NONE){
	/*Can't resolve host string as dot notation IP number...
	  try lookup IP from hostname */
	struct hostent *he;
	he=gethostbyname(hostname);
	if(he==NULL){
	    mexPrintf("\nUNKNOWN HOST:%s\n",hostname);
	    return -1;  /* Can not lookup hostname */
	}
	addr = *((struct in_addr *)he->h_addr);
    }
    con[con_index].remote_addr.sin_family=AF_INET;
    con[con_index].remote_addr.sin_port=htons(port);
    con[con_index].remote_addr.sin_addr=addr;
    memset(&con[con_index].remote_addr.sin_zero, 0,8);
    return 0;
}


/**********************************************************************/
/* Writes from specified position (pointer) in buffer of spec. length */
int writedata(char *ptr, int len)
{
    double timeoutat;
    const int  fid=con[con_index].fid;
    int sentlen=0, iscon=0;
    int retval=0;
    int lastsize=1000000;

    if (ptr==NULL) {ptr=con[con_index].write.ptr;iscon=1;}
    if (len<0) len=con[con_index].write.pos;

    if(con[con_index].status<STATUS_IO_OK)
	return 0;

    timeoutat=my_now()+con[con_index].writetimeout;
    while(sentlen<len)
    {
	if(lastsize<1000) usleep(DEFAULT_USLEEP);
	if(IS_STATUS_UDP_NO_CON(con[con_index].status)) {
	    retval = sendto(fid,&ptr[sentlen],len-sentlen,MSG_NOSIGNAL,
			    (struct sockaddr *)&con[con_index].remote_addr,sizeof(struct sockaddr));}
	else {
    retval=send(fid,&ptr[sentlen],len-sentlen,MSG_NOSIGNAL);}
	lastsize=retval>0?retval:0;
        if (lastsize>0) { timeoutat=my_now()+con[con_index].writetimeout;}
	sentlen+=lastsize;
	if(retval<0 && s_errno!=EWOULDBLOCK  IFWINDOWS( && s_errno!=WSAECONNRESET  )      	   
	   ){
	    con[con_index].status=STATUS_NOCONNECT;
	    mexPrintf("\nREMOTE HOST DISCONNECTED\n");
	    break;
	}
	if( !IS_STATUS_TCP_CONNECTED(con[con_index].status) && sentlen>0 )
	  {break;}
	if(timeoutat<=my_now())
	    {break;}
    }
    if (iscon) {
     con[con_index].write.pos=0;
     printf("sent %i doing memmove",con[con_index].write.pos);fflush(stdout);
      /* pmemmove(con[con_index].write.ptr, &con[con_index].write.ptr[sentlen], con[con_index].write.pos);
     newbuffsize(&con[con_index].write,con[con_index].write.pos);*/
     }
    return sentlen;
}

/********************************************************************/
/* Init current record with values                                  */
void init_con(int fid,int status)
{
    memset(&con[con_index],0,sizeof(con_info));
    con[con_index].fid=fid;
    con[con_index].status=status;
    con[con_index].writetimeout=DEFAULT_WRITETIMEOUT;
    con[con_index].readtimeout=DEFAULT_READTIMEOUT;
}

/********************************************************************/
/* Close con struct                                                 */
void close_con()
{
    if(con[con_index].fid>=0)
	close(con[con_index].fid);
    else
	mexWarnMsgTxt("Closing already closed connection!");
    newbuffsize(&con[con_index].write,-1);
    newbuffsize(&con[con_index].read,-1);
    init_con(-1,STATUS_FREE);
}

/*******************************************************************      */
/* Function to close all still open tcpip connections */
int closeall(void)
{
    int flag=0;
    for(con_index=0;con_index<MAX_CON;con_index++)
	if(con[con_index].fid>=0) {  /* Already closed?? */
	    close_con();
	    flag=1;
	}
    return flag;
}

/****************************************************************************/
/* This function is called on unloading of mex-file                         */
void CleanUpMex(void)
{
    if(closeall()) /* close all still open connections...*/
	mexWarnMsgTxt("Unloading mex file. Unclosed tcp/udp/ip connections will be closed!");
    IFWINDOWS(   WSACleanup();  );
}


/********************************************************************/
/* Read to fill buffer with specified length from UDP or TCP network*/
int bytes2buff(int len, int typ, int SwapFlag, char* BUF)
{
    double timeoutat;
    int retval=-1, pos, j1, readlen;
    char tp;

    pos=0; readlen=len; if (typ==0) readlen=1; 
    timeoutat=my_now()+con[con_index].readtimeout;
    if(0==IS_STATUS_IO_OK(con[con_index].status))
        return(-2);/* If not read/write fid (broken pipe) then exit.*/
    while(1){

	if(readlen>0){
	    if(IS_STATUS_CONNECTED(con[con_index].status))
		retval=recv(con[con_index].fid,&BUF[pos],readlen,MSG_NOSIGNAL);
	    else{
		struct sockaddr_in my_addr;
		int fromlen=sizeof(my_addr); 

		/* Copy 0.0.0.0 adress and 0 port to remote_addr as init-value.*/
		memset(&my_addr,0,sizeof(my_addr));
		con[con_index].remote_addr.sin_addr = my_addr.sin_addr;
		con[con_index].remote_addr.sin_port = my_addr.sin_port;
		retval=recvfrom(con[con_index].fid,&BUF[pos],
				readlen,MSG_NOSIGNAL,(struct sockaddr *)&my_addr, &fromlen);
		if (retval>0){
		    con[con_index].remote_addr.sin_addr = my_addr.sin_addr;
		    con[con_index].remote_addr.sin_port = htons((unsigned short int)ntohs(my_addr.sin_port));
		}
	    }
	    if( retval==0){
		mexPrintf("\nREMOTE HOST DISCONNECTED\n");
		con[con_index].status=STATUS_NOCONNECT;
		break;
	    }
	    if(retval<0 && s_errno!=EWOULDBLOCK	       ) {
		con[con_index].status=STATUS_NOCONNECT;
		perror( "recvfrom() or recv()" );
		break;
	    }
            timeoutat=my_now()+con[con_index].readtimeout;
	}
	readlen=retval>0?retval:0;

        if (typ==0){ /* read line and break if 0 */
          j1=pos+readlen; for (;pos<j1;pos++) {
            if (BUF[pos]==10 || BUF[pos]==13) {
	      if (pos>0) { printf("\nend readline %i-%i-%i\n",j1-readlen,readlen,pos); }
	      BUF[pos]='\0';len=pos-1; break;
	    }
          }
          if (readlen>1) printf("READ TOO MUCH\n");
          readlen=1;
          if (pos==0) {usleep(1000);}/* if nothing new sleep a while */
	} else	/* non text line read */
        { pos+=readlen;  if(readlen<1000) {usleep(1000);}
          readlen=len-pos;
        }

	if( !IS_STATUS_TCP_CONNECTED(con[con_index].status) && pos>0 )
	  { printf("Not connected"); break;}
	if( pos>=len-1 ) {
	 if (typ>0) { printf("\nRead bytes %i-%i\n",pos,len); }
         break;
        }
	if(timeoutat<=my_now())  { printf("time out %.0f readlen=%i\n",con[con_index].readtimeout,readlen); break;}
    }
    if (SwapFlag==4) SwapInt(&BUF[0],len/4);
    if (SwapFlag==8) SwapDouble(&BUF[0],len/8);

    return pos;
}


/********************************************************************/
/* Function Creating a tcpip connection and returns handler number  */
int tcp_connect(const char *hostname,const int port)
{
    if(ipv4_lookup(hostname,port)==-1)
	return -1;
    con[con_index].fid=socket(AF_INET, SOCK_STREAM, 0);
    if(con[con_index].fid== CON_FREE){
	/*Can't open socket */
	close_con();
	return -1;
    }
    if(connect(con[con_index].fid,(struct sockaddr *)&con[con_index].remote_addr,sizeof(struct sockaddr)) == -1){
	/*Can't connect to remote host. */
	close_con();
	return -1;
    }
    con[con_index].status=STATUS_TCP_CLIENT;
    nonblockingsocket(con[con_index].fid); /* Non blocking read! */
    return con_index;
}

/*******************************************************************     
 Function Creating a TCP server socket                                 
 or a connectionless UDP client socket.
*/
int tcp_udp_socket(int port,int dgram_f)
{
    int sockfd;
    struct sockaddr_in my_addr;    /* my address information */
    const int on=1;
    if(dgram_f)
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    else
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1)
	return -1;
    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(port);       /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    memset(&(my_addr.sin_zero),0, 8);        /* zero the rest of the struct */
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&on,sizeof(on));
    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1)
    {
	close(sockfd);
	return -1;
    }
    listen(sockfd,BACKLOG);
    nonblockingsocket(sockfd);
    return sockfd;
}

/*****************************************************************/
/* Listen to socket and returns connection if there is one...
   else it returns -1 */
int tcpiplisten(int noblock)
{
    const double timeoutat=my_now()+con[con_index].readtimeout;
    int new_fd;
    const int sock_fd= con[con_index].fid;
    int sin_size = sizeof(struct sockaddr_in);

    move_con(CON_FREE);        /* Find a new free con struct for the new connection.... */

    while(1){
	    if ((new_fd = accept(sock_fd, (struct sockaddr *)&con[con_index].remote_addr,&sin_size)) > -1)
            break;
        if(timeoutat<=my_now()|| noblock)
	        return -1;
	    usleep(DEFAULT_USLEEP);
    }
    nonblockingsocket(new_fd); /* Non blocking read! */
    setsockopt(new_fd,SOL_SOCKET,SO_KEEPALIVE,(void *)1,0); /* realy needed? */
    con[con_index].fid=new_fd;
    con[con_index].status=STATUS_TCP_SERVER;
    return con_index;
}

/*****************************************************************/
/* Make a UDP socket be a "connected" UDP socket                 */
int udp_connect(const char *hostname,int port)
{	
    if(con[con_index].status < STATUS_UDP_CLIENT)
	mexErrMsgTxt("Must pass UDP client or server handler!");
    if(ipv4_lookup(hostname,port)==-1)
	return -1;
    if(connect(con[con_index].fid,(struct sockaddr *)&con[con_index].remote_addr,sizeof(struct sockaddr)) == -1)
	return -1; /*Can't connect to remote host. */
    if(con[con_index].status == STATUS_UDP_CLIENT)
	con[con_index].status = STATUS_UDP_CLIENT_CONNECT;
    else
	con[con_index].status = STATUS_UDP_SERVER_CONNECT;
    nonblockingsocket(con[con_index].fid); /* Non blocking read! */
    return con[con_index].status;
}

/*****************************************************************/
	/* start the server */
/*****************************************************************/
int myMexFunction( int* in1, char* fun, void* BUF )
{
  int i1;

/* Initialization on first call to the mex file */
if (mex_call_counter==0)
    {
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int wsa_err;    
	wVersionRequested = MAKEWORD( 2, 0 );
	wsa_err = WSAStartup( wVersionRequested, &wsaData );
	if (wsa_err)
	    mexErrMsgTxt("Error starting WINSOCK32.");
#endif

    Print_Start_Message();
    /* Init all connecttions to to free */
    for(con_index=0;con_index<MAX_CON;con_index++)	   init_con(-1,STATUS_FREE);
    con_index=0;
    }
    mex_call_counter++;

    if(in1[0]==-1){
	/* GET FIRST ARGUMENT -- The "function" name */
	
	/* Find of the function name corresponds to a non connection associated function */
	if(myoptstrcmp(fun,"CLOSEALL")==0){
	    closeall();
	    return(0);
	}
	if(myoptstrcmp(fun,"TCPSOCKET")==0){
	    const int fd=tcp_udp_socket(in1[1], 0);
	    if(fd>=0){
		move_con(STATUS_FREE);
		init_con(fd,STATUS_TCP_SOCKET);
		return(con_index);
	    }else	return(-1);
	}
    } /* call with string only */
    /* Get connection handler and suppose that it is a connection assosiated function */

    /* Find given handel */

     if(move_con(in1[0])==0)  mexErrMsgTxt("Unknown connection handler");
       
    if(myoptstrcmp(fun,"CLOSE")==0){
	   close_con();
	return(1);
       }
    if(myoptstrcmp(fun,"TCPLISTEN")==0){

      printf("con[%i].status = %i",con_index,con[con_index].status);

     	if(con[con_index].status!=STATUS_TCP_SOCKET)
		{ mexErrMsgTxt("Invalid socket for LISTEN, Already open, or UDP?...");return(-2);}

        return(tcpiplisten(0));
    }
    if(myoptstrcmp(fun,"PRINTF")==0){
	InputArray2Buff(BUF,&con[con_index].write);

	if(IS_STATUS_TCP_CONNECTED(con[con_index].status))
	  { writedata(NULL,-1); fflush(stdout);}
	    return(1);
    }
    if(myoptstrcmp(fun,"READBYTES")==0){
	    if(IS_STATUS_TCP_CONNECTED(con[con_index].status))
	      i1=bytes2buff(in1[1],1,in1[2],BUF); return(i1);
    }
    if(myoptstrcmp(fun,"READLINE")==0){
      if(IS_STATUS_TCP_CONNECTED(con[con_index].status))
	{ i1=bytes2buff(1024,0,0,BUF);return(i1); }
      else {printf("Not connected cannot read line");return(-1);}
	
    }
    if(myoptstrcmp(fun,"STATUS")==0){ return(con[con_index].status); }
    if(myoptstrcmp(fun,"GETHOST")==0){
	int n;
	double ip_bytes[4] = {0, 0, 0, 0};
	const unsigned char *ipnr=(const unsigned char *)&con[con_index].remote_addr.sin_addr;
	for(n=0;n<4;n++)
	    ip_bytes[n] = (double)ipnr[n];
	return;
    }
    if(myoptstrcmp(fun,"SETREADTIMEOUT")==0){
	con[con_index].readtimeout=(double)in1[1];
	return;
    }
    mexErrMsgTxt("Unknown 'function name' in argument.");
}


/* ----------------------------------------------------------------------
 THIS CONTAINS THE SPFMEX CALLS
   ---------------------------------------------------------------------- */

#ifdef ForPetsc
static int      cF=-1, indF[20];
#else
static FactorInfo      FI[20];
static void ClearAllFactors(void)
  { ClearMtx(FI,-1); }
#endif

/* ----------------------------------------------------------------------
 THIS IS THE MAIN SERVER FUNCTION
   ---------------------------------------------------------------------- */

int main(int argc, char* argv[])
{


  double timeoutat;

  int sock[10], out, in1[10], j1;
  char    CAM[256];

#ifdef ForPetsc
  Mat                APetsc,fact;
  MPI_Comm           comm;
  PetscErrorCode     ierr;
  IS                    Petscrow,Petsccol; 
  MatFactorInfo         pf;
  Vec                   X,B;
  PetscScalar    	*Xarray;
#endif

  CAM[0]='\0';   printf("Start\n");

#ifdef ForPetsc
  PetscFunctionBegin;
  ierr = PetscInitialize(&argc,&argv,0,0);CHKERRQ(ierr);
  comm = MPI_COMM_SELF;
#endif

in1[0]=-1; in1[1]=8888;  out=myMexFunction(in1,"TCPSOCKET",CAM);
sock[0]=out;
printf("Open %i\n",sock[0]);

in1[0]=sock[0]; in1[1]=1; out=myMexFunction(in1,"SETREADTIMEOUT",CAM);

fprintf(stdout,"timeout %i\n",(int)con[con_index].readtimeout);

printf("\nStarting listening %i \n",mex_call_counter);

/* Open Connection - - - - - - - - - - - - - - - - - - - - - - - - - -*/
while (mex_call_counter<99)  
    {
    out=myMexFunction(in1,"TCPLISTEN",CAM);
    printf(" counter %i, out %i\n",(int)mex_call_counter,out);
    fflush(stdout);
    if( out!=-1.0 || out==-2 ) break;
    }

  if (in1[0]==-1) {printf("No connection received");return;}
  printf("Received incoming connection\n");
  in1[0]=out;  

  timeoutat=my_now()+AutoTimeOut;

/* Loop when the server is running - - - - - - - - - - - - - - - - - - -*/
while (my_now()<timeoutat)
    {

      /* receive a command from the client */ 
      con[con_index].writetimeout=1.0;
      j1=pnet(in1,"READLINE",CAM);
      if (j1==-1) return(-1); /* disconnected */

      if (CAM[0]!='\0') printf("readline->%s<-(%i)",CAM,(int)CAM[0]);
      if ( (mex_call_counter/40)*40==mex_call_counter) printf("\n");

     if (CAM[0]=='G'){

      usleep(DEFAULT_USLEEP);
      out=pnet(in1,"PRINTF","connecting done\n");
      out=pnet(in1,"PRINTF","status\n");
      out=pnet(in1,"PRINTF","end of reading\n");
      printf("done sending\n");

#if defined(ForPetsc)
/* PETSCFACT -------------------------------------------------------------- */
     } else if (!strcmp("PETSCFACT",CAM)){

      int *ir, *jc, i1[10];
      double *pr; 

      
      con[con_index].readtimeout=10;

      /* read header info  [M N NZMAX 0] */
      printf(" i1 : ");
      usleep(1000);in1[1]=4*4;in1[2]=NeedSwapInt;
      printf("NeedSwapInt %i", NeedSwapInt);
      j1=pnet(in1,"READBYTES",i1);

      for (j1=0;j1<4;j1++) printf("%i ",i1[j1]);
      fflush(stdout);

      jc=myCalloc(i1[0]+1,sizeof(int));
      ir=myCalloc(i1[2],sizeof(int));
      pr=myCalloc(i1[2],sizeof(double));

      fflush(stdout);

      usleep(1000);

      fflush(stdout);

      in1[1]=i1[2]*4; in1[2]=NeedSwapInt;  j1=pnet(in1,"READBYTES",ir);  
      if (j1!=in1[1]) {printf("Failed getting ir");return;}
      in1[1]=i1[0]*4+4; in1[2]=NeedSwapInt;  j1=pnet(in1,"READBYTES",jc); 
      if (j1!=in1[1]) {printf("Failed getting jr %i-%i",j1,in1[1]);return;}
      in1[1]=i1[2]*8; in1[2]=NeedSwapDouble;  j1=pnet(in1,"READBYTES",pr);
      if (j1!=in1[1]) {printf("Failed getting pr %i-%i",j1,in1[1]);return;}
      usleep(1000);

     printf("N=%i:nnz=%i:jc[end]=%i\n",i1[0],i1[2],jc[i1[0]]);
 
      printf("jc :");
      for (j1=0;j1<i1[0]+1;j1++) printf("%i ",jc[j1]);
      printf("\nir :");
      for (j1=0;j1<i1[2];j1++) printf("%i ",ir[j1]);
      printf("\npr :");
      for (j1=0;j1<i1[2];j1++) printf("%f ",pr[j1]);

	const PetscInt *nnzPetsc2, *nnzPetsc;
        int i,j,j2,j3,irr=0,occurence=0;
        
        PetscMalloc(i1[0]*sizeof(PetscInt),&nnzPetsc);
        PetscMalloc(i1[0]*sizeof(PetscInt),&nnzPetsc2);

        for (i=0;i<i1[0];i++) {
	nnzPetsc[i] = jc[i+1] - jc[i]; 
        occurence=0;
        for (j=0;j<i1[2];j++) {if(ir[j]==i) occurence++;}
        nnzPetsc2[i]=occurence;
        printf("i,  nnzPetsc[i] %i %i\n", i, nnzPetsc[i]); 
        printf("i,  nnzPetsc2[i] %i %i\n", i, nnzPetsc2[i]); 
        }


/* PetscErrorCode MatCreateSeqAIJ(MPI_Comm comm,PetscInt m,PetscInt n,
                                PetscInt nz,const PetscInt nnz[],Mat *A)
Collective on MPI_Comm
Input Parameters
        comm    - MPI communicator, set to PETSC_COMM_SELF
        m       - number of rows
        n       - number of columns
        nz      - number of nonzeros per row (same for all rows)
        nnz     - array containing the number of nonzeros in the various rows
        (possibly different for each row) or PETSC_NULL
*/

        /* Create Matrix  - nnzPetsc2 array containing the number of nonzeros in the various rows
        */
  	ierr = MatCreateSeqAIJ(comm,i1[0],i1[0],0,nnzPetsc2,&APetsc); CHKERRQ(ierr);

  	for (j=0; j<i1[0]; j++) {
    	    for (j3=0 ; j3 <nnzPetsc[j] ; j3++) {
      	    ierr = MatSetValue(APetsc,ir[irr],j,pr[irr],INSERT_VALUES); CHKERRQ(ierr);
            irr++; }
  	}

  	ierr = MatAssemblyBegin(APetsc,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  	ierr = MatAssemblyEnd(APetsc,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr); 
                                                                                                   
  	printf("\nPetsc matrix:\n\n"); MatView(APetsc,0);
  
/* 	Reordering
     type 	- type of reordering, one of the following:
     MATORDERING_NATURAL - Natural
     MATORDERING_ND - Nested Dissection
     MATORDERING_1WD - One-way Dissection
     MATORDERING_RCM - Reverse Cuthill-McKee
     MATORDERING_QMD - Quotient Minimum Degree
*/
  ierr = MatGetOrdering(APetsc,MATORDERING_NATURAL,&Petscrow,&Petsccol);CHKERRQ(ierr);                                                                                                     
/* Initalise options for factorization */
  ierr = MatFactorInfoInitialize(&pf);CHKERRQ(ierr);


/* 
Performs symbolic LU factorization of A. Call this routine before calling MatLUFactorNumeric().
 MatLUFactorSymbolic(Mat mat,IS row,IS col,MatFactorInfo *info,Mat *fact)
Input Parameters
	mat 	- the matrix
	row, col 	- row and column permutations
	info 	- options for factorization, includes
        fill - expected fill as ratio of original fill.
        dtcol - pivot tolerance (0 no pivot, 1 full column pivoting)
                  Run with the option -log_info to determine an optimal value to use
Output Parameter
fact -new matrix that has been symbolically factored
*/
  ierr = MatLUFactorSymbolic(APetsc,Petscrow,Petsccol,&pf,&fact);CHKERRQ(ierr); 

/* Performs numeric LU factorization of APetsc  */
  ierr = MatLUFactorNumeric(APetsc,&fact);CHKERRQ(ierr); 

/*  Performs in-place LU factorization of matrix. */ 
/*  ierr = MatLUFactor(APetsc,Petscrow,Petsccol,&pf); */

  ierr= PetscViewerSetFormat(PETSC_VIEWER_STDOUT_SELF,PETSC_VIEWER_ASCII_MATLAB);
  printf("\nPetsc factor matrix:\n\n"); MatView(fact,PETSC_VIEWER_STDOUT_SELF);

/****************************************************/

     pnet(in1,"PRINTF","DONE\n");
     i1[0]=cF;if (NeedSwapInt)  SwapInt(&i1[0],1);
/*     writedata(&i1[0],1*4);  Return factor number  */
     printf("Done factor %i\n",cF); fflush(stdout);
      free(ir); free(jc); free(pr); 

     timeoutat=my_now()+AutoTimeOut;

/* PETSCSOLVE ------------------------------------------------------------- */
     } else if (!strcmp("PETSCSOLVE",CAM)){

      int error, i1[10], j1, j2, M, N;
      double *pXi, *pYi; 

      con[con_index].readtimeout=10;

      /* read header info  [M N CF 0] */
      usleep(1000);in1[1]=4*4; in1[2]=NeedSwapInt; j1=pnet(in1,"READBYTES",i1);

      for (j2=0;j2<4;j2++) printf("i1 %i \n", i1[j2]);

      cF=i1[2]; M=i1[0]; N=i1[1]; 

       pXi=myCalloc(M*N,sizeof(double));
       pYi=myCalloc(M*N,sizeof(double));

       usleep(100); in1[1]=M*N*8; in1[2]=NeedSwapDouble;
       j1=pnet(in1,"READBYTES",pYi); 
       
       for (j2=0;j2<(M*N);j2++) printf("pYi %f \n", pYi[j2]);

       ierr = VecCreateSeq(comm,M*N,&B); CHKERRQ(ierr);
       ierr = VecDuplicate(B,&X); CHKERRQ(ierr);

       /* PYi to B */	
       for (j2=0;j2<(M*N);j2++) {
         ierr = VecSetValue(B,j2,pYi[j2],INSERT_VALUES); CHKERRQ(ierr);
       }
       /*       ierr=VecAssemblyBegin(B); ierr=VecAssemblyEnd(B); */
       printf("\nPetsc vector:\n\n"); VecView(B,0);

       printf("ready to solve "); 
       ierr = MatSolve(fact,B,X);CHKERRQ(ierr); /* MatSolve(A,B,X); if "in-place factorization" */
       printf("\nSolution : \n\n"); VecView(X,0);

       /* X to PXi */
       ierr = VecGetArray(X,&Xarray); CHKERRQ(ierr);
       for (j2=0;j2<(M*N);j2++) {pXi[j2]=Xarray[j2]; printf("pXi %f \n", pXi[j2]);}
       ierr = VecRestoreArray(X,&Xarray); CHKERRQ(ierr);
       

       printf(" -> done\n");fflush(stdout);

       pnet(in1,"PRINTF","double\n");printf(".0\n");fflush(stdout);
       i1[0]=2;i1[2]=N; /* i1[1]=FI[cF].neqns; */  
       if (NeedSwapInt) SwapInt(&i1[0],3);
       

/*      writedata(&i1[0],3*4);  
       usleep(1000);
       printf("sending q(%ix%i) (%i bytes)",M,N,M*N*8);
       if (NeedSwapDouble) SwapDouble(&pXi[0],M*N); 
       writedata(&pXi[0],M*N*8);  */

       printf("-> done\n");fflush(stdout);

       free(pXi);free(pYi); 

/*      } */

     timeoutat=my_now()+AutoTimeOut;

/* PETSCCLEAR --------------------------------------------------------- */
     } else if (!strcmp("PETSCCLEAR",CAM)){

/*      int error, i1[10], j1; */

  ierr = MatDestroy(APetsc);CHKERRQ(ierr); 

/*      in1[1]=1*4;  j1=pnet(in1,"READBYTES",i1);  SwapInt(&i1[0],1); */
/*      cF=i1[0]; 
      ClearMtx(FI,cF); */

       printf("Cleared factor %i\n",cF);fflush(stdout);

#else
/* SPOOLESFACT -------------------------------------------------------------------- */
     } else if (!strcmp("SPOOLESFACT",CAM)){

      int *ir, *jc, i1[10];
      double *pr; 

      con[con_index].readtimeout=10;

      /* read header info  [M N NZMAX 0] */
      printf(" i1 : ");
      usleep(1000);in1[1]=4*4;in1[2]=NeedSwapInt; j1=pnet(in1,"READBYTES",i1);

      for (j1=0;j1<4;j1++) printf("%i ",i1[j1]);
      fflush(stdout);

      jc=myCalloc(i1[0]+1,sizeof(int));
      ir=myCalloc(i1[2],sizeof(int));
      pr=myCalloc(i1[2],sizeof(double));

      usleep(1000);
      in1[1]=i1[2]*4; in1[2]=NeedSwapInt;  j1=pnet(in1,"READBYTES",ir);  
      if (j1!=in1[1]) {printf("Failed getting ir");return;}
      in1[1]=i1[0]*4+4; in1[2]=NeedSwapInt;  j1=pnet(in1,"READBYTES",jc); 
      if (j1!=in1[1]) {printf("Failed getting jr %i-%i",j1,in1[1]);return;}
      in1[1]=i1[2]*8; in1[2]=NeedSwapDouble;  j1=pnet(in1,"READBYTES",pr);
      if (j1!=in1[1]) {printf("Failed getting pr %i-%i",j1,in1[1]);return;}
      usleep(1000);

     printf("N=%i:nnz=%i:jc[end]=%i\n",i1[0],i1[2],jc[i1[0]]);

     cF=0; while (cF<20&&FI[cF].neqns!=0) { cF++; }
     if (cF==19) {mexErrMsgTxt("20 factors is the max, use ofact clear");}
     fflush(stdout);

     FI[cF].msgFile = stdout ;

     FI[cF].neqns   = i1[0];
     FI[cF].maxdomainsize = (int)FI[cF].neqns/32;
     FI[cF].maxzeros      = (int)(0.01*FI[cF].neqns);
     FI[cF].msglvl        = 0;
     FI[cF].matlabmsglvl  = 0;
     printf("c"); fflush(stdout);

     mexPrintf("\n SPFMEX Factor %i (isreal = %d, non-sym = %d, N = %i, NZ=%i", 
	       cF,1,0,FI[cF].neqns,jc[FI[cF].neqns]) ; 
     /* type,symmetryflag,pivotingflag, ir, jc pr pi,seed */
     FI[cF]=FactorMtx(FI[cF],1,0,1,ir,jc,pr,NULL,10101);

     pnet(in1,"PRINTF","DONE\n");
     i1[0]=cF;if (NeedSwapInt)  SwapInt(&i1[0],1);
     writedata(&i1[0],1*4); /* Return factor number */
     printf("Done factor %i\n",cF); fflush(stdout);
     free(ir); free(jc); free(pr); 
     timeoutat=my_now()+AutoTimeOut;

/* SPOOLESSOLVE -------------------------------------------------------------------- */
     } else if (!strcmp("SPOOLESSOLVE",CAM)){

      int error, i1[10], j1, j2, M, N;
      double *pX, *pY, *pXi, *pYi; 

      con[con_index].readtimeout=10;

      /* read header info  [M N CF 0] */
      usleep(1000);in1[1]=4*4; in1[2]=NeedSwapInt; j1=pnet(in1,"READBYTES",i1);

      cF=i1[2]; M=i1[0]; N=i1[1]; 
      if (i1[0]!=FI[cF].neqns) 
        { mexErrMsgTxt("k and b have different number of rows"); }
      else {

       pX=myCalloc(M*N,sizeof(double));
       pXi=myCalloc(M*N,sizeof(double));
       pY=myCalloc(M*N,sizeof(double));
       pYi=myCalloc(M*N,sizeof(double));

       usleep(100); in1[1]=M*N*8; in1[2]=NeedSwapDouble; j1=pnet(in1,"READBYTES",pYi); 
       for (j2=0;j2<N;j2++){for (j1=0;j1<M;j1++) { pY[j1+j2*M]=pYi[((*(FI[cF].newToOldIV)).vec)[j1]+j2*M];}}
       
       printf("ready to solve %i ",FI[cF].neqns);

       error=SolveMtx(FI[cF],FI[cF].type,N,pX,NULL,pY,NULL);


       for (j2=0;j2<N;j2++){for (j1=0;j1<M;j1++) { pXi[((*(FI[cF].newToOldIV)).vec)[j1]+j2*M]=pX[j1+j2*M];}}
       
       printf(" -> done\n");fflush(stdout);

       pnet(in1,"PRINTF","double\n");printf(".0\n");fflush(stdout);
       i1[0]=2;i1[2]=N; i1[1]=FI[cF].neqns;   if (NeedSwapInt) SwapInt(&i1[0],3);
       writedata(&i1[0],3*4); /* datadims,M,N */

       usleep(1000);
       printf("sending q(%ix%i) (%i bytes)",M,N,M*N*8);
       if (NeedSwapDouble) SwapDouble(&pXi[0],M*N); 
       writedata(&pXi[0],M*N*8); /* send data */
       printf("-> done\n");fflush(stdout);

       free(pX);free(pY); free(pXi);free(pYi); 

      }
     timeoutat=my_now()+AutoTimeOut;

/* SPOOLESCLEAR --------------------------------------------------------- */
     } else if (!strcmp("SPOOLESCLEAR",CAM)){

      int error, i1[10], j1;

      in1[1]=1*4;  j1=pnet(in1,"READBYTES",i1);  SwapInt(&i1[0],1);
      cF=i1[0]; 
      ClearMtx(FI,cF);
       printf("Cleared factor %i\n",cF);fflush(stdout);

#endif     
/* QUIT ------------------------------------------------------------------ */
     } else if (!strcmp("QUIT",CAM)){

       printf("Quit\n"); return(1);

     } /* end of commands */
     CAM[0]='\0';
     fflush(stdout);

/* ------------------------------------------------------------------ */
    }

  printf("sdt_server exited because of auto TimeOut\n");
  CleanUpMex();


}


