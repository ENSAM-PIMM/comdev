# ln -s ../../../sdtdev/mex50/make.lms-elan .
# export HOST='lms-elan'


CPP=g++
DIROUT=~

MATROOT=/o/MATLAB/M714

include make.$(HOST)
# -DMKL_ILP64 -no-vec
CPPFLAGS = -O2 -DMATLAB_MEX_FILE -ansi -D_GNU_SOURCE -fPIC \
   -fno-omit-frame-pointer -pthread  -DOSTYPEmexa64 -DMatlabVER=714 \
   -DSDT_ADD -DFORMATLAB -DNDEBUG  

LDFLAGS = -L$(TBB_LIBRARY_PATH) -ltbb \
   -Wl,--start-group $(MKL_LIBRARY_PATH)/libmkl_intel_lp64.a \
   $(MKL_LIBRARY_PATH)/libmkl_intel_thread.a \
   $(MKL_LIBRARY_PATH)/libmkl_core.a -Wl,--end-group \
   -L$(OMP_LIBRARY_PATH) -liomp5 \
   -lpthread -lm 
LDLOCAL =  -shared \
 -Wl,--version-script,$(MATROOT)/extern/lib/glnxa64/mexFunction.map \
 -Wl,--no-undefined \
 -L${TBB_LIBRARY_PATH} -ltbb \
 -ldl   -Wl,-rpath-link,$(MATROOT)/bin/glnxa64 -L$(MATROOT)/bin/glnxa64 -lmx -lmex -lmat 

EXE=mklserver_intel_lp_a64.exe

DIRSRCC=../src

CPATH=$(DIRSRCC):$(MKL_INCLUDE_PATH):$(TBB_INCLUDE_PATH)
#LIBRARY_PATH=$(LIBRARY_PATH):$(MKL_LIBRARY_PATH):$(TBB_LIBRARY_PATH)

#SRCC= $(wildcard ./*.cpp)
#mklserv_client is the local mex, mex_client is the 
SRCC=mklserv_client.cpp comutile.cpp  c_pardiso.cpp  main_serv.cpp 

OBJ= $(SRCC:.cpp=.o) #OBI is list of cpp but with .o
OBJ= comutile.o  c_pardiso.o  main_serv.o
OB2= c_pardiso.o mklserv_client.o
#
all:$(EXE) local
#	
$(EXE): $(OBJ)
	$(CPP) -o $(DIROUT)/$@ $^  $(LDFLAGS) $(LDLOCAL)
#		
%.o:$(DIRSRCC)/%.cpp
	$(CPP) -I$(TBB_INCLUDE_PATH) \
    -I$(MATROOT)/extern/include \
    -I$(MKL_INCLUDE_PATH) -I$(DIRSRCC) -o $@ $(CPPFLAGS) -c $<

# clean generate the local mex
local: $(OB2)
	$(CPP) -o mklserv_client.mexa64 $^  $(LDFLAGS) $(LDLOCAL)


#--------------------------------------------------------
clean:
	rm -rf $(OBJ) $(OB2)

rmproper: clean
	rm -rf $(DIROUT)/$(EXE)

#print:
#	@echo $(CPATH)

