#include "comutile.h"
#ifdef linux
 #include "limits.h"
#endif

void recv_data_bloc(int sock,char* & data,size_t size)
{
    //printf("receving %d byte <<< ",size);
    if(data==NULL)data=(char*)malloc(size);

    char* data_pos=&data[0];
    size_t recvb_glob=0;
    do
    {
		int to_recvb;
		recv(sock,(char*)&to_recvb,sizeof(int),0);
		int recvb_loc=0;
		do
		{
	        int recvb=recv(sock,data_pos,to_recvb-recvb_loc,0);
			recvb_loc+=recvb;	
			data_pos=&data[recvb_glob+recvb_loc];
		}while(recvb_loc!=to_recvb);
		int stop;send(sock,(char*)&stop,sizeof(int),0);
        recvb_glob+=(size_t)recvb_loc;
        data_pos=&data[recvb_glob];
	}while(recvb_glob!=size);  

    //printf("%i\n",1);
    int retval=1;
    send(sock,(char*)&retval,sizeof(int),0);
}

void send_data_bloc(int sock,char* data,size_t size)
{
    //printf("sending %d byte >>> ",size);
    char* data_pos=&data[0];
    size_t sendb_glob=0;

	int sb_size;
	socklen_t opt_len=sizeof(sb_size);
	int res=getsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&sb_size,&opt_len);

    do
    {
        int sendb=(size-sendb_glob)<sb_size?(size-sendb_glob):sb_size;
        send(sock,(char*)&sendb,sizeof(int),0);
		send(sock,data_pos,sendb,0);
		int stop;recv(sock,(char*)&stop,sizeof(int),0);
        sendb_glob+=(size_t)sendb;
        data_pos=&data[sendb_glob];
    }while(sendb_glob!=size);
    
    int retval=0;
    recv(sock,(char*)&retval,sizeof(int),0);
    //printf("%i\n",retval);
}

//---------------------------------------------------------------------------//

void recv_sparse_matrix_from_client(int sock,int& mtype,int& m,int& n,int& complex,
                                    MKL_INT*& ia,MKL_INT*& ja,double*& a)
{
    recv(sock,(char*)&mtype,sizeof(int),0);
    recv(sock,(char*)&m,sizeof(int),0);
    recv(sock,(char*)&n,sizeof(int),0);
    recv(sock,(char*)&complex,sizeof(int),0);

	int size_index_serv=sizeof(MKL_INT);send(sock,(char*)&size_index_serv,sizeof(int),0);
	int size_index_client;recv(sock,(char*)&size_index_client,sizeof(int),0);
	int expend_index=((size_index_client==4)&&(size_index_serv==8))?1:0;
    
	recv_data_bloc(sock,(char*&)ia,size_index_client*(m+1));
  
	size_t i;
	if(expend_index)
	{
		ia=(MKL_INT*)realloc(ia,sizeof(MKL_INT)*(m+1));
		int* ia_int=(int*)ia;for(i=0;i<m+1;i++)ia[m-i]=ia_int[m-i];
	}
	for(i=0;i<m+1;i++)ia[i]++;/* use 1 based indexing*/
	
	size_t size_ja=ia[m]-1;
	recv_data_bloc(sock,(char*&)ja,size_index_client*size_ja);
	if(expend_index)
	{
		ja=(MKL_INT*)realloc(ja,sizeof(MKL_INT)*size_ja);
		int* ja_int=(int*)ja;for(i=0;i<size_ja;i++)ja[size_ja-1-i]=ja_int[size_ja-1-i];
	}
	for(i=0;i<size_ja;i++)ja[i]++;/* use 1 based indexing*/
	
    if(complex)
    {
        if(a==NULL)a=(double*)malloc(2*sizeof(double)*size_ja);
        double* a_bis=(double*)malloc(sizeof(double)*size_ja);
        recv_data_bloc(sock,(char*&)a,sizeof(double)*size_ja);
        recv_data_bloc(sock,(char*&)a_bis,sizeof(double)*size_ja);
        for(i=0;i<size_ja;i++)a[2*(size_ja-1-i)]=a[size_ja-1-i];
        for(i=0;i<size_ja;i++)a[2*i+1]=a_bis[i];
        free(a_bis);
    }
    else recv_data_bloc(sock,(char*&)a,sizeof(double)*size_ja);
}

void send_sparse_matrix_to_serv(int sock,int mtype,int m, int n,int complex,
                                size_t* ia,size_t* ja,double* a,double* ima)
{
	send(sock,(char*)&mtype,sizeof(int),0);
    send(sock,(char*)&m,sizeof(int),0);
    send(sock,(char*)&n,sizeof(int),0);
    send(sock,(char*)&complex,sizeof(int),0);

	int size_index_serv;recv(sock,(char*)&size_index_serv,sizeof(int),0);
	int compresse_index=((size_index_serv==4)&&(sizeof(size_t)==8))?1:0;
	int size_index_client=compresse_index?4:sizeof(size_t);
	send(sock,(char*)&size_index_client,sizeof(int),0);

	size_t size_ja=ia[m];
	if(compresse_index)
	{
		size_t i;
		int* ia_int=(int*)ia;for(i=0;i<(m+1);i++)ia_int[i]=ia[i];
		int* ja_int=(int*)ja;for(i=0;i<size_ja;i++)ja_int[i]=ja[i];
	}
	
    send_data_bloc(sock,(char*&)ia,size_index_client*(m+1));
    send_data_bloc(sock,(char*&)ja,size_index_client*size_ja);

	if(compresse_index)
	{
		size_t i;
		int* ia_int=(int*)ia;for(i=0;i<(m+1);i++)ia[m-i]=ia_int[m-i];
		int* ja_int=(int*)ja;i;for(i=0;i<size_ja;i++)ja[size_ja-1-i]=ja_int[size_ja-1-i];
	}

	send_data_bloc(sock,(char*&)a,sizeof(double)*size_ja);
    if(complex)send_data_bloc(sock,(char*&)ima,sizeof(double)*size_ja);
}

//---------------------------------------------------------------------------//

void recv_matrix_from_client(int sock,int& m,int& n,int& complex,double*& a)
{
    recv(sock,(char*)&m,sizeof(int),0);
    recv(sock,(char*)&n,sizeof(int),0);
    recv(sock,(char*)&complex,sizeof(int),0);
    if(complex)
    {
        if(a==NULL)a=(double*)malloc(2*sizeof(double)*m*n);
        double* a_bis=(double*)malloc(sizeof(double)*m*n);
        recv_data_bloc(sock,(char*&)a,sizeof(double)*m*n);
        recv_data_bloc(sock,(char*&)a_bis,sizeof(double)*m*n);
        size_t i;
        for(i=0;i<m*n;i++)a[2*(m*n-1-i)]=a[m*n-1-i];
        for(i=0;i<m*n;i++)a[2*i+1]=a_bis[i];
        free(a_bis);
    }
    else recv_data_bloc(sock,(char*&)a,sizeof(double)*m*n);
}

void send_matrix_to_serv(int sock,int m,int n,int complex,double* a,double* ima)
{
    send(sock,(char*)&m,sizeof(int),0);
    send(sock,(char*)&n,sizeof(int),0);
    send(sock,(char*)&complex,sizeof(int),0);
    send_data_bloc(sock,(char*&)a,sizeof(double)*m*n);
    if(complex)send_data_bloc(sock,(char*&)ima,sizeof(double)*m*n);
}

//---------------------------------------------------------------------------//

void recv_matrix_from_serv(int sock,int& m,int& n,int& complex,double*& a,double*& ima)
{
    recv(sock,(char*)&m,sizeof(int),0);
    recv(sock,(char*)&n,sizeof(int),0);
    recv(sock,(char*)&complex,sizeof(int),0);
    recv_data_bloc(sock,(char*&)a,sizeof(double)*m*n);
    if(complex)recv_data_bloc(sock,(char*&)ima,sizeof(double)*m*n);
}

void send_matrix_to_client(int sock,int m,int n,int complex,double* a)
{
    send(sock,(char*)&m,sizeof(int),0);
    send(sock,(char*)&n,sizeof(int),0);
    send(sock,(char*)&complex,sizeof(int),0);
    if(complex)
    {
        double* a_bis=(double*)malloc(sizeof(double)*m*n);
        size_t i;
        for(i=0;i<m*n;i++)a_bis[i]=a[2*i];
        send_data_bloc(sock,(char*&)a_bis,sizeof(double)*m*n);
        for(i=0;i<m*n;i++)a_bis[i]=a[2*i+1];
        send_data_bloc(sock,(char*&)a_bis,sizeof(double)*m*n);
        free(a_bis);
    }
    else send_data_bloc(sock,(char*&)a,sizeof(double)*m*n);
}

//---------------------------------------------------------------------------//

void send_str(int sock,string* p_str)
{
	int size=p_str->size();
	send(sock,(char*)&size,sizeof(int),0);
	if(size!=0)send_data_bloc(sock,(char*)p_str->c_str(),sizeof(char)*size);
}

void recv_str(int sock,string* p_str)
{
	int size;recv(sock,(char*)&size,sizeof(int),0);
	if(size!=0)
	{
		p_str->resize(size);
		char* p_c_str=(char*)p_str->c_str();
		recv_data_bloc(sock,(char*&)p_c_str,sizeof(char)*size);
	}
}

//---------------------------------------------------------------------------//

void send_int_data(int sock,int size,int* data)
{
	send(sock,(char*)&size,sizeof(int),0);
	send_data_bloc(sock,(char*)data,sizeof(int)*size);
}

void recv_int_data(int sock,int& size,int*& data)
{
	recv(sock,(char*)&size,sizeof(int),0);
	recv_data_bloc(sock,(char*&)data,sizeof(int)*size);
}

//---------------------------------------------------------------------------//

void send_float_data(int sock,int size,double* data)
{
	send(sock,(char*)&size,sizeof(int),0);
	send_data_bloc(sock,(char*)data,sizeof(double)*size);
}

void recv_float_data(int sock,int& size,double*& data)
{
	recv(sock,(char*)&size,sizeof(int),0);
	recv_data_bloc(sock,(char*&)data,sizeof(double)*size);
}

//---------------------------------------------------------------------------//
