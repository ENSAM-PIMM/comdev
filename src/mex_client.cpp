#include "comutile.h"
#include <iostream>
#ifdef OSTYPEmexa64
#include "string.h"
#endif
#include "mex.h"
 using namespace std;

//int MatlabPid=0;
//void ExitFcn (void) {
//    char* ip_addr=mxArrayToString(mxGetCell(prhs[1],0));
//    u_short port=*((double*)mxGetPr(mxGetCell(prhs[1],1))); 
//}

void mxCreateInt32(mxArray*& mxval,int val) {
	mxval=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
    *((int*)mxGetPr(mxval))=val;
	return;
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[] ) {
                 int nrhs, const mxArray *prhs[] ) {
    //-----------------------------------------------------------------------//

    char* commande;int* p_commande,p_help[1]={1886152040};
    if(nrhs==0)
	{
        commande=mxArrayToString(mxCreateString("help"));
        p_commande=p_help;
    } 
	else 
	{
		commande=mxArrayToString(prhs[0]);
		p_commande=(mxGetNumberOfElements(prhs[0])==4)?(int*)commande:p_help;
    }
    //mexPrintf("commande : %s -> %i\n",commande,*p_commande);
    
	if(nrhs>1)
	{
		if(!mxIsCell(prhs[1])){mexErrMsgTxt(" expecting cell!\n");mxCreateInt32(plhs[0],-1);return;}
		if(mxGetNumberOfElements(prhs[1])!=2){mexErrMsgTxt("show help !\n");mxCreateInt32(plhs[0],-1);return;}
    }
    else if(!strcmp("help",commande))//((*p_commande)==1886152040)//"help" 
    {
        mexPrintf("\n\
    - mklserv_client('newf',serv,factor_id) create new factor\n\
        serv : cell array, {ip,port}\n\
    - mklserv_client('delf',serv,factor_id) delete factor \n\
    - mklserv_client('setf',serv,factor_id) set active factor \n\
	- [ret,info]=mklserv_client('geti',serv) get pardiso information of active factor : \n\
			info(1-67): pardiso parameter\n\
			info(68): error\n\
			info(69): time last sym facto\n\
			info(70): time last num facto\n\
			info(71): time last solve\n\
    - [ret,stat]=mklserv_client('staf',serv) get factor status \n\
        stat(1,:): factor id, (2,:) status decode with dec2bin(stat(2,:),11)\n\
	    SOLVE_C-SOLVE_B-SOLVE_A-NUM_FACTO_C-\n\
        NUM_FACTO_B-NUM_FACTO_A-SYM_FACTO_C-SYM_FACTO_B-SYM_FACTO_A-MAT_B_SET-MAT_A_SET\n\
            *_A : started, *_B : ready to finalise, *_C : finalised\n\
    - ret=mklserv_client('3264',serv) get server index type (32/64)\n\
    - replace pardiso parameter : mklserv_client('setp',serv,param)\n\
	  param : array of 64+3 value : \n\
			param(1:64): pardiso parameter \n\
			param(65): Maximum number of numerical factorizations\n\
			param(66): Which factorization to use\n\
			param(67): Print statistical information in file\n\
		or 15 value \n\
		    param(n): [default]\n\
			param(1): [2] Fill-in reordering from METIS\n\
			param(2): [1] Numbers of processors, value of OMP_NUM_THREADS\n\
			param(3): [0] No iterative-direct algorithm\n\
			param(4): [0] No user fill-in reducing permutation\n\
			param(5): [2] Max numbers of iterative refinement steps\n\
			param(6): [13] Perturb the pivot elements with 1E-13\n\
			param(7): [1] Use nonsymmetric permutation and scaling MPS\n\
			param(8): [0] Maximum weighted matching algorithm is switched-on (default for non-symmetric)\n\
			param(9): [0] Output: Number of perturbed pivots\n\
			param(10): [-1] Output: Number of nonzeros in the factor LU\n\
			param(11): [-1] Output: Mflops for LU factorization\n\
			param(12): [0] Output: Numbers of CG Iterations\n\
			param(13): [1] Maximum number of numerical factorizations\n\
			param(14): [1] Which factorization to use\n\
			param(15): [1] Print statistical information in file\n\
	- mklserv_client('seta',serv,A.',type) set the matrix A, and her type :\n\
        1 : real and structurally symmetric matrix\n\
        2 : real and symmetric positive definite matrix\n\
     (-)2 : real and symmetric indefinite matrix\n\
        3 : complex and structurally symmetric matrix\n\
        4 : complex and Hermitian positive definite matrix\n\
     (-)4 : complex and Hermitian indefinite matrix\n\
        6 : complex and symmetric matrix\n\
       11 : real and unsymmetric matrix\n\
       13 : complex and unsymmetric matrix\n\
    - mklserv_client('setb',serv,b) set the b matrix\n\
    - mklserv_client('syfr',serv) symbolic factorisation\n\
    - Status=mklserv_client('syfc',serv) check symbolic factorization \n\
        0 : not launched yet, 1 : runing, 2 : done\n\
    - mklserv_client('nufr',serv) numeric factorization \n\
    - status=mklserv_client('nufc',serv) check numeric factorization\n\
    - mklserv_client('solr',serv) solve Ax=b \n\
    - status=mklserv_client('solc',serv) check solve \n\
    - [ret,x]=mklserv_client('getx',serv) get the solution matrix x \n\
    - mklserv_client('sout',serv) get string output \n\
    - mklserv_client('quit',serv) stop the server\n");
        return;
    } else if  (!strcmp("cvs",commande))  {
      plhs[0]=mxCreateString("$Revision: 145 $  $Date: 2020-05-22 15:34:28 +0200 (Fri, 22 May 2020) $");
      return;
    }
     int Verbose=0;

    //-----------------------------------------------------------------------//
     if (!mxIsChar(mxGetCell(prhs[1],0))) {
     if(!strcmp("loca",commande)) { 
        mxCreateInt32(plhs[0],0); /* non local */
     }
	return;
      }
   char* ip_addr=mxArrayToString(mxGetCell(prhs[1],0));
     u_short port=*((double*)mxGetPr(mxGetCell(prhs[1],1))); 
     //-----------------------------------------------------------------------//
     #if defined(WIN32)||defined(WIN64)
      WSADATA WSAData;
      if(WSAStartup(MAKEWORD(2,0),&WSAData)!= NO_ERROR) {
        mexPrintf("Error at WSAStartup()\n");
        mxCreateInt32(plhs[0],-1);return;
     }
    #endif
     //--------------------------------------------------------------------------//
     if (Verbose) mexPrintf("Opening socket");
     SOCKET sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
     if(sock == INVALID_SOCKET) {
        mexPrintf("Error at socket()\n");
        #if defined(WIN32)||defined(WIN64)
        WSACleanup();
        #endif
		mxCreateInt32(plhs[0],-1);
        return;
     }

    //--------------------------------------------------------------------------//

    sockaddr_in sin;
    sin.sin_addr.s_addr = inet_addr(ip_addr);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
	
	int sb_size=1024*1024;// 1 mega byte
	socklen_t opt_len=sizeof(sb_size);
	int res_setsnd=setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&sb_size,opt_len);
	int res_setrcv=setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&sb_size,opt_len);
	//int res_getsnd=getsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&sb_size,&opt_len);
	//int res_getrcv=getsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&sb_size,&opt_len);
   if (Verbose) mexPrintf("server at %s:%i trying_\n",ip_addr,port);

      if (connect(sock,(SOCKADDR*)&sin,sizeof(sin))==SOCKET_ERROR) {
      mexPrintf("server at %s:%i ...\n",ip_addr,port);
      #if defined(WIN32)||defined(WIN64)
		int errCode= WSAGetLastError();
		  // ..and the human readable error string!!
         // Interesting:  Also retrievable by net helpmsg 10060
// was LPWSTR
      #define CurLP LPWSTR
        CurLP errString = NULL;  // will be allocated and filled by FormatMessage
       int size = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM, // use windows internal message table
                 0,errCode,0,(CurLP)&errString,0, 0 ); 
        mexPrintf( "Error code %d:  %s\n\n", errCode, errString) ;
        LocalFree( errString ) ; // if you don't do this, you will get an
        WSACleanup();
      #else
        mexPrintf("Failed to connect.\n");
      #endif
      mxCreateInt32(plhs[0],-1);
      if (nlhs==2) plhs[1]=mxCreateNumericMatrix(0,0,mxDOUBLE_CLASS,mxREAL);
      return;
    }
    // else mexPrintf("connected with server.\n");
    
    //--------------------------------------------------------------------------//
	if (Verbose) mexPrintf("Sending"); 
	send(sock,(char*)p_commande,sizeof(int),0);  /* sends command */ 
    
    if(((*p_commande)==1719100782)//"newf"
        ||((*p_commande)==1718379876)//"delf"
        ||((*p_commande)==1718904179))//"setf"
    {          
        mexPrintf("server at %s:%i ...\n",ip_addr,port);
        int id=(int)*((double*)mxGetPr(prhs[2]));
        send(sock,(char*)&id,sizeof(int),0);
        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
    }
    else if((*p_commande)==1886676339) {//"setp" set parameters
        size_t nb_param=mxGetM(prhs[2])*mxGetN(prhs[2]);
        double* param=mxGetPr(prhs[2]);

        send_matrix_to_serv(sock,nb_param,1,0,param,NULL);
        
        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
    } else if((*p_commande)==1635018099) {//"seta"  set matrix
        size_t na=mxGetM(prhs[2]);
        size_t ma=mxGetN(prhs[2]);
        int complex=mxIsComplex(prhs[2]);
        double* pa=mxGetPr(prhs[2]);
        double* pima=mxGetPi(prhs[2]);
        size_t* ja=(size_t*)mxGetIr(prhs[2]);
        size_t* ia=(size_t*)mxGetJc(prhs[2]);
        double mtype=*((double*)mxGetPr(prhs[3]));
        
        send_sparse_matrix_to_serv(sock,mtype,ma,na,complex,ia,ja,pa,pima);

        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
    } else if((*p_commande)==1651795315) {//"setb" set RHS
        size_t mb=mxGetM(prhs[2]);
        size_t nb=mxGetN(prhs[2]);
        int complex=mxIsComplex(prhs[2]);
        double* pb=mxGetPr(prhs[2]);
        double* pimb=mxGetPi(prhs[2]);

        send_matrix_to_serv(sock,mb,nb,complex,pb,pimb);

        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
    } else if(((*p_commande)==1919318387)//"syfr"
        ||((*p_commande)==1667660147)//"syfc"
        ||((*p_commande)==1919317358)//"nufr"
        ||((*p_commande)==1667659118)//"nufc"
        ||((*p_commande)==1919709043)//"solr"
        ||((*p_commande)==1668050803)//"solc"
		||((*p_commande)==1717855602))//"redf"
    {

        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
    } else if((*p_commande)==2020894055){ //"getx" receive vector    
        int mx=0;
        int nx=0;
        int complex=0;
        double* px=NULL;
        double* pimx=NULL;

        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
        if(*((int*)mxGetPr(plhs[0])))
        {
            recv_matrix_from_serv(sock,mx,nx,complex,px,pimx);
            plhs[1]=mxCreateDoubleMatrix(mx,nx,complex?mxCOMPLEX:mxREAL);
            memcpy(mxGetPr(plhs[1]),px,sizeof(double)*nx*mx);
            free(px);
            if(complex)
            {
                memcpy(mxGetPi(plhs[1]),pimx,sizeof(double)*nx*mx);
                free(pimx);
            }
        }
		else mxCreateInt32(plhs[1],-1);
     } else if((*p_commande)==1953853299) { //sout
        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
        if(*((int*)mxGetPr(plhs[0])))
        {
            string str;
            recv_str(sock,&str);
            if(str.size()!=0)
			{
				mexPrintf("\n#---------------------------------------------\
                        -----------------------\nbegin deported console\n");
				mexPrintf("%s\n",str.c_str());
				mexPrintf("end deported console\n#-------------------------\
                        -------------------------------------------\n");
			}
        }
    } else if((*p_commande)==1717662835) { //"staf" factor status
		plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
        if(*((int*)mxGetPr(plhs[0])))
        {
			int size=0;
			int* data=NULL;
			recv_int_data(sock,size,data);
			plhs[1]=mxCreateNumericMatrix(2,size/2,mxINT32_CLASS,mxREAL);
			memcpy(mxGetPr(plhs[1]),data,sizeof(int)*size);
			free(data);
		}
		else {
            plhs[1]=mxCreateNumericMatrix(1,0,mxINT32_CLASS,mxREAL);
            //mexPrintf("No factor on server \n");
        }
    } else if ((*p_commande)==875967027 || //"3264"
         (*p_commande)==1734633840) {//"pidg" get Matlab PID from server
	    mexPrintf("Waiting for %s\n",commande);
        plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetData(plhs[0]),sizeof(int),0);
    } else if (!strcmp("quit",commande)) {
    } else if ((*p_commande)==1769235815)
	{ //"geti" get information
		plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        recv(sock,(char*)mxGetPr(plhs[0]),sizeof(int),0);
        if(*((int*)mxGetPr(plhs[0]))) {
			int size=0;
			double* data=NULL;
			recv_float_data(sock,size,data);
			plhs[1]=mxCreateDoubleMatrix(1,size,mxREAL);
			memcpy(mxGetPr(plhs[1]),data,sizeof(double)*size);
			free(data);
		} else {
			plhs[1]=mxCreateNumericMatrix(1,0,mxINT32_CLASS,mxREAL);
			mexPrintf("No factor on server \n");
	    }
     } else {
		mexPrintf("%s %i unknown !\n",commande,p_commande);
     }
   
    //-----------------------------------------------------------------------//
    
    closesocket(sock);
#if defined(WIN32)||defined(WIN64)
    WSACleanup();
#endif
    mexPrintf("."); // connection closed

    //-----------------------------------------------------------------------//
    
    return;
}
