#include "c_pardiso.h"
//#include "c_red_flux.h"
#include <vector>
#include <iostream>
using namespace std;
#if defined(MatlabVER)
 #include "mex.h"
#else
 #define mexPrintf printf
#endif 
#ifndef LOCAL
class pardiso_task: public task 
{
public:
	size_t id;
	c_pardiso* p_data;
	void (*pardiso_func)(c_pardiso* p_data_);
    pardiso_task(c_pardiso* p_data_,void (*pardiso_func_)(c_pardiso* p_data_),size_t id_):p_data(p_data_),pardiso_func(pardiso_func_),id(id_){}
	task* execute()
	{ 
		double T_0=clock();
		pardiso_func(p_data);
		double T_1=clock();
		p_data->pardiso_info[id]=((double)(T_1-T_0)/CLOCKS_PER_SEC);
		return NULL;
	};
};
#endif
c_pardiso::c_pardiso() // Initialize new instance of pardiso factor
{
    init_param();
    init_pt(); Mat_A.ia=NULL;Mat_A.ja=NULL;Mat_A.a=NULL;Mat_B.a=NULL;Mat_X.a=NULL;
    Status=0;
	pardiso_info=(double*)calloc(nb_pardiso_info,sizeof(double));
}

c_pardiso::~c_pardiso()
{
    //if(Status&A_SET){ free(Mat_A.ia);Mat_A.ia=NULL; free(Mat_A.ja);Mat_A.ja=NULL;free(Mat_A.a);Mat_A.a=NULL;}
    //if(Status&B_SET){ free(Mat_B.a); Mat_B.a=NULL;free(Mat_X.a);Mat_B.a=NULL;}
    //if(Status&SYM_FACTO_C) release_memory();
	release_memory(); free(pardiso_info);
}

void c_pardiso::init_param()
{
    int i;for(i=0;i<64;i++)iparm[i]=0;
    iparm[0] = 1; /* No solver default */
    iparm[1] = 2; /* Fill-in reordering from METIS */
    iparm[2] = 1; /* Numbers of processors, value of OMP_NUM_THREADS */
    iparm[3] = 0; /* No iterative-direct algorithm */
    iparm[4] = 0; /* No user fill-in reducing permutation */
    iparm[5] = 0; /* Write solution into x */
    iparm[6] = 0; /* Not in use */
    iparm[7] = 2; /* Max numbers of iterative refinement steps */
    iparm[8] = 0; /* Not in use */
    iparm[9] = 13; /* Perturb the pivot elements with 1E-13 */
    iparm[10] = 1; /* Use nonsymmetric permutation and scaling MPS */
    iparm[11] = 0; /* Not in use */
    iparm[12] = 0; /* Maximum weighted matching algorithm is switched-on (default for non-symmetric) */
    iparm[13] = 0; /* Output: Number of perturbed pivots */
    iparm[14] = 0; /* Not in use */
    iparm[15] = 0; /* Not in use */
    iparm[16] = 0; /* Not in use */
    iparm[17] = -1; /* Output: Number of nonzeros in the factor LU */
    iparm[18] = -1; /* Output: Mflops for LU factorization */
    iparm[19] = 0; /* Output: Numbers of CG Iterations */

    maxfct = 1; /* Maximum number of numerical factorizations. */
    mnum = 1; /* Which factorization to use. */
    msglvl = 1; /* Print statistical information in file */
	error=0;
}

/* set parameters */ 
int c_pardiso::set_param(int nb,double* param)
{
	if(nb==2) { /* allow delayed setting of msglvl */
     if (param[0]==66) {msglvl = (MKL_INT)param[1];return 1;}
	 return 0;
	}
    if(Status&SYM_FACTO_A)return 0;
    if(nb==15)
	{
		iparm[1] = (MKL_INT)param[0];
		iparm[2] = (MKL_INT)param[1];
		iparm[3] = (MKL_INT)param[2];
		iparm[4] = (MKL_INT)param[3];
		iparm[7] = (MKL_INT)param[4];
		iparm[9] = (MKL_INT)param[5];
		iparm[10] = (MKL_INT)param[6];
		iparm[12] = (MKL_INT)param[7];
		iparm[13] = (MKL_INT)param[8];
		iparm[17] = (MKL_INT)param[9];
		iparm[18] = (MKL_INT)param[10];
		iparm[19] = (MKL_INT)param[11];
		maxfct = (MKL_INT)param[12];
		mnum = (MKL_INT)param[13];
		msglvl = (MKL_INT)param[14];
	}
	else if(nb>66)
	{
		size_t i;
		for(i=0;i<64;i++)iparm[i] = (MKL_INT)param[i];
		maxfct = (MKL_INT)param[64];
		mnum = (MKL_INT)param[65];
		msglvl = (MKL_INT)param[66];
	}
	else return 0;

	if (msglvl)
	{
		size_t i;
		for(i=0;i<64;i++) mexPrintf("iparm[%d]=%i\n",i,iparm[i]);
		mexPrintf("maxfct=%i\n",maxfct);
		mexPrintf("mnum=%i\n",mnum);
		mexPrintf("msglvl=%i\n",msglvl);
	}
    return 1;
}

void c_pardiso::init_pt()
{
    int i;for(i=0;i<64;i++)pt[i]=0; 
}

void c_pardiso::release_memory()
{
    // .. Termination and release of memory
    MKL_INT phase = -1;
    pardiso(pt,&maxfct,&mnum,&Mat_A.mtype,&phase,
            &Mat_A.m,&Mat_A.a,Mat_A.ia,Mat_A.ja,&idum,&Mat_B.n, // a,ia,ja,perm,nrhs 
            iparm,&msglvl,&Mat_B.a,&Mat_X.a,&error);
	if (Mat_A.ia) {free(Mat_A.ia);Mat_A.ia=NULL; } 
	if (Mat_A.ja) {free(Mat_A.ja);Mat_A.ja=NULL;}
	if (Mat_A.a)  {free(Mat_A.a);Mat_A.a=NULL;}
	if (Mat_B.a) {free(Mat_B.a);Mat_B.a=NULL; } 
	if (Mat_X.a) {free(Mat_X.a);Mat_X.a=NULL; } 

}

void c_pardiso::get_pardiso_info(int& s_data,double*& p_data)
{
	s_data=64+4+3;
	p_data=(double*)malloc(sizeof(double)*s_data);
	size_t i;
	for(i=0;i<64;i++) p_data[i]=iparm[i];
	p_data[64]=maxfct;
	p_data[65]=mnum;
	p_data[66]=msglvl;
	p_data[67]=error;
	p_data[68]=pardiso_info[0];
	p_data[69]=pardiso_info[1];
	p_data[70]=pardiso_info[2];
	if (msglvl) {mexPrintf("Done\n"); }
}

//---------------------------------------------------------------------------//
int c_pardiso::set_matrix_a(int mtype,int m,int n,int complex, 
                             MKL_INT* ia,MKL_INT* ja,double* a)
{
    if(((Status&SYM_FACTO_A)&&(!(Status&SYM_FACTO_C)))||
       ((Status&NUM_FACTO_A)&&(!(Status&NUM_FACTO_C)))||
       ((Status&SOLVE_A)&&(!(Status&SOLVE_C)))) return 0;

    if(Status&A_SET) { } else Status+=A_SET;

    if(Status&SYM_FACTO_C) {
        release_memory(); init_pt();
        Status=Status-SYM_FACTO_A-SYM_FACTO_B-SYM_FACTO_C;
    }

    if(Status&NUM_FACTO_C) Status=Status-NUM_FACTO_A-NUM_FACTO_B-NUM_FACTO_C;
    if(Status&SOLVE_C) Status=Status-SOLVE_A-SOLVE_B-SOLVE_C;

    Mat_A.mtype=mtype;
    Mat_A.m=m;
    Mat_A.n=n;
    Mat_A.complex=complex;
	if (Mat_A.ia!=NULL && Mat_A.ia!=ia) {free(Mat_A.ia);} ; Mat_A.ia=ia;
    if (Mat_A.ja!=NULL && Mat_A.ja!=ja) {free(Mat_A.ja);} ; Mat_A.ja=ja;
    if (Mat_A.a!=NULL && Mat_A.a!=a)    {free(Mat_A.a); } ; Mat_A.a=a;
    
    if ((!(Status&B_SET))||(Mat_B.m!=Mat_A.m)) {
		size_t i3=sizeof(double)*m*(complex?2:1);
		if (i3>0) set_vector_b(m,1,complex,(double*)malloc(i3));
	}
    return 1;
}

//---------------------------------------------------------------------------//

int c_pardiso::set_vector_b(int m,int n,int complex,double* a)
{
    if(((Status&SYM_FACTO_A)&&(!(Status&SYM_FACTO_C)))||
       ((Status&NUM_FACTO_A)&&(!(Status&NUM_FACTO_C)))||
       ((Status&SOLVE_A)&&(!(Status&SOLVE_C))))return 0;

	if(Status&B_SET){} else Status+=B_SET;

    Mat_B.m=m;  Mat_B.n=n;  Mat_B.complex=complex;
	if (Mat_B.a!=NULL && Mat_B.a!=a) { free(Mat_B.a); }; // if changed free old 
	Mat_B.a=a;
    
    Mat_X.m=m; Mat_X.n=n; Mat_X.complex=complex;
    if (Mat_X.a!=NULL) {free(Mat_X.a);}; Mat_X.a=(double*)malloc(sizeof(double)*m*n*(complex?2:1));

    return 1;
}

//---------------------------------------------------------------------------//

int c_pardiso::get_vector_x(int& m,int& n,int& complex,double*& a)
{
	if(Status&SOLVE_C)
	{
		m=Mat_X.m;
		n=Mat_X.n;
		complex=Mat_X.complex;
		a=Mat_X.a;
		return 1;
	}
	return 0;
}

//---------------------------------------------------------------------------//

void _sym_facto(c_pardiso* p_p)
{
    //.. Reordering and Symbolic Factorization. This step also allocates
    // all memory that is necessary for the factorization

    p_p->error = 0;
    MKL_INT phase = 11;/* warning ia here is ja of Matlab */
    pardiso(p_p->pt,&p_p->maxfct,&p_p->mnum,&p_p->Mat_A.mtype,&phase,
            &p_p->Mat_A.m,p_p->Mat_A.a,p_p->Mat_A.ia,p_p->Mat_A.ja,&p_p->idum,&p_p->Mat_B.n,
            p_p->iparm,&p_p->msglvl,&p_p->ddum,&p_p->ddum,&p_p->error);
    
    if(p_p->error!= 0){
        mexPrintf("\nERROR during symbolic factorization: %d",p_p->error);
        #if defined(MatlabVER)
		return;
        #else
		exit(1);
        #endif
    }
	if (p_p->msglvl>1) {
     mexPrintf("\nReordering completed ... ");
     mexPrintf("\nNumber of nonzeros in factors = %d",p_p->iparm[17]);
     mexPrintf("\nNumber of factorization MFLOPS = %d",p_p->iparm[18]);
	}
    p_p->Status+=SYM_FACTO_B;
}

void c_pardiso::sym_facto()
{
    if(!(Status&A_SET))return;

    if(Status&SYM_FACTO_A)return;

    Status+=SYM_FACTO_A;
    //p_thread=new tbb_thread(_sym_facto,this);
	//p_task=new(task::allocate_root())empty_task;p_task->set_ref_count(2);
	//pardiso_task& child_task=*new( p_task->allocate_child())pardiso_task(this,&_sym_facto,0);
	//p_task->spawn(child_task);
    _sym_facto;
}

int c_pardiso::sym_facto_stat()
{
    if(!(Status&SYM_FACTO_A))return 0;

    if(Status&SYM_FACTO_C)return 2;

    //if(Status&SYM_FACTO_B)
  #ifndef LOCAL
    if(p_task->ref_count()==1) {
        //p_thread->join();
        //delete p_thread;
        p_task->decrement_ref_count();
		p_task->destroy(*p_task);
		Status+=SYM_FACTO_C;
        return 2;
    }
  #endif
    return 1;
}

//---------------------------------------------------------------------------//

void _num_facto(c_pardiso* p_p)
{
    // .. Numerical factorization
    
    p_p->error = 0;
    MKL_INT phase = 22;
    pardiso(p_p->pt,&p_p->maxfct,&p_p->mnum,&p_p->Mat_A.mtype,&phase,
            &p_p->Mat_A.m,p_p->Mat_A.a,p_p->Mat_A.ia,p_p->Mat_A.ja,&p_p->idum,&p_p->Mat_B.n,
            p_p->iparm,&p_p->msglvl,&p_p->ddum,&p_p->ddum,&p_p->error);
    
    if(p_p->error != 0){
        mexPrintf("\nERROR during numerical factorization: %d",p_p->error);
        #if defined(MatlabVER)
		return;
        #else
		exit(2);
        #endif
    }
    if (p_p->msglvl>1) mexPrintf("\nFactorization completed ... ");
    p_p->Status+=NUM_FACTO_B;
}

void c_pardiso::num_facto()
{
    if(!(Status&SYM_FACTO_C)) return;
    
    if(Status&NUM_FACTO_A)return;

    Status+=NUM_FACTO_A;
    //p_thread=new tbb_thread(_num_facto,this);
  #ifndef LOCAL
	p_task=new(task::allocate_root())empty_task;p_task->set_ref_count(2);
	pardiso_task& child_task=*new( p_task->allocate_child())pardiso_task(this,&_num_facto,1);
	p_task->spawn(child_task);
  #else
    _num_facto;
  #endif
}

int c_pardiso::num_facto_stat()
{
    if(!(Status&NUM_FACTO_A))return 0;

    if(Status&NUM_FACTO_C)return 2;

    //if(Status&NUM_FACTO_B)
  #ifndef LOCAL
    if(p_task->ref_count()==1) {
        //p_thread->join();
        //delete p_thread;
        p_task->decrement_ref_count();
		p_task->destroy(*p_task);
		Status+=NUM_FACTO_C;
        return 2;
    }
    return 1;
  #endif  
  return 1;
}

//---------------------------------------------------------------------------//

void _solve(c_pardiso* p_p)
{
    //.. Back substitution and iterative refinement
    
    p_p->error = 0;
    MKL_INT phase = 33;
    //if (p_p->Mat_B.a== p_p->Mat_X.a) p_p->iparm[5]=1; else p_p->iparm[5] = 0;// overwrite
    p_p->iparm[5] = 0;
    pardiso(p_p->pt,&p_p->maxfct,&p_p->mnum,&p_p->Mat_A.mtype,&phase,
            &p_p->Mat_A.m,p_p->Mat_A.a,p_p->Mat_A.ia,p_p->Mat_A.ja,&p_p->idum,&p_p->Mat_B.n,
            p_p->iparm,&p_p->msglvl,p_p->Mat_B.a,p_p->Mat_X.a,&p_p->error);
    
    if(p_p->error != 0){
        mexPrintf("\nERROR during solution: %d",p_p->error);
        #if defined(MatlabVER)
		return;
        #else
        exit(3);
        #endif
    }

    p_p->Status+=SOLVE_B;
}

void c_pardiso::solve()
{
    if(!(Status&NUM_FACTO_C))return;

    if((Status&SOLVE_A)&&(!(Status&SOLVE_C)))return;

    if(Status&SOLVE_C){Status-=SOLVE_A;Status-=SOLVE_B;Status-=SOLVE_C;}

    Status+=SOLVE_A;
    //p_thread=new tbb_thread(_solve,this);
  #ifndef LOCAL
    p_task=new(task::allocate_root())empty_task;p_task->set_ref_count(2);
	pardiso_task& child_task=*new( p_task->allocate_child())pardiso_task(this,&_solve,2);
	p_task->spawn(child_task);
 #else
   _solve;
 #endif
}

int c_pardiso::solve_stat()
{
    if(!(Status&SOLVE_A))return 0;

    if(Status&SOLVE_C)return 2;

    //if(Status&SOLVE_B)
 #ifndef LOCAL
    if(p_task->ref_count()==1) {
        //p_thread->join();
        //delete p_thread;
        p_task->decrement_ref_count();
		p_task->destroy(*p_task);
		Status+=SOLVE_C;
        return 2;
    }
 #endif
    return 1;
}
int c_pardiso::get_diag(double* PR)
{          
      pardiso_getdiag(pt,PR,PR+Mat_A.m,&mnum,&error);
      return(error);
}


//---------------------------------------------------------------------------//

