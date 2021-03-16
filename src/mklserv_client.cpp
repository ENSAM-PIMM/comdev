#include "mex.h"
#include <iostream>
#ifdef OSTYPEmexa64
#include "string.h"
#endif

//#include "comutile.h"
#include "c_pardiso.h" 
#include <vector>

//#include "c_red_flux.h"

#include <map>
using namespace std;

extern void _sym_facto(c_pardiso* p_p);
void _num_facto(c_pardiso* p_p);
void _solve(c_pardiso* p_p);

void mxCreateInt32(mxArray*& mxval,int val)
{
	mxval=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
    *((int*)mxGetPr(mxval))=val;
	return;
}

void memRIcpy(double* pa,const mxArray* field,int nz, int complex) {
#if MatlabVER >= 904
    double* lpa = (double*)mxGetData(field);
    if (complex) memcpy(pa,lpa,nz * sizeof(double) * 2);
    else memcpy(pa,lpa,nz * sizeof(double));
#else
    double* lpa = mxGetPr(field);
    double* pima = mxGetPi(field);
    if (complex) {
        for (size_t j1 = 0; j1 < nz;j1++) { pa[j1 * 2] = (double)lpa[j1]; pa[j1 * 2 + 1] = (double)pima[j1]; }
    }
    else memcpy(pa,lpa,nz * sizeof(double));
#endif

}
void memRIto(double* pa,const mxArray* field,int nz,int complex) {
#if MatlabVER >= 904
    double* lpa = (double*)mxGetData(field);
    if (complex) memcpy(lpa,pa,nz * sizeof(double) * 2);
    else memcpy(lpa,pa,nz * sizeof(double));
#else
    double* lpa = mxGetPr(field);
    double* pima = mxGetPi(field);
    if (complex) {
        for (size_t j1 = 0; j1 < nz;j1++) { lpa[j1]=pa[j1*2]; pima[j1]= pa[j1*2+1]; }
    }
    else memcpy(pa,lpa,nz * sizeof(double));
#endif

}


//c_red_flux* p_redflux=NULL;

//---------------------------------------------------------------------------//

class c_set_sol
{
private:
    map<int,c_pardiso*>  map_psol;
public:
    long id_active_psol;
    c_set_sol():id_active_psol(-1){};
    ~c_set_sol(){};

    bool create(int id) {
        if(map_psol.find(id)!=map_psol.end())return 0;
        map_psol.insert(make_pair(id,new c_pardiso));
        id_active_psol=id;
        return 1;
    };

    bool erase(int id) {
        map<int,c_pardiso*>::iterator i=map_psol.find(id);
        if(i==map_psol.end())return 0;
        pair<int,c_pardiso*> p=*i;
        delete p.second;
        map_psol.erase(id);
        return 1;
    };
    
    bool set_active(int id) {
        if(map_psol.find(id)==map_psol.end())return 0;
        id_active_psol=id;
        return 1;
    };

    c_pardiso* get_active() {
        if(id_active_psol==-1)return NULL;
        map<int,c_pardiso*>::iterator i=map_psol.find(id_active_psol);
        if(i==map_psol.end())return NULL;
        pair<int,c_pardiso*> p=*i;
        return p.second;
    };

	int get_statistic(vector<int>* p_sta) {
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
c_set_sol set_sol; /* persistent solution holder */
/* --------------------------------------------------------------------*/
void clearData() {
  mexPrintf("Clearing ");
  c_set_sol* p_ss; p_ss=&set_sol; 
  c_pardiso* p_p=p_ss->get_active();
  for (int id=1;id<20;id++) {
	  int b=p_ss->set_active(id);
	  b=p_ss->erase(id);
  }
  mexPrintf(". all factors done \n");
}

//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[] )
{
    char* commande;int* p_commande,p_help[1]={1886152040};
    c_set_sol* p_ss; p_ss=&set_sol; 
    if(nrhs==0) {
        commande=mxArrayToString(mxCreateString("help")); p_commande=p_help;
    } else {
	  commande=mxArrayToString(prhs[0]);
	  p_commande=(mxGetNumberOfElements(prhs[0])==4)?(int*)commande:p_help;
    }
    
	if(nrhs>1) {
        if (mxGetNumberOfElements(prhs[1]) && mxIsNumeric(prhs[1])) {
            int id = (int)*((double*)mxGetPr(prhs[1]));
            int b = p_ss->set_active(id);
            if (b==0) mexErrMsgTxt("Factor does not exist");
        } else if(!mxIsCell(prhs[1])){ mexErrMsgTxt(" expecting cell!\n");
        } else if(mxGetNumberOfElements(prhs[1])!=2){ 
			mexErrMsgTxt("show help !\n");
		}
    } else if(!strcmp("help",commande)) {//((*p_commande)==1886152040)//"help"     
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
      plhs[0]=mxCreateString("$Revision: 157 $  $Date: 2020-11-26 18:32:05 +0100 (Thu, 26 Nov 2020) $");
      return;
    }

    // prhs[1]={'ip',port}  unused here ----------------------------------------------//
    int Verbose=0;plhs[0]=mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
    // Sending p_command -------------------------------------------------// 


 if ((*p_commande) == 1987015539) {
  //"ssov" vector set and solve in place int64(comstr('ssov',-32))
  int mb = (int)mxGetM(prhs[2]); int nb = (int)mxGetN(prhs[2]);int complex = mxIsComplex(prhs[2]);
  c_pardiso* p_p = p_ss->get_active();int b = p_p != NULL;
  if (b) {
     if (p_p[0].Mat_B.a != NULL) { free(p_p[0].Mat_B.a);p_p[0].Mat_B.a = NULL; }
     if (p_p[0].Mat_X.a != NULL) { free(p_p[0].Mat_X.a);p_p[0].Mat_X.a = NULL; }
     p_p[0].Mat_B.m = (int)mb;p_p[0].Mat_B.n = (int)nb;p_p[0].Mat_B.complex = complex;
     p_p[0].Mat_X.m = (int)mb;p_p[0].Mat_X.n = (int)nb;p_p[0].Mat_X.complex = complex;
     if (p_p[0].Mat_B.m!= p_p[0].Mat_A.m) mexErrMsgTxt("size mismatch");
     if (!(p_p->Status&NUM_FACTO_C)) return;
     if ((p_p->Status&SOLVE_A) && (!(p_p->Status&SOLVE_C))) return;
     if (p_p->Status&SOLVE_C) { p_p->Status -= SOLVE_A;p_p->Status -= SOLVE_B;p_p->Status -= SOLVE_C; }
     p_p->Status += SOLVE_A;
     p_p[0].Mat_B.a = (double*)mxGetData(prhs[2]);
     p_p[0].Mat_X.a = (double*)mxGetData(prhs[3]);
     _solve(p_p);
     p_p[0].Mat_B.a = NULL;p_p[0].Mat_X.a = NULL;
     p_p->Status += SOLVE_C;
  }
 } else if ((*p_commande)==1719100782) { //"newf" new factor
        int id=(int)*((double*)mxGetData(prhs[2]));
        int b=p_ss->create(id);
		if (b) {if (Verbose) mexPrintf("id : %i created",id);} else mexPrintf("Error : failed newf\n"); 
        *(int*)mxGetData(plhs[0])=b;
 } else if ((*p_commande)==1718379876) {//"delf"
        int id=(int)*((double*)mxGetData(prhs[2]));
	    int b;
		if (id==-1){ clearData();b=1;
		} else {
	      int b=p_ss->erase(id);
		 if (b) {if (Verbose) mexPrintf("id : %i erased",id);} else mexPrintf("Error : failed delf\n"); 
		}
        *(int*)mxGetData(plhs[0])=b;
    } else if ((*p_commande)==1718904179) {//"setf"
        int id=(int)*((double*)mxGetData(prhs[2]));
        int b=p_ss->set_active(id);
		if (b) {if (Verbose) mexPrintf("id : %i set",id);} else mexPrintf("Error : failed setf\n"); 
        *(int*)mxGetData(plhs[0])=b;
    } else if((*p_commande)==1717662835) { //"staf" factor status	
   	    vector<int> stats;
        int b=p_ss->get_statistic(&stats);
        if (b) {
	 plhs[1]=mxCreateNumericMatrix(2,stats.size()/2,mxINT32_CLASS,mxREAL);
	 memcpy(mxGetData(plhs[1]),&(stats[0]),sizeof(int)*stats.size());
	} else { if (Verbose) mexPrintf("there is no factor ! \n");
            plhs[1]=mxCreateNumericMatrix(1,0,mxINT32_CLASS,mxREAL);
	}
    } else if((*p_commande)==1886676339) {//"setp" set parameters
        size_t nb_param=mxGetM(prhs[2])*mxGetN(prhs[2]);
        double* param=(double*)mxGetData(prhs[2]);
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b) b=p_p->set_param((int)nb_param,param);
		if(b) {if (Verbose) mexPrintf("parameter set\n");} else mexPrintf("Error failed setp\n");
        *(int*)mxGetData(plhs[0])=b;

    } else if((*p_commande)==1635018099) {//"seta"  set matrix
      mexAtExit(clearData);

        size_t ma=mxGetM(prhs[2]);
        size_t na=mxGetN(prhs[2]);
        int complex=mxIsComplex(prhs[2]);

        size_t* lia=(size_t*)mxGetIr(prhs[2]);
        size_t* lja=(size_t*)mxGetJc(prhs[2]);
        double mtype=*((double*)mxGetData(prhs[3]));
		mwSize nz=mxGetNzmax(prhs[2]);
        MKL_INT* ia=(MKL_INT*)calloc(nz,(mwSize)sizeof(MKL_INT));
		double* pa=(double*)calloc((mwSize)nz*(complex ? 2 : 1), (mwSize)sizeof(double));
        memRIcpy(pa,prhs[2],nz,complex);
        MKL_INT* ja=(MKL_INT*)calloc((mwSize)(ma+1),(mwSize)sizeof(MKL_INT));
        for (size_t j1=0;j1<nz;j1++) ia[j1]=(int)lia[j1]+1;
		for (size_t j1=0;j1<ma+1;j1++) ja[j1]=(int)lja[j1]+1;

        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if (b) b=p_p->set_matrix_a((int)mtype,(int)ma,(int)na,complex,ja,ia,pa);
	    if (b) {if (Verbose) mexPrintf("done\n");} else{ mexPrintf(" failed seta\n");}
        *(int*)mxGetData(plhs[0])=b;/*b=1 is set ok*/
    } else if ((*p_commande) == 1651795315) {//"setb" set RHS int64(comstr('setb',-32))
		size_t mb = mxGetM(prhs[2]);
		size_t nb = mxGetN(prhs[2]);
		int complex = mxIsComplex(prhs[2]);
		double* pb = (double*)calloc((mwSize)(mb*nb)*(complex?2:1), (mwSize)sizeof(double));
        memRIcpy(pb,prhs[2],mb*nb,complex);
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if (b) b=p_p->set_vector_b((int)mb,(int)nb,complex,pb);
	    if (b) {if (Verbose) mexPrintf("done\n");} else{ mexPrintf("Failed setb\n");}
        *(int*)mxGetData(plhs[0])=b;
    } else if ((*p_commande)==1717855602) {//"redf" 
        mexPrintf("redirecting stdout and stderr...\n");
	 int b=0;//if(p_redflux==NULL){p_redflux=new c_red_flux;b=p_redflux!=NULL;}
	*(int*)mxGetData(plhs[0])=b;
    } else if ((*p_commande)==1919318387) {//"syfr" 
        if (Verbose) mexPrintf("symbolic factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if (b) {_sym_facto(p_p);}
        else mexPrintf("cannot proces, no factor set\n");
 	    *(int*)mxGetData(plhs[0])=2;
    } else if ((*p_commande)==1667660147) {//"syfc"
        mexPrintf("checking symbolic factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){ mexPrintf("processing...\n");b=p_p->sym_facto_stat();}
        else mexPrintf("cannot proces, no factor set\n");
 	*(int*)mxGetData(plhs[0])=b;
    } else if ((*p_commande)==1919317358) {//"nufr"
        if (Verbose) mexPrintf("numeric factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if (b) {
			p_p->Status+=NUM_FACTO_A;_num_facto(p_p); /* fact started=A,completed B*/
			p_p->Status+=NUM_FACTO_C;/* factorization thread closed */
			// p_p->Mat_A.ia=NULL; p_p->Mat_A.ja=NULL;p_p->Mat_A.a=NULL; /* do not free since matlab memory used */
		}
        else mexPrintf("cannot proces, no factor set\n");
 	    *(int*)mxGetData(plhs[0])=2;
    } else if ((*p_commande)==1667659118) {//"nufc"
        mexPrintf("checking numeric factorization\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if (b){mexPrintf("processing...\n");b=p_p->num_facto_stat();}
        else mexPrintf("cannot proces, no factor set\n");
 	*(int*)mxGetData(plhs[0])=b;
    } else if ((*p_commande)==1919709043) {//"solr"
        if (Verbose) mexPrintf("solve\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
		if(b) {
 		 if(!(p_p->Status&NUM_FACTO_C)) return;
         if((p_p->Status&SOLVE_A)&&(!(p_p->Status&SOLVE_C))) return;
         if(p_p->Status&SOLVE_C){p_p->Status-=SOLVE_A;p_p->Status-=SOLVE_B;p_p->Status-=SOLVE_C;}
         p_p->Status+=SOLVE_A;
         _solve(p_p);
         p_p->Status+=SOLVE_C;
		} else mexPrintf("cannot proces, no factor set\n");
 	    *(int*)mxGetData(plhs[0])=2;
    } else if ((*p_commande)==1668050803) {//"solc"
        if (Verbose) mexPrintf("checking solve\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if(b){ b=p_p->solve_stat(); }
        else mexPrintf("cannot proces, no factor set\n");
 	    *(int*)mxGetData(plhs[0])=b;
    } else if((*p_commande)==2020894055){ //"getx" receive vector    
        int mx=0,nx=0,complex=0;
		double* px = NULL; double* pimx = NULL;

        if (Verbose) mexPrintf("getting solution\n");
        c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
        if (b) {b=p_p->get_vector_x(mx,nx,complex,px);}
        if (b) { plhs[1]=mxCreateDoubleMatrix(mx,nx,complex?mxCOMPLEX:mxREAL);
            memRIto(px,plhs[1],mx*nx,complex);
	    } else mxCreateInt32(plhs[1],-1);
     } else if((*p_commande)==1953853299) { //sout
	 c_pardiso* p_p=p_ss->get_active();int b=0;//int b=(p_p!=NULL)&&(p_redflux!=NULL);
	 if(b){// string out;p_redflux->get_out(&out);
        // if(out.size()!=0) {
		mexPrintf("\n#---------------------------------------------\
                        -----------------------\nbegin deported console\n");
		//mexPrintf("%s\n",out.c_str());
		mexPrintf("end deported console\n#-------------------------\
                        -------------------------------------------\n");
	  //}
         }

 	*(int*)mxGetData(plhs[0])=b;
    } else if ((*p_commande)==875967027){ //"3264"
        *(int*)mxGetData(plhs[0])=sizeof(MKL_INT)*8;
    } else if ((*p_commande)==1734633840) {//"pidg" get Matlab PID from server
        *mxGetPr(plhs[0])=0;
    } else if (!strcmp("quit",commande)) {
    } else if ((*p_commande)==1769235815) { //"geti" get information
	  int s_data;
	  double* p_data;
      if (Verbose) mexPrintf("getting pardiso info...");
	  c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
      p_p->get_pardiso_info(s_data,p_data);

      *(int*)mxGetData(plhs[0])=b;
      if (*((int*)mxGetData(plhs[0]))) {
         plhs[1]=mxCreateDoubleMatrix(1,s_data,mxREAL);
         memcpy(mxGetPr(plhs[1]),p_data,sizeof(double)*s_data);
      } else {
			plhs[1]=mxCreateNumericMatrix(1,0,mxINT32_CLASS,mxREAL);
			mexPrintf("No factor on server \n");
      }
	 } else if(!strcmp("loca",commande)) {  *(int*)mxGetData(plhs[0])=1;
	 } else if(!strcmp("diag",commande)) {       
	  c_pardiso* p_p=p_ss->get_active();int b=p_p!=NULL;
	  if (p_p==NULL) {
		  *(int*)mxGetData(plhs[0])=0;plhs[1]=mxCreateDoubleMatrix(0,0,mxREAL);
		  mexPrintf("No factor on server \n");
	  } else{
       plhs[1]=mxCreateDoubleMatrix(p_p->Mat_A.m,2,mxREAL);
       b=p_p->get_diag(mxGetPr(plhs[1]));*(int*)mxGetData(plhs[0])=1;
	  }
     } else {
		mexPrintf("%s %i unknown !\n",commande,p_commande);
     }
   
    //-----------------------------------------------------------------------//
        
    return;
}
