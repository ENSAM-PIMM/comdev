#pragma once

#if defined(WIN32)||defined(WIN64)
#include <windows.h>
#include <process.h> 
#include <stddef.h>
#include <conio.h>
#include <signal.h>
#else
#include<signal.h>
#include<pthread.h>
#endif

#if defined(WIN32)||defined(WIN64)
#include <tchar.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <time.h>
#include <math.h>
#include <algorithm>
using namespace std;

class c_red_flux {

private:

	int id_stdout_file;
	FILE* stdout_file;
	string stdout_file_name[2];

public:

	c_red_flux()
	{
		ostringstream o_stdout_file_name;o_stdout_file_name<<"stdout"<<"_"<<getpid();
		stdout_file_name[0]=o_stdout_file_name.str()+"_0";
		stdout_file_name[1]=o_stdout_file_name.str()+"_1";
		id_stdout_file=0;
		stdout_file=freopen(stdout_file_name[id_stdout_file].c_str(),"w",stdout);
		if(stdout_file==NULL){perror("stdout redirection failure\n" );exit(1);}
	};
	
	void get_out(string* p_out)
	{
		id_stdout_file=!id_stdout_file;
		stdout_file=freopen(stdout_file_name[id_stdout_file].c_str(),"w",stdout);
		if(stdout_file==NULL){perror("stdout redirection failure\n" );exit(1);}
		FILE* stdout_file_old=fopen(stdout_file_name[!id_stdout_file].c_str(),"r");
		if(stdout_file_old==NULL){perror("stdout file open failure\n" );exit(1);}
		char* buffer[255];
		do
		{
			size_t s_read=fread(buffer,1,255,stdout_file_old);
			if(s_read==0)break;
			p_out->append((char*)buffer,s_read);
		}while(1);
		p_out->append("\n\0");
		fclose(stdout_file_old);
	}

    ~c_red_flux()
    {
		freopen("CON","w",stdout);
	};
};