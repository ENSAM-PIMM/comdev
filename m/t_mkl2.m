function out=t_mkl2(varargin)

% TEST of the mklserv functionality
%
% Windows Visual Studio config
%  Options:Projet:Solutions : repertoires VC++
%  
%  sdtweb('_tracker',588)

 %#ok<*NOSEM,*ASGLU,*NASGU,*UNRCH>
if nargin==0

 sdtweb t_mkl2('lowlevel')
 sdtw('Need to be reviewed')
 return;
 % open ../src/mex_client.cpp
 % t_mkl2('mex')
 %"o:\distrib\lib\mklserv\64\mklserver_intel_lp_w64.exe" 1707
 %"p:\balmes\sdt.cur\test\comdev\\mklserver_intel_lp_w64.exe" 1707
 %P:\balmes\sdt.cur\test\comdev\vcsln\depsol\x64
 %"P:\balmes\sdt.cur\test\comdev\vcsln\depsol\x64\Release\mklserver_intel_lp_w64.exe" 1708 
 % mklserver_intel_lp_w64.exe 1707
 if 1==2 
   r1=mklserv_utils('servlp');
   mklserv_client('geti',r1.Serv)
   mklserv_client('pidg',r1.Serv)
   kd=ofact(speye(12),'method','mklserv_utils');ofact('clear',kd');
   ofact mklserv_utils;d1=fe_eig(femesh('testubeam'));
      
   fprintf('cd %s;make clean;make\n',fullfile(fileparts( ...
       which('t_mkl2')),'..','install'))
 end
else
 [CAM,Cam]=comstr(varargin{1},1);carg=2;
end

if comstr(Cam,'serv')
%% #serv test the mkl_server functionality - - - - - - - - - - - - - - - - - -

if ~exist('t_mkl2','file');cd(fullfile(fileparts(which('t_mkl')),'comdev','m'));end
wd=t_mkl2('path');addpath(wd);cd(wd)
%sdtweb mklserv_utils('defparam')
which mklserv_client

mklserv_utils('servlpstart')

mklserv_utils('defparam')
k=sparse(diag(1:10));b=rand(size(k,1),3);
kd=ofact(k,'method','mklserv_utils');
q=kd\b;
if norm(k*q-b)/norm(b)>1e-10; error('Mismatch');end
ofact('clear',kd)

SE=femesh('testhexa20b');SE.il(3,4)=-3;SE=fe_mknl(SE);
k=SE.K{1}+SE.K{2};b=rand(size(k,1),3);
kd=ofact(k,'method','mklserv_utils','msglvl',1);
q=kd\b;
if norm(k*q-b)/norm(b)>1e-6; error('Mismatch');end
ofact('clear',kd)


% Larger scale test with the meshed cube example
RO.Gen={'div','10 40 100'};
RO.methods={'mklserv_utils','spfmex'};
t_perf

kd=ofact(k,'msglvl',1);q=kd\b;norm(k*q-b);q=kd\b;norm(k*q-b);q=kd\b;norm(k*q-b);

model=demosdt('demoubeam -noplot');
ofact('method mklserv_utils');
def=fe_eig(model,[5 10 0]);

% non symmetric matrices
c=t_mkl2('def');
%ofact('method mklserv_utils');
kd=ofact(c.t11,'method','mklserv_utils','mtype',11);
b=rand(size(kd,1),2);q=kd\b;
r1=[q./(c.t11\b)]-1;
if norm(r1)>sqrt(eps);error('mismatch');end

% xxxLUNES structure pour passage possible 
r1=sp_util('sp2st',c.t13); % retour structure
r1.trans=0; % matrice ou sa transposee
r1.ir=int32(r1.ir); r1.jc=int32(r1.jc);
% field=mxGetField(prhs[1],0,"ir"); mxIsInt32(field)


a=mklserv_utils('serv');serv=a.Serv;
mklserv_client('sout',serv)
status=mklserv_client('sout',serv);
[ret,stat]=mklserv_client('staf',serv);% [IdFact;Status]

load(comgui('cd','/o/sdtdata/AcouFren/ref_test/tgv_unsymFact.mat'))
ofact('method mklserv_utils');
kd=ofact(kd,'mtype',11,'msglvl',1);
u=Tin+T*(kd\(T'*fmu)); % non-sym no ofact

elseif comstr(Cam,'bug1')
%% #Bug1  bug on linux

cd /home/groups/balmes/sdt.cur;sdtcheck path
addpath /home/groups/balmes/sdt.cur/test/comdev/m
cd(fileparts(which('t_mkl2')))
sdtweb t_mkl2('bug1');
load BugPardiso

% Start

setpref('SDT','MklServ',{{'lms-bigsave.paris.ensam.fr',1707}});
setpref('SDT','MklServ',{{'127.0.0.2',1707,'RemoteShell',''}});
%setpref('SDT','MklServ',{{'10.134.16.14',1707,'RemoteShell',''}});% tesla
clear ofact; ofact('method mklserv_utils')
r1=mklserv_utils('serv');disp(r1.Start)

fprintf('Start the server on %s now\n',r1.Serv{1});

eval(cmd)




elseif comstr(Cam,'path')
%% #Path return valid path for environment - - - - - - - - - - - - - - - - - -

out={comgui('cd o:/balmes/sdt.cur/test/comdev/m'), ...
    'p:\balmes\sdt.cur\test\comdev\m', ...
    '\\lms-bigsave\groups\balmes\sdt.cur\test\comdev\m'};

if exist('t_mkl2','file');out{end+1}=fileparts(which('t_mkl2'));end
out=nas2up('firstdir',out);

elseif comstr(Cam,'sync')
%% #sync return valid path for environment - - - - - - - - - - - - - - - - - -
!rsync -rvp /cygdrive/p/balmes/sdt.cur/test/comdev/m/[36]* /cygdrive/o/balmes/sdt.cur/test/comdev/m/
!rsync -rvp /cygdrive/o/balmes/sdt.cur/test/comdev/m/[36]* /cygdrive/p/balmes/sdt.cur/test/comdev/m/

!scp LMS-BIGSAVE:/home/IAL/dev/comdev/trunk/m/64/mkl*_64.exe 64/.
!scp LMS-BIGSAVE:sdt.cur/test/comdev/m/64/mkl*.exe 64/.


%lms-tesla
%include install/make.$(OSTYPE)$(VARIANT)


elseif comstr(Cam,'def');[CAM,Cam]=comstr(CAM,4);
    
ia = [ 1, 1, 1, 1,2, 2, 2,3, 3,4, 4,5, 5, 5,...
		6, 6,7,8 ];
ja = [ 1, 3, 6, 7,2, 3, 5,3, 8,4, 7,5, 6, 7,6, 8,7,8 ];
pr = [ 7.0-1.i, 1.0+5.i, 2.0+8.i, 7.0-10.i,...
		-4.0+3.i, 8.0+8.i, 2.0+3.i,...
		1.0-3.i, 5.0+1.i,...
		7.0+1.i, 9.0-8.i,...
		5.0-1.i, 1.0-2.i, 5.0+4.i,...
		-1.0+8.i, 5.0-1.i,...
		11.0+3.i,...
		5.0-2.i];
b=[1:8]'+[8:-1:1]'*1i;b=[b,b.^2,b.^3];
out=struct('t6',sparse(ia,ja,pr)','b',b); % store TRIL

out.t3=spdiags(spdiags(out.t6,0),0,out.t6+out.t6');
out.t11=sparse([1 0;2 2]); % non sym real
out.t13=sparse([1 0;2 2]*(1+.01i)); % non sym complex
out.t1=real(out.t6);
out.t2=real(out.t3);



elseif comstr(Cam,'lowlevel')
%% -------------------------------------------------------------------

if exist('f:\actu','dir')
  setpref('OpenFEM','MklServPath','F:\actu\com\depor_solv\server\win\server\Release\')
end

if ~exist('t_mkl2','file');cd(fullfile(fileparts(which('t_mkl')),'comdev','m'));end
cd(t_mkl2('path')); c=t_mkl2('def');
if isunix
%         libm.so.6 => /lib64/libm.so.6 (0x0000003931400000)
%         libtbb.so.2 => /opt/intel/composerxe-2011.0.084/tbb/lib/intel64//cc4.1.0_libc2.4_kernel2.6.16.21/libtbb.so.2 (0x00002b7b86ad4000)
%         libiomp5.so => /opt/intel/composerxe-2011.0.084/compiler/lib/intel64/libiomp5.so (0x00002b7b86c13000)
%         libpthread.so.0 => /lib64/libpthread.so.0 (0x0000003931c00000)
%         libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x0000003936400000)
%         libc.so.6 => /lib64/libc.so.6 (0x0000003931000000)
%         /lib64/ld-linux-x86-64.so.2 (0x0000003930c00000)
%         libstdc++.so.6 => /usr/lib64/libstdc++.so.6 (0x0000003921600000)
%         libdl.so.2 => /lib64/libdl.so.2 (0x0000003931800000)
%         librt.so.1 => /lib64/librt.so.1 (0x0000003933c00000)

    
 !echo $LD_LIBRARY_PATH:`pwd`:/n/APP/intel/Compiler/11.0/083/mkl/lib/em64t
!rsync 10.134.16.14:/opt/intel/composerxe-2011.0.084/compiler/lib/intel64/libiomp5.so .
% cd /o/balmes/sdt.cur/test/comdev/m/64
% ldd : lists linked libraries
% export LD_LIBRARY_PATH=`pwd`:/n/APP/intel/Compiler/11.0/083/lib/intel64:/n/APP/intel/Compiler/11.0/083/mkl/lib/em64t:/n/MATLAB/M79/bin/glnxa64
% export LD_LIBRARY_PATH=`pwd`;./mklserver_lp_a64.exe 1707
 
end

%a=mklserv_utils('servilp');serv=a.Serv;dos(a.Start)
a=mklserv_utils('servlp');serv=a.Serv;

if 1==2
    copyfile(fullfile('F:\actu\com\depor_solv\mex_client',['mex_client.' mexext]), ...
    fullfile(t_mkl2('path')))
end


idf=33;
try % verify started or restart
 ret=mklserv_client('newf',serv,idf);
catch
 system(a.Start)
 ret=mklserv_client('newf',serv,idf);
end

ret=mklserv_client('setf',serv,idf);
ret=mklserv_client('setp',serv,mklserv_utils('defparam')); % parametres pardisos

ret=mklserv_client('seta',serv,c.t3,3); %send matrix
%ret=mklserv_client('seta',serv,tril(c.t3),6); %send sym matrix
ret=mklserv_client('syfr',serv);
while 1 % wait till done
    pause(0.5) 
    ret=mklserv_client('syfc',serv);
    if ret == 2
        break
    end
end

ret=mklserv_client('nufr',serv);
while 1
    pause(0.5)
    ret=mklserv_client('nufc',serv);
    if ret == 2
        break
    end
end

ret=mklserv_client('setb',serv,c.b);
ret=mklserv_client('solr',serv);
while 1
    pause(0.5)
    ret=mklserv_client('solc',serv);
    if ret == 2
        break
    end
end

[ret,x]=mklserv_client('getx',serv); %#ok<*ASGLU>

max_dif=max(max(abs(c.t3*x-c.b)))

ret=mklserv_client('delf',serv,idf);
%mklserv_client('quit',serv);
%mklserv_client('help',serv);

elseif comstr(Cam,'mex')
%% #mex compile mex_client pardiso server developped with lunes --------------

 cd(fullfile(t_mkl2('path'),'..','src'))
 if isunix
  %% #unix : compile the mklserv_client (local) no longer mex_client (pipe)
  fprintf('Compile on linux define export HOST=''lms-elan'' or variant \n');
  fprintf('cd /o/balmes/sdt.cur/test/comdev/install;make\n')
  return
 end
 if exist('sd','file');a=sd('mextarget');
 else;a=struct('MEXOPT','-v -DOSTYPEmexa64');
 end
 if ~isempty(strfind(Cam,'debug')); a.MEXOPT=[a.MEXOPT ' -g']; end
 if isunix
     RO.inc=' ';
%      RO.inc=[' -I"/opt/intel/compilers_and_libraries_2017/linux/mkl/include" '...
%       ' -I"/opt/intel/compilers_and_libraries_2017/linux/tbb/include" ' ...
%       ' -I"/opt/intel/clck/2017.1.016/provider/share/common/lib/intel64" '
%       ' -I"/opt/intel/compilers_and_libraries_2017/linux/tbb/lib/intel64/gcc4.7" '...
%       ' -I"/opt/intel/compilers_and_libraries_2017/linux/mkl/lib/intel64" '];
 else
     RO.inc=' -I"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include" -lws2_32 ';
 end
 %% Obsolete call to compile the server/client call mex_client.cpp
 warning('Obsolete server/client call mex_client.cpp, now  t_pardiso(''servCompileLocal'')')
 st=sprintf('mex -output mklserv_client %s %s  mex_client.cpp comutile.cpp ', ...
     a.MEXOPT,RO.inc);
 st=strrep(st,'-R2018a','-R2017b');
 disp(st);eval(st)
 clear(['mklserv_client.',mexext])
 if ~isempty(strfind(mexext,'64'))
  st=sprintf('!mv %s.%s* %s','mklserv_client',mexext, ...
        fullfile(pwd,'..','m','.'));
  disp(st);eval(st);
  if ~isunix
   ! cp -v p:\balmes\sdt.cur\test\comdev\m\mklserv_client.mexw64 o:\balmes\sdt.cur\test\comdev\m\mklserv_client.mexw64
  end
  
 else
 end
 return
 
% xxx obsolete ----------------------------------------------------

r1=t_mkl('tbbpath')
eval(sprintf('mex -DOSTYPE%s -v -c %s -I"%s"   %s', ...
    mexext,r1.tbb,pwd,'c_pardiso.cpp comutile.cpp main_serv.cpp')) 

RO.ClOpt=['/O2 /Oi /GL /D "WIN64" /D "NDEBUG" /D "_CONSOLE" ' ...
    '/D "_CRT_SECURE_NO_WARNINGS" /D "_UNICODE" /D "UNICODE" ' ...
    '/FD /EHsc /MD /Gy /Fo"x64\Release\\" /Fd"x64\Release\vc90.pdb" ' ...
    '/W3 /nologo /c /Zi /TP /errorReport:prompt']

RO.LnkOpt=['/OUT:"P:\balmes\sdt.cur\test\comdev\vcsln\depsol\x64\' ...
    'Release\mklserver_intel_lp_w64.exe" /INCREMENTAL:NO ' ...
    '/NOLOGO /MANIFEST ' ...
    '/MANIFESTFILE:"x64\Release\mklserver_intel_lp_w64.exe.intermediate.manifest"' ...
    '/MANIFESTUAC:"level=''asInvoker'' uiAccess=''false''"' ...
    '/DEBUG /PDB:"p:\balmes\sdt.cur\test\comdev\vcsln\depsol\x64\Release\mklserver_intel_lp_w64.pdb" ' ...
    '/SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF /LTCG /DYNAMICBASE /NXCOMPAT ' ...
    '/MACHINE:X64 /ERRORREPORT:PROMPT ' ...
    'mkl_core.lib libiomp5md.lib mkl_intel_lp64.lib ' ...
    'mkl_intel_thread.lib Ws2_32.lib  kernel32.lib user32.lib ' ...
    'gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ' ...
    'ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib']


st=sprintf('cl -DOSTYPE%s /O2 /Oi /GL -I"%s" -I"%s" %s',mexext, ...
    r1.TbbInc,r1.MklInc, ...
    ['/D "NDEBUG" /D "_CONSOLE" /D "_WIN" /D "_UNICODE" ' ...
     '/D "UNICODE" /FD /EHsc /MD /Gy /Fo"Release\\"' ...
     '/Fd"Release\vc90.pdb" /W3 /nologo /c /Zi /TP /errorReport:prompt']);

 
%  '/O2 /Oi /GL ' 
%   /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_WIN" 
%   /D "_UNICODE" /D "UNICODE" /FD /EHsc /MD /Gy 
%   /Fo"x64\Release\\" /Fd"x64\Release\vc90.pdb" 
%   /W3 /nologo /c /Zi /TP /errorReport:prompt
 
cd "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"
%vcvarsall.bat  amd64
disp([st ' c_pardiso.cpp comutile.cpp main_serv.cpp']);

r1.lib64=['tbb.lib ws2_32.lib libiomp5md.lib mkl_intel_thread.lib ' ...
    'mkl_core.lib mkl_intel_lp64.lib  kernel32.lib user32.lib ' ...
    'gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ' ...
    'ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib'];

% % cl  -DOSTYPEmexw32 /c /Zp8 /GR /W3 /EHs 
% /D_CRT_SECURE_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE 
% /D_SECURE_SCL=0 /DMATLAB_MEX_FILE /nologo /MD 
% /FoC:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\comstr.obj 
% -ID:\APP\M710\extern\include /O2 /Oy- /DNDEBUG -DMX_COMPAT_32 comstr.c 
% 
% %--> link /out:"comstr.mexw64" /dll /export:mexFunction 
% /LIBPATH:"D:\APP\M710\extern\lib\win64\microsoft" 
% libmx.lib libmex.lib libmat.lib 
% /MACHINE:X64 kernel32.lib user32.lib gdi32.lib winspool.lib 
% comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib 
% uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:NO 
% /implib:"C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.x" 
% /MAP:"comstr.mexw64.map"  
% @C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\MEX_TMP.RSP   
 
%--> del "C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.x" "C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.exp" 
 
%--> mt -outputresource:"comstr.mexw64;2" -manifest "comstr.mexw64.manifest" 
 
%    Cr�ation de la biblioth�que C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.x et de l'objet C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.exp 
%  
% --> del "C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.x" "C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_US~1\templib.exp" 
%  
%  
% --> mt -outputresource:"comstr.mexw64;2" -manifest "comstr.mexw64.manifest" 

sprintf(['link /OUT:"sever.exe" /INCREMENTAL:NO /NOLOGO /LIBPATH:"%s" ' ...
   '/LIBPATH:"%s" /MANIFEST /MANIFESTFILE:"server.exe.intermediate.manifest" ' ...
  '/MANIFESTUAC:"level=''asInvoker'' uiAccess=''false''" ' ...
  '/DEBUG ' ...%/PDB:"server.pdb" 
  '/SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF /LTCG /DYNAMICBASE ' ...
  '/NXCOMPAT /MACHINE:X64 '  ...
  '/ERRORREPORT:PROMPT ' ...
  'tbb.lib ws2_32.lib libiomp5md.lib mkl_intel_thread.lib ' ...
  'mkl_core.lib mkl_intel_c.lib  kernel32.lib user32.lib gdi32.lib ' ...
  'winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib ' ...
  'oleaut32.lib uuid.lib odbc32.lib odbccp32.lib'    ...
   ],r1.MklLib,r1.TbbLib)


%manifest /nologo /outputresource:".\Release\server.exe;#1"


%!link /out:"main_serv.exe" /LIBPATH:"D:\APP\M710\extern\lib\win64\microsoft"  /MACHINE:X64 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:NO /implib:"C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_HX~1\templib.x" /MAP:"mex_client.mexw64.map"  
%@C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_HX~1\MEX_TMP.RSP  "C:\Program Files\Microsoft SDKs\Windows\v6.0A\LIB\x64\ws2_32.lib"  
 
 %Cr�ation de la biblioth�que C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_HX~1\templib.x et de l'objet C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_HX~1\templib.exp 
 
%--> del "C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_HX~1\templib.x" "C:\USERS\BALMES\APPDATA\LOCAL\TEMP\MEX_HX~1\templib.exp" 
 
 
%--> mt -outputresource:"mex_client.mexw64;2" -manifest "mex_client.mexw64.manifest" 

%Librairie :
% ws2_32.lib +
% tbb.lib +
% mkl_core.lib mkl_intel_thread.lib libiomp5md.lib +
% mkl_intel_ilpl64.lib (compilation sous 64 entier 64) ou 
% mkl_intel_lp64.lib (compilation sous 64 entier 32) ou 
% mkl_intel.lib (compilation sous 32)

  
% les seules chose � modifier sont :
%  
% 1- les path mkl et tbb pour le server dans le makefile / solution visual studio.
%  
% 2- pour entiers 64 sous 64 : link avec la lib libmkl_intel_ilp64 + mot cl� pr�pro MKL_ILP64 � la compilation
%     pour entiers 32 sous 64 : link avec la lib libmkl_intel_lp64
%     sour 32 : link avec la lib libmkl_intel_c
%  
% n'hesite pas a me faire signes en cas de pb.
% lounes.
% 
%% #distrib ->sdtweb t_mkl('distrib')
elseif comstr(Cam,'distrib')
 wd=fileparts(which('t_mkl2'));
 pw0=pwd;cd(wd);li=dir('mklserv_*');li={li.name};li=li(:);
 for j1=1:length(li);li{j1,2}=fullfile(wd,li{j1,1});end
 out=li;cd(pw0);
end
