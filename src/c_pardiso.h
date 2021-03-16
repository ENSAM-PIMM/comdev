#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mkl_pardiso.h>

//---------------------------------------------------------------------------//
/* PARDISO prototype. */
#if defined(WIN32)||defined(WIN64)||defined(OSTYPEmexw64)
 #define pardiso_ pardiso
 #include <time.h>
#else
 #define pardiso_ pardiso 
 #define pardiso PARDISO
#endif
#if defined(MKL_ILP64)
#define MKL_INT long long
#else
#define MKL_INT int
#endif
/* no longer needed due to mkl_pardiso.h include
 extern "C" MKL_INT PARDISO
    (void *, MKL_INT *, MKL_INT *, MKL_INT *, MKL_INT *, MKL_INT *,
    double *, MKL_INT *, MKL_INT *, MKL_INT *, MKL_INT *, MKL_INT *,
    MKL_INT *, double *, double *, MKL_INT *);*/

//---------------------------------------------------------------------------//
#ifndef LOCAL
 #include "tbb/tbb_thread.h"
 #include "tbb/task.h"
 #include "tbb/task_scheduler_init.h"
 #include "tbb/tick_count.h"
 using namespace tbb;
#endif

#define A_SET 1
#define B_SET 2

#define SYM_FACTO_A 4
#define SYM_FACTO_B 8
#define SYM_FACTO_C 16

#define NUM_FACTO_A 32
#define NUM_FACTO_B 64
#define NUM_FACTO_C 128

#define SOLVE_A 256
#define SOLVE_B 512
#define SOLVE_C 1024

#define nb_pardiso_info 3

struct s_sparse_matrix
{
    MKL_INT mtype;
    MKL_INT m;
    MKL_INT n;
    int complex;
    MKL_INT* ia;
    MKL_INT* ja;
    double* a;
};

struct s_matrix
{
    MKL_INT m;
    MKL_INT n;
    int complex;
    double* a;
};

class c_pardiso
{
public:
    //matrix  
    s_sparse_matrix Mat_A;
    //RHS vectors
    s_matrix Mat_B;
    //solution vectors
    s_matrix Mat_X;
    
    // Internal solver memory pointer pt
    // 32-bit: int pt[64]; 64-bit: long int pt[64]
    // or void *pt[64] should be OK on both architectures
    void* pt[64];

    // Pardiso control parameters    
    MKL_INT iparm[64];
    MKL_INT maxfct, mnum, error, msglvl;
    
    // Auxiliary variables
    double ddum; // Double dummy
    MKL_INT idum; // Integer dummy

    int Status;
    // tbb_thread* p_thread;
	// task* p_task;
	double* pardiso_info;

public:
    c_pardiso();
    ~c_pardiso();
    void init_param();
    void init_pt();
    void release_memory();
    int set_param(int nb,double* param);
    int set_matrix_a(int mtype,int m,int n,int complex,
                     MKL_INT* ia,MKL_INT* ja,double* a);
    int set_vector_b(int m,int n,int complex,double* a);
    int get_vector_x(int& m,int& n,int& complex,double*& a);
	void get_pardiso_info(int& s_data,double*& p_data);
	void sym_facto();
    int sym_facto_stat();
    void num_facto();
    int num_facto_stat();
    void solve();
    int solve_stat();
    int get_diag(double* PR);
};
