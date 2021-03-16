#include "comutile.h"
#include "c_pardiso.h" 
#include "c_red_flux.h"

#include <map>
using namespace std;

c_red_flux* p_redflux=NULL;

//---------------------------------------------------------------------------//

class c_set_sol
{
private:
    long id_active_psol;
    map<int,c_pardiso*>  map_psol;
public:
    c_set_sol():id_active_psol(-1){};
    ~c_set_sol(){};

    bool create(int id)
    {
        if(map_psol.find(id)!=map_psol.end())return 0;
        map_psol.insert(make_pair(id,new c_pardiso));
        id_active_psol=id;
        return 1;
    };

    bool erase(int id)
    {
        map<int,c_pardiso*>::iterator i=map_psol.find(id);
        if(i==map_psol.end())return 0;
        pair<int,c_pardiso*> p=*i;
        delete p.second;
        map_psol.erase(id);
        return 1;
    };
    
    bool set_active(int id)
    {
        if(map_psol.find(id)==map_psol.end())return 0;
        id_active_psol=id;
        return 1;
    };

    c_pardiso* get_active()
    {
        if(id_active_psol==-1)return NULL;
        map<int,c_pardiso*>::iterator i=map_psol.find(id_active_psol);
        if(i==map_psol.end())return NULL;
        pair<int,c_pardiso*> p=*i;
        return p.second;
    };

	int get_statistic(vector<int>* p_sta)
	{
		if(map_psol.empty())return 0;
		map<int,c_pardiso*>::iterator i;
		for(i=map_psol.begin();i!=map_psol.end();i++)
		{
			pair<int,c_pardiso*> paire_i=*i;
			p_sta->push_back(paire_i.first);
			p_sta->push_back(paire_i.second->Status);
		}
		return 1;
	};
};

//---------------------------------------------------------------------------//

void switch_task(int sock,c_set_sol* p_ss,int taskid, int Verbose, 
        int MatlabPid)
{
    //-----------------------------------------------------------------------//
	
	if(taskid==1769235815)//"geti" fprintf('%i\n',comstr('geti',-32))
    {   
		int s_data;
		double* p_data;
        printf("getting pardiso info...");
		c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
		send(sock,(char*)&b,sizeof(int),0);
		if(b){printf(" done\n");
			  p_p->get_pardiso_info(s_data,p_data);
		      send_float_data(sock,s_data,p_data);}
		else printf(" cannot proces, no factor set\n");
	}
	else if(taskid==1717855602)//"redf" 
    {   
        printf("redirecting stdout and stderr...\n");
		int b=0;if(p_redflux==NULL){p_redflux=new c_red_flux;b=p_redflux!=NULL;}
		send(sock,(char*)&b,sizeof(int),0);
	}
	else if(taskid==1719100782)//"newf"
    {          
        printf("creating new factor ");
        int id;recv(sock,(char*)(&id),sizeof(int),0);
        printf("id : %i ",id);
        int b=p_ss->create(id);send(sock,(char*)(&b),sizeof(int),0);
        if (b) printf(" done\n"); else printf("Error : failed\n"); 
    }
    else if(taskid==1718379876)//"delf"
    {
        printf("deleting factor ");
        int id;recv(sock,(char*)(&id),sizeof(int),0);
        printf("id : %i ",id);
        int b=p_ss->erase(id);send(sock,(char*)(&b),sizeof(int),0); 
        if(b)printf(" done\n"); else printf(" Error : failed\n");
    }
    else if(taskid==1718904179)//"setf"
    {
        printf("setting active factor ");
        int id;recv(sock,(char*)(&id),sizeof(int),0);
        printf("id : %i",id);
        int b=p_ss->set_active(id);send(sock,(char*)(&b),sizeof(int),0); 
        if(b) printf(" done\n"); else printf("Error failed\n");
    }
	else if(taskid==1717662835)//"staf"
    {
        printf("Factors statistic\n");
		vector<int> stats;
        int b=p_ss->get_statistic(&stats);send(sock,(char*)(&b),sizeof(int),0); 
        if(b)send_int_data(sock,stats.size(),&(stats[0]));
        else printf("there is no factor ! \n");
    } else if(taskid==1886676339) {//"setp"    
        printf("Setting parameter ");
        int m,n,complex;
        double* m_param=NULL;
        recv_matrix_from_client(sock,m,n,complex,m_param);
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b)b=p_p->set_param(m,m_param);
        if(b) printf("parameter set\n");else printf("Error failed\n");
        send(sock,(char*)(&b),sizeof(int),0);
        free(m_param);
    } else if(taskid==1635018099) {//"seta"
        if (Verbose) printf("setting matrix a");
        int mtype,m,n,complex;
        MKL_INT* ia=NULL;
        MKL_INT* ja=NULL;
        double* a=NULL;
        recv_sparse_matrix_from_client(sock,mtype,m,n,complex,ia,ja,a);

        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b)b=p_p->set_matrix_a(mtype,m,n,complex,ia,ja,a);
		if (b) {if (Verbose) printf("done\n");}
        else{free(ia);free(ja);free(a);printf(" failed\n");}
        send(sock,(char*)(&b),sizeof(int),0);
    } else if(taskid==1651795315) {//"setb"
        if (Verbose) printf("seting vector b ");
        int m,n,complex;
        double* m_b=NULL;
        recv_matrix_from_client(sock,m,n,complex,m_b);

        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b)b=p_p->set_vector_b(m,n,complex,m_b);
		if (b) {if (Verbose) printf("done\n");}
        else{free(m_b);printf("Failed\n");}
        send(sock,(char*)(&b),sizeof(int),0);
	} else if(taskid==1919318387) {//"syfr"
        if (Verbose) printf("symbolic factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){printf("processing...\n");p_p->sym_facto();}
        else printf("cannot proces, no factor set\n");
        send(sock,(char*)(&b),sizeof(int),0);
    } else if(taskid==1667660147) {//"syfc"
        printf("checking symbolic factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){printf("processing...\n");b=p_p->sym_facto_stat();}
        else printf("cannot proces, no factor set\n");
        send(sock,(char*)&b,sizeof(int),0);
    } else if(taskid==1919317358) {//"nufr"
        if (Verbose) printf("numeric factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){printf("processing...\n");p_p->num_facto();}
        else printf("cannot proces, no factor set\n");
        send(sock,(char*)&b,sizeof(int),0);
    } else if(taskid==1667659118) { //"nufc"
        printf("checking numeric factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){printf("processing...\n");b=p_p->num_facto_stat();}
        else printf("cannot proces, no factor set\n");
        send(sock,(char*)&b,sizeof(int),0);
    } else if(taskid==1919709043) {//"solr"
        if (Verbose) printf("solve\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){p_p->solve();}
        else printf("cannot proces, no factor set\n");
        send(sock,(char*)&b,sizeof(int),0);
    } else if(taskid==1668050803) {//"solc"
        if (Verbose) printf("checking solve\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){b=p_p->solve_stat(); }
        else printf("cannot proces, no factor set\n");
        send(sock,(char*)&b,sizeof(int),0);
    } else if(taskid==2020894055) {//"getx"
		int m,n,complex;
		double* x;
        if (Verbose) printf("getting solution\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){b=p_p->get_vector_x(m,n,complex,x);
			  send(sock,(char*)&b,sizeof(int),0);
			  if(b)send_matrix_to_client(sock,m,n,complex,x);}
		else{send(sock,(char*)&b,sizeof(int),0);printf("cannot proces, no factor set\n");}
    } else if(taskid==1953853299) {//"sout"	
		c_pardiso* p_p=p_ss->get_active();int b=(p_p!=NULL)&&(p_redflux!=NULL);
		send(sock,(char*)&b,sizeof(int),0);
		if(b){string out;p_redflux->get_out(&out);send_str(sock,&out);}
	} else if(taskid==1734633840) {//"pidg"	
		printf("MatlabPid=%i \n",MatlabPid);
    	send(sock,(char*)&MatlabPid,sizeof(int),0);
	} else if(taskid==875967027) {//"3264"
		int size_mklint=sizeof(MKL_INT)*8;
		send(sock,(char*)&size_mklint,sizeof(int),0);
	} else {
		printf("command %i does not exist !\n",taskid);
		send(sock,0,sizeof(int),0); /* send something back */
	}
    if (Verbose>1) printf("end switch_task\n");
}

//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//

int main(int argc, char *argv[])
{	
	int port=1707; int Verbose=0; int MatlabPid=0;
    if(argc<2)fprintf(stderr,"Using default port %i\n",port);
	else port=atoi(argv[1]);
    if(argc>2) Verbose=atoi(argv[2]);
    if(argc>3) MatlabPid=atoi(argv[3]);

    //-----------------------------------------------------------------------//
    
    printf("$Revision: 105 $ - $Date: 2017-01-24 15:02:20 +0100 (Tue, 24 Jan 2017) $ \n");
    printf("Verbose = %i, argc=%i\n",Verbose,argc); 
    //--------------------------------------------------------------------------//
#if defined(WIN32)||defined(WIN64)   
    WSADATA WSAData;
    if(WSAStartup(MAKEWORD(2,0),&WSAData)!= NO_ERROR)
    {
        printf("Error at WSAStartup()\n"); return 1;
    }
#endif
    //--------------------------------------------------------------------------//
    SOCKET sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sock == INVALID_SOCKET) {
		printf("Error at socket()\n");
        #if defined(WIN32)||defined(WIN64)
        WSACleanup();
        #endif
        return 1;
    }
    if (Verbose) printf("Done socket open\n");
    //--------------------------------------------------------------------------//
    
    SOCKADDR_IN sin;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if(bind(sock,(SOCKADDR*)&sin,sizeof(sin))== SOCKET_ERROR)
    {
        printf("bind() failed.\n");
        closesocket(sock);
#if defined(WIN32)||defined(WIN64)
        WSACleanup();
#endif
        return 1;
    }
    if (Verbose) printf("Step3\n");

	int sb_size=1024*1024;// 1 mega byte
	socklen_t opt_len=sizeof(sb_size);
	int res_setsnd=setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&sb_size,opt_len);
	int res_setrcv=setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&sb_size,opt_len);
	//int res_getsnd=getsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&sb_size,&opt_len);
	//int res_getrcv=getsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&sb_size,&opt_len);
    if (Verbose) printf("Step4\n");

    if(listen(sock,0)==SOCKET_ERROR) {
      printf("Error listening on socket.\n"); closesocket(sock);
      #if defined(WIN32)||defined(WIN64)
        WSACleanup();
      #endif
      return 1;
    }

    //--------------------------------------------------------------------------//

	task_scheduler_init init(2);  if (Verbose) printf("Step5\n");    
    c_set_sol set_sol;
    bool close_server=0;
    
    while(!close_server) {
        SOCKADDR_IN csin;
        SOCKET csock;
        socklen_t sinsize=sizeof(csin);
        if (Verbose) printf("Step6 accept\n");    
        csock=accept(sock,(SOCKADDR *)&csin,&sinsize);
        if (Verbose) printf("Step6 accepted\n");    
        if (csock== INVALID_SOCKET) {
            printf("invalid socket ERROR(1)"); break;
        } else {
            if (Verbose>1) printf("connection opened Verbose %i\n",Verbose);
            do
            {
                int taskid;
                if(recv(csock,(char*)(&taskid),sizeof(int),0))
                {
					if (Verbose>1) {
				     char commande[5];int* p_commande=(int*)commande;
                     commande[4]=0;*p_commande=taskid;
                     printf("procesing : %s ...\n",commande);
					}
                    if(taskid==1953068401)//"quit"
                    {
                        printf("server closed\n");close_server=1;break;
                    }
                    if(taskid==1936682083) {//"clos"
                        printf("command is close\n");break;
                    }
                    switch_task(csock,&set_sol,taskid,Verbose,MatlabPid);
                }
                else 
                {
                    if (Verbose>1) printf("connection closed\n"); break;
                }
            } while(1);
			closesocket(csock);
        } 
        if (Verbose>1) printf("serverloop\n");
    }
    if (Verbose>1) printf("loopexit\n");

    //--------------------------------------------------------------------------//

    closesocket(sock);
    #if defined(WIN32)||defined(WIN64)
     WSACleanup();
    #endif
	if(p_redflux!=NULL)p_redflux->~c_red_flux();

    return 0;
}
 

