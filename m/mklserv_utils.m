function out=mklserv_utils(varargin);


% Gateway function for inclusion of MKL server into the 
% OpenFEM ofact object. Install using sdtcheck(''patchMkl'')
%
%  mklserv_utils('defparam') % returns default parameters 
% set default IP and port
%     a=mklserv_utils('serv');setpref('SDT','MklServ',a.Serv) 
%
%   ki=ofact(kd,'method','mklserv_utils','mtype',11);
%   oProp={'method','mklserv_utils','mtype',13,'silent',1};kd=ofact(speye(3)*(1+1i),oProp{:})
%    mklserv_utils('mtype') % for type list
% - For large RHS buffering use 
%    setpref('SDT','MklServBufSize',1) % can be used for B buffering in GB

% - To close your server : mklserv_utils('clear')
% - Low level check of start options (without starting)
%    a=mklserv_utils('servBack')
%   !netstat -tuplen | grep 1707
%   mklserv_client('pidg',{'127.0.0.1'    [1707]})
%   mklserv_client('loca',{0,0}) % Recent test that mex is local

% Etienne Balmes, Illoul Amran, Guillaume Vermot des Roches
% Copyright (c) 2010-2024 by SDTools and ENSAM
% For revision information see mklserv_utils('cvs')

%#ok<*NOSEM,*NASGU,*ASGLU>
persistent Factors CheckedServ isLocal
if isempty(isLocal);
 try; mklserv_client('cvs');
 catch;sdtcheck('syspathmex');
 end
 isLocal=mklserv_client('loca',{0,0});
 if isLocal; CheckedServ=1; end
end

if nargin>0;[CAM,Cam]=comstr(varargin{1},1);carg=2;
else;
 if isLocal; fprintf('mklserv local client in use\n');
 else
  i1=mklserv_utils('servCheck');
  if i1==-1;st1='Not started';
  else; st1=sprintf('MatlabPID: %i',i1);
  end
  fprintf('mklserver IP: %s port: %i %s\n\n',CheckedServ{:},st1);
 end
 return;
end
%% #Method  ------------------------------------------------------------------
if comstr(Cam,'method') % This is where known methods are defined

   ks.name='mklserv_utils';
   ks.header='MKL/PARDISO sparse solver';
   ks.SymRenumber='';
                                               %msg CPU
   ks.FactBuild=@mklserv_fact;
   %'ks=pardiso_utils(''fact'',k,ks,[0 1]);';
   ks.Solve=@mklserv_solve; %'q=pardiso_utils(''solve'',k,full(b));';
   ks.Clear=@mklserv_clear;
   ks.Available=exist('mklserv_client','file')==3;
   ks.HandlesComplex=1;
   [i1,r1]=sdtdef('in','MklServ');
   if i1 %ispref('SDT','MklServ'); r1=getpref('SDT','MklServ');
    if iscell(r1)&&length(r1)>1;r1=struct('Serv',{r1});
    else; r1=struct('Serv',r1);
    end
   else
    r1=mklserv_utils('serv');
    if iscell(r1.Serv)&&~isLocal
     fprintf('Undefined setpref(''SDT'',''MklServ'',%s {''%s'',%i}\n', ...
      '{''IP'',port}) using' ,r1.Serv{:});
    end
   end
   ks.param=struct('Server',{r1.Serv},...
       'param',mklserv_utils('defparam'),'MoveFromCaller',1);
   carg=3;
   if ~isempty(strfind(Cam,'silent'));ks.param.param(67)=0;end
   while carg<=nargin
    if comstr(varargin{carg},'msglvl')
     ks.param.param(67)=varargin{carg+1};
    elseif ismember(varargin{carg},fieldnames(ks))
     ks.(varargin{carg})=varargin{carg+1};
    end
    carg=carg+2;
   end
   out=ks;
   i1=0; 
   try; i1=exist('sdtkey','file')&&sdtkey('cvsnum','mklserv_client')<78;
   catch; i1=1;end
   if i1
       error('Your revision of ''%s'' is too old',which('mklserv_client'));
   end
  %serv={'10.134.16.14',23};
   
%% #NumFact This is where the factorization is actually - - - - - -- - - - - -
elseif comstr(Cam,'numfact')||comstr(Cam,'symbfact')||comstr(Cam,'fact')

 k=varargin{carg};carg=carg+1;
 ks=varargin{carg};carg=carg+1; 
 sdtweb('_link','sdtweb mklserv_utils(''fact'')','Open Fact code');
 error('not reimplemented in this context');

elseif comstr(Cam,'solve'); error('not reimplemented in this context');
%% #NewFact - - - - - - - - - - - - - - - - - - - - -
elseif comstr(Cam,'newfact')  
    
 try
   [ret,i2]=mklserv_client('staf',varargin{2});Factors=i2(1,:);
 end
 if isempty(Factors);Factors=1;else;Factors(1,end+1)=max(Factors)+1;end
 out=Factors(end);
   
%% #Clear - - - - - - - - - - - - - - - - - - - - -
elseif comstr(Cam,'clear')  % - - - - - - - - - - - - - - - - - - - - - -

 if nargin<3;serv=CheckedServ;
 else
  method=varargin{3};
  if ~isfield(method,'param')||~isfield(method.param,'Server')
   r1=mklserv_utils('serv');serv=r1.Serv;
  else; serv=method.param.Server;
  end
 end
 if nargin>=2&&~isequal(varargin{2},-1)
  i1=varargin{2};
  Factors(ismember(Factors,i1))=[];
 else % clear all factors
  i1=Factors;Factors=[];
  if isLocal; clear mklserv_client; return; end
 end
 if isequal(i1,[])&&isempty(Factors) % close server
  if ~isempty(CheckedServ)
   mklserv_client('quit',serv);fprintf('Closed server\n');
   CheckedServ=[]; Factors=[];
  end
  try;go=findall(sdtdef('rwarn'),'tag','mklserv_utils');
   if ~isempty(go); go(go==gcbo)=[];end
   if ~isempty(go); set(go,'DeleteFcn','');delete(go);end
  end
 else % clear present factors
  [ret,i2]=mklserv_client('staf',serv);
  if ret==-1;return;end
  i1=intersect(i1,i2(1,:));
  for j1=1:length(i1);
   ret=mklserv_client('delf',serv,double(i1(j1)));
  end
 end
 return

elseif comstr(Cam,'serv');[CAM,Cam]=comstr(CAM,5);
%% #Serv start server - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% "sdt\mklserv\64\mklserver_intel_lp_w64.exe" Port Verbose(0) Pid &
%  in matlab pid=feature('getpid')
% Pid used to interogate all servers to see if one matches current Matlab PID
  %isLocal=mklserv_client('loca',{0,0});
  wd=mklserv_utils('path');
  out=struct;
  if ~isempty(CheckedServ);out.Serv=CheckedServ;
  elseif sdtdef('in','MklServ') %ispref('SDT','MklServ')
   r1=sdtdef('MklServ'); %getpref('SDT','MklServ');
   if isempty(r1)
    %sdtweb('_link','rmpref(''SDT'',''MklServ'');','Remove empty pref');
    sdtweb('_link','sdtdef(''rm'',''MklServ'');','Remove empty pref');
    error('Preference SDT,MklServ exist but is empty. See link above to remove')
   end
   if iscell(r1)&&length(r1)>1;r1=struct('Serv',{r1});
   else; r1=struct('Serv',r1);
   end
   % RemoteUserHost, RemoteShell
   if length(r1.Serv)>2; r2=struct(r1.Serv{3:end});
    st=fields(r2);for j1=1:length(st);out.(st{j1})=r2.(st{j1});end
    r1.Serv(3:end)=[];
   end
   out.Serv=r1.Serv;
  else % recover IP now
   st=sdtcheck('HostIP'); out.Serv={st 1707};
  end
%   elseif isunix % xxx /sbin/ifconfig
%     [i1,st]=system(sprintf('ping -c1 %s',getenv('HOSTNAME')));
%     st(1:find(st=='(',1,'first'))=''; st=textscan(st,'%s',1);
%     st=st{1}{1};st(end)=''; out.Serv={st 1707};
%   else
%     [i1,st]=system('ipconfig');
%     i1=strfind(st,'IPv4');
%     if ~isempty(i1)
%      st(1:strfind(st,'IPv4'))='';st(1:find(st==':',1,'first'))='';
%     else
%      st(1:strfind(st,'IP Address'))='';st(1:find(st==':',1,'first'))='';
%     end
%     st=textscan(st,'%s',1);st=st{1};
%     out.Serv={st{1} 1707};
%   end
  % server names mklserver_(intel/amd)_(lp/ilp)_(w32/w64).exe
  %  lp (32bit pointer) ilp (64 bit pointer)
  st1='';
%   if strcmpi(getenv('CPU'),'x64');out.Arch='intel';
%   elseif ~isempty(strfind(lower(getenv('PROCESSOR_IDENTIFIER')),'intel'))
%       out.Arch='intel';
%   else;out.Arch='amd';
%   end
  out.Arch=sdtcheck('cpuvendor');
  out.wd=wd; i1=feature('getpid'); 
  if isLocal
  elseif strcmpi(mexext,'mexw32')
   out.Exe='mklserver_lp_w32.exe';
   out.Start={'%s"%s" %i 0 %i &',st1,st,out.Serv{2},i1};
  elseif strcmpi(mexext,'mexw64')
   out.Exe=sprintf('mklserver_%s_lp_%s.exe',out.Arch,comstr(mexext,4));
   st=fullfile(wd,out.Exe);
   out.Start={'%s"%s" %i 0 %i &',st1,st,out.Serv{2},i1};
  else
   out.Exe=sprintf('mklserver_%s_lp_%s.exe',out.Arch,comstr(mexext,4));
   if ~exist(fullfile(wd,out.Exe),'file');out.Arch='intel';
    out.Exe=sprintf('mklserver_%s_lp_%s.exe',out.Arch,comstr(mexext,4));
   end
   st1=sprintf('cd "%s";export LD_LIBRARY_PATH="%s";',wd,wd);
   st=out.Exe;out.Start={'%s./%s %i 0 %i &',st1,st,out.Serv{2},i1};
  end
  if comstr(Cam,'ilp')&&~isempty(strfind(mexext,'64')); 
      out.Start=strrep(out.Start,'_lp','_ilp');
  end
  % Now really do start - - - - - - - - - - - - - - - - - -
  st='';
  if isLocal;CheckedServ=1;out.Start={''};out.Serv={'local',0};
  elseif isempty(strfind(Cam,'start'));st='ok';
  elseif isfield(out,'RemoteShell')
     if isempty(out.RemoteShell);out.RemoteShell='ssh';end
     if ~isfield(out,'RemoteUserHost')
       out.RemoteUserHost=sprintf('%s@%s',sdtcheck('user'),out.Serv{1});
     end
     warning('Automated remote server start with \n%s', ...
         sprintf('%s %s %s ""%s %i &',out.RemoteShell,out.RemoteUserHost, ...
         st1,st,out.Serv{2}))
  elseif isunix
     [i1,st]=system(sprintf('ps -ef | grep mklserv'));
     i1=strfind(st,out.Exe);
     if isempty(i1);st='';
     %else; out.Serv{2}=sscanf(st(i1+length(out.Exe):end),'%i',1);
     end
     %[i1,st]=system(sprintf('ps -lu -h -C %s',out.Exe))
  else
     [i1,st]=system(sprintf('tasklist /v | find "%s"',out.Exe(1:20))); 
  end
  if ~isempty(st) % If Mkl_serv found check that available
      % i1= mklserv_client('pidg',{'127.0.0.1' ,[1707]}) ;
      i1=mklserv_client('pidg',out.Serv);
      if i1~=feature('getpid');CheckedServ=[];end
  end
  if ~isLocal&&(isempty(CheckedServ)||comstr(Cam,'check'))% Check version
    fid=fullfile(out.wd,out.Exe);
    if ~exist(fid,'file');error('File %s not found',fid);end
    fid=fopen(fid,'r');
    st=fread(fid,'char=>char')';fclose(fid);
    i1=sscanf(st(findstr(st,'$Revision')+[10:100]),'%i',1);
    if i1<77; error('Incorrect revision');end
     try 
          i1=mklserv_client('pidg',out.Serv);% likely lock-up
          while i1~=feature('getpid')&&i1>0
            out.Serv{2}=out.Serv{2}+1;out.Start{end-1}=out.Serv{2};
            i1=mklserv_client('pidg',out.Serv);% -1 Failed to connect is good here
          end
          if i1>0;st=sprintf('ok %i',i1);else;st='';end
          CheckedServ=out.Serv;
     catch;error('bug please report')
     end
  end
  out.Start=sprintf(out.Start{:});
   
  if comstr(Cam,'check'); out=i1; % Check server running
  elseif comstr(Cam,'back'); % Return start options data
  elseif isempty(st)&&~isLocal;
     disp(out.Start);
     if isunix;
         [i1,st3]=system(out.Start);if ~isempty(st3)||i1;error(st3);end
         i1=mklserv_client('pidg',out.Serv);
     else;system(out.Start);
     end
     ki=ofact(speye(10),'method','mklserv_utils','mtype',11);
     q=ki\ones(10,1);ofact('clear',ki);
     gf=sdtdef('rwarn');
     go=uicontrol('parent',gf,'visible','off','tag','mklserv_utils', ...
         'deletefcn','mklserv_utils(''clear'');mklserv_utils(''clear'') ');
     out.Serv=CheckedServ; % /!\ May have been updated in ofact call !!
  end % Really start
  
elseif comstr(Cam,'mtype')  
%% #mtype - - - - - - - - - - - - - - - - - - - - - -
    
out={1 'real structurally symmetric'
 2 'real TRIL positive definite'
-2 'real TRIL indefinite'
 3 'complex structurally symmetric'
 4 'complex and Hermitian positive definite'
-4 'complex and Hermitian indefinite'
 6 'complex TRIL'
11 'real and unsymmetric'
13 'complex and unsymmetric'};
   
elseif comstr(Cam,'path')
 %% #Path: recover path to libraries
 [i1,wd]=sdtdef('in','MklServPath'); %if ispref('SDT','MklServPath'); wd=getpref('SDT','MklServPath');
 if isLocal; 
 elseif isempty(wd)||~exist(wd,'dir'); %else
  wd=fileparts(which('mklserv_utils.m'));% SDT root
  if exist(fullfile(wd,'mklserv'),'dir');wd=fullfile(wd,'mklserv');
  elseif exist(fullfile(sdtcheck('SDTRootDir'),'mklserv'),'dir')
   wd=fullfile(sdtcheck('SDTRootDir'),'mklserv');
  else; wd='';
  end
  if ~isempty(wd) % found in SDT path or pref
   if any(strcmp(wd(end+(-1:0)),{'32','64'}));
   elseif ~isempty(strfind(mexext,'32'));wd=fullfile(wd,'32');
   else;wd=fullfile(wd,'64');
   end
  else % Could not find, try to check LD_LIBRARY_PATH
   r1=getenv('LD_LIBRARY_PATH');
   if ~isempty(r1)
    r1=textscan(r1,'%s','delimiter',':'); r1=r1{1};
    r1(cellfun(@(x)isempty(strfind(x,'mklserv')),r1))=[];
    try; wd=nas2up('firstdir',r1); catch; wd=''; end
   end
  end
 end
 out=wd;

elseif comstr(Cam,'silent')  % - - - - - - - - - - - - - - - - - - - - - -
 %% #Silent
 [CAM,Cam]=comstr(CAM,7);
 ks=varargin{carg}; carg=carg+1;
 out=varargin{carg}; carg=carg+1;
 if carg<=nargin; opt=varargin{carg};
 elseif ~isempty(CAM); opt=comstr(CAM,-1);
 else; opt=0; 
 end
 out.param.param(67)=opt;

%% #DefParam  - - - - - - - - - - - - - - - - - - - - - -
elseif comstr(Cam,'defparam')  
    
 %{'maxfct(1#%g#"Max fact")'}
 r1=[];
 if carg<=nargin
   param=varargin{carg};carg=carg+1;
   r1=struct;for j1=1:64;r1.(sprintf('i%i',j1))=param(j1);end
   st={'maxfact','mnum','msglvl','error','t1','t2','t3'};
   for j2=1:length(st);r1.(st{j2})=param(64+j2);end
 end
a=['i1(0#%g#"UseDef") i2(2#%g#"MMD(1),Metis,OMPMMD")' ...
  'i3(0#%g#"") i4(0#%g#"CGS")' ...
  'i5(0#%g#"Reuse fill-in") i6(0#%g#"Overwrite F")' ...
  'i7(0#%g#"Niter_out") i8(0#%g#"Refine_Step")' ...
  'i9(0#%g#"Unused") i10(13#%g#"Pivoting sym 13 nsym 8")' ...
  'i11(0#%g#"scale : 1 ty 11,13 0:ty -2 -4 6") i12(0#%g#"")' ...
  'i13(0#%g#"weigth: 1 ty 11,13 0:ty -2 -4 6") i14(0#%g#"N perturbed pivots")' ...
  'i15(0#%g#"peak symbfact") i16(0#%g#"perm symbfact")' ...
  'i17(0#%g#"mem fact") i18(-1#%g#"nnz_fact")' ...
  'i19(-1#%g#"report flops") i20(0#%g#"CG diag")' ...
  'i21(0#%g#"pivot 0 diag, 1 BK") i22(0#%g#"pos eig")' ...
  'i23(0#%g#"neg eig") i24(0#%g#"")' ...
  'i27(0#%g#"Check Sort") i28(0#%g#"1=single_prec")' ...
  'i30(0#%g#"N zero failed pivot")' ...
  'i60(0#%g#"0 IC, 1 OOC>Mem, 2 OOC") i61(0#%g#"Peak MB")' ...
  'i62(0#%g#"Peak DP Fact") i63(0#%g#"Peak DP Solve")' ...
  'maxfact(1#%g#"Number of factors i65") mnum(1#%g#"UseFactor i")' ...
  'msglvl(1#%g#"MsgLvl") error(0#%g#"Error")' ...
  't1(0#%g#"T_sym_fact") t2(0#%g#"T_num_fact") t3(0#%g#"T_solve")' ...
  ];
 if isempty(r1);b=cingui('paramedit -DoClean',a);
 else;b=cingui('paramedit',a,r1);
  b=fe_def('cleanentry cell',b);b(cellfun('isempty',b(:,3)),:)=[];
  b{end-2,2}=b{end-2,2}*1000;b{end-1,2}=b{end-1,2}*1000;b{end,2}=b{end,2}*1000;
  b=b';fprintf('%8s %4.0f :  %s\n',b{:})
  return
 end
 
 st=fieldnames(b);param=zeros(71,1);
 i2=find(strcmpi(st,'maxfact'));
 for j1=1:length(st)
  if j1<i2;r1=eval(st{j1}(2:end));param(r1)=b.(st{j1});
  else;param(65+j1-i2)=b.(st{j1});
  end
 end
out=param;
%maxfct = 1;     /* Maximum number of numerical factorizations. */
%mnum = 1;       /* Which factorization to use. */
%msglvl = 0;     /* Print statistical information in file */
%error = 0;      /* Initialize error flag */

elseif comstr(Cam,'msglvl')
  kd=varargin{2};
  if ~isempty(kd.method); error('Not implemented');end
%% #Setup
elseif comstr(Cam,'setup')  % - - - - - - - - - - - - - - - - - - - - - -
  r1=mklserv_utils('serv');
  [i2,r2]=sdtdef('in','MklServPath');
  if i2 %ispref('SDT','MklServPath'); 
    fprintf('Found getpref(''SDT'',''MklServPath'',''%s'')', r2); 
    %sdtweb('_link','rmpref(''SDT'',''MklServPath'')','Remove')
    sdtweb('_link','sdtdef(''rm'',''MklServPath'')','Remove')
  else; fprintf('Empty getpref(''SDT'',''MklServPath''), using %s\n',r1.wd);      
  end
  [i1,r1]=sdtdef('in','MklServ');
  if i1 %ispref('SDT','MklServ')
    fprintf('Found getpref(''SDT'',''MklServ'')');
    %sdtweb('_link','rmpref(''SDT'',''MklServ'')','Remove')
    sdtweb('_link','sdtdef(''rm'',''MklServ'')','Remove')
    disp(r1);
  else
    fprintf('Empty getpref(''SDT'',''MklServ''), using %s\n', ...
        strutil(r1.Serv,-30));      
  end

%% #oprop
elseif comstr(Cam,'oprop');[CAM,Cam]=comstr(CAM,6);
 out={ ...
 'RealSym',{'method','mklserv_utils','mtype',1,'silent',1}
 'RealPosSym',{'method','mklserv_utils','mtype',2,'silent',1}
 'RealUndefSym',{'method','mklserv_utils','mtype',-2,'silent',1}
 'RealNonsym',{'method','mklserv_utils','mtype',11,'silent',1}
 'CpxSym',{'method','mklserv_utils','mtype',6,'silent',1}
 'CpxNonSym',{'method','mklserv_utils','mtype',13,'silent',1}
     };
 if isempty(Cam)&&carg<=nargin&&ischar(varargin{carg});
     CAM=varargin{carg};carg=carg+1;Cam=CAM;
 end
 if ~isempty(Cam);out=out{strcmpi(out(:,1),CAM),2};end
%% #cvs
elseif comstr(Cam,'cvs')
 out=sdtcheck('revision','$Revision: 86c7445 $  $Date: 2021-04-26 17:11:55 +0200 $ ');
elseif comstr(Cam,'@'); out=eval(CAM);
else; error('%s unknown',CAM);
end % - - - - - - - - - - - - - - - - - - - - - -


if 1
elseif param(104)<0; % obsolete
    
 st1={0  'no error'
  -1 'input inconsistent'
 -2 'not enough memory'
 -3 'reordering problem'
 -4 'zero pivot, numerical factorization or iterative refinement problem'
 -5 'unclassified (internal) error'
 -6 'reordering failed (matrix types 11 and 13 only)'
 -7 'diagonal matrix is singular'
 -8 '32-bit integer overflow problem'
 -9 'not enough memory for OOC'
 -10 'problems with opening OOC temporary files'
 -11 'read/write problems with the OOC data file'}
 try; 
   st1=st1{abs(double(param(24))),2};
 catch
   try; st1=st1{abs(double(param(104))),2}; 
   catch
    st1=sprintf('%i (%i) error code',double(param([24 104])));
   end
 end
 error(st1);
end
end % Function

%% #fact mklserv_fact -------------------------------------------------------
function out=mklserv_fact(k,ks,method,varargin) %#ok<INUSL>

r1=method.param;
serv=r1.Server;if length(r1)~=1; error('Inconsistent server entry');end
r2=r1.param;
r1.param=mklserv_utils('defparam');
i1=67; i1=[3 52 60 67];
r1.param(i1)=r2(i1);% msglvl
r3=sdtdef('MklServOOC-safe',[0 10000]);
if r3(1)>0  %if ~isempty(getenv('MKL_PARDISO_OOC_MAX_CORE_SIZE'))
 setenv('MKL_PARDISO_OOC_PATH',nas2up('tempname_MKL_OOC'));
 setenv('MKL_PARDISO_OOC_MAX_CORE_SIZE',num2str(r3(2)))
 setenv('MKL_PARDISO_OOC_MAX_SWAP_SIZE','0')
 setenv('MKL_PARDISO_OOC_KEEP_FILE','1')
 r1.param(60)=r3(1); % xxx
end
%r1.param([11 13 34 2 24 25 3])=[1 2 1 2 1 1 24];% check identical results
%r1.param([34 2 24 25 3])=[ 1 2 1 1 24];% check identical results
%r1.param([2 8])=[0 15]

RO=struct(varargin{:});
if isfield(RO,'MoveFromCaller')&&RO.MoveFromCaller
 k1=inputname(1); 
 if ~isempty(k1); evalin('caller',sprintf('clear %s',k1)); end
end
method.param.isreal=isreal(k);if ~issparse(k);k=sparse(k);end
if isfield(RO,'mtype')
elseif ~isreal(k); RO.mtype=6;% Assume symmetric complex 
  if sdtdef('verm')<904;error('Not implemented use spfmex or Matlab>=904');end
else;  RO.mtype=-2;% assume symmetric indefinite real 
end
if any(RO.mtype==[11 13]);%11 real non-sym, 13 cpx non sym
      k=k.'; if isreal(k);RO.mtype=11;end
      sp_util('setinput',r1.param,[13 1],zeros(1)+9); %r1.param(1+[9:10])%pivot,scaling
      %sp_util('setinput',r1.param,1,-zeros(1)+12); %iparm(1+12) improved acc
elseif RO.mtype==6;
      k=tril(k); 
      if ~isreal(k)
      elseif sdtdef('verm')>907; k=complex(k);method.param.isreal=0; 
      else; RO.mtype=-2;
      end
      sp_util('setinput',r1.param,13,zeros(1)+9); 
elseif any(RO.mtype==[1 -2 2 6]);
      k=feval(ofact('@check_k'),k);
      k=tril(k);sp_util('setinput',r1.param,13,zeros(1)+9); 
      if RO.mtype==-2;  sp_util('setinput',r1.param,1,zeros(1)+55); end% iparam55=[1] to get pivot
end
st=fieldnames(RO);st(~strncmpi(st,'i',1))=[];
for j1=1:length(st)
  sp_util('setinput',r1.param, ... % i9=13,i12=1
      double(RO.(st{j1})),zeros(1)+sscanf(st{j1}(2:end),'%i'));
end
if ~isfield(RO,'msglvl');RO.msglvl=0;end
if isfield(RO,'silent')&&RO.silent; 
    sp_util('setinput',r1.param,double(~RO.silent),ones(1)*66); 
    sp_util('setinput',method.param.param,double(~RO.silent),ones(1)*66); 
end
ite=2; mklserv_utils('serv lpstart');% check server started

while ite % send factor to client
 i1=mklserv_utils('newfact',serv); % factor index 
 % xxx i1=mklserv_client('pidg',serv)
 ite=ite-1; i1=double(i1); % robust id casting issue for newf
 try;   ret=mklserv_client('newf',serv,i1);% New factor
  if ret==0
   if mklserv_client('loca',{0,0}) % new topo without preliminary clear
    sdtw('_nb','problem with newf, retrying after full clear')
    mklserv_utils('clear');
    i1=mklserv_utils('newfact',serv); % factor index: should be 1
    ret=mklserv_client('newf',serv,i1);% New factor
   end
   if ret==0; error('problem with newfact'); end
  end
 catch; ret=-1;
 end
 if ret==-1;error('Server start failed'); end
 ret=mklserv_client('setf',serv,i1);% Set active factor
 if ret==1; break;
 elseif ret==0; error('Failed to set factor');
 end
end

%    setenv('MKL_PARDISO_OOC_CFG_PATH',sdtdef('tempdir'))
%    setenv('MKL_PARDISO_OOC_CFG_FILE_NAME','pardiso_ooc.cfg')
    %MKL_PARDISO_OOC_KEEP_FILE=0 store files

r1.param=r1.param+0; % duplicate parameters / thread r1.param(3)=4;
ret=mklserv_client('setp',serv,double(r1.param(1:67))); % pardiso parameters
if 1==2
  a=mklserv_client('geti',serv);%mklserv_utils('defparam',a);
  mklserv_client('setp',serv,double(r1.param(1:67)));
end
% mklserv_utils('mtype')
% in local version k is modified in place
size_k_1=size(k,1); 
if mklserv_client('loca',{0,0})
%% Local implement
 ret=mklserv_client('seta',serv,k,RO.mtype); %send matrix
 clear k
 ret=mklserv_client('syfr',serv); % symbolic factor
 if r1.param(60)~=0
  [a,b]=mklserv_client('geti',serv); b(60)=r1.param(60);
  ret=mklserv_client('setp',serv,b);
 end
 ret=mklserv_client('nufr',serv); % numeric factor
 if RO.msglvl;mklserv_client('sout',serv);;end
else % remote implement  
 ret=mklserv_client('seta',serv,k,RO.mtype); %send matrix
 clear k % usefull with MoveFromCaller, do before fact

 ret=mklserv_client('syfr',serv); % symbolic factor
 ite=1e5;dt=.1;
 while ite % wait till done
    if ret==0; error('Symbolic factorization failed to start');end
    if ret==2; break; end % done
    ite=ite-1;pause(dt); if ite<5e4;dt=dt*2;end 
    ret=mklserv_client('syfc',serv);
 end
 if RO.msglvl;mklserv_client('sout',serv);else;fprintf('\n');end
 ret=mklserv_client('nufr',serv); % numeric factor
 ite=1e5;dt=.5;
 while ite
    if ret == 2; break; 
    elseif ret==-1;
        if sdtkey('isdev');dbstack; keyboard;
        else;error('Failed to connect');
        end
    end % done
    ite=ite-1;pause(dt); if ite<5e4;dt=dt*2;end 
    ret=mklserv_client('nufc',serv); % 1 started, 2 done
 end
 if RO.msglvl;mklserv_client('sout',serv);end
end
if r1.param(67);
    [a,b]=mklserv_client('geti',serv);%,r1.param);
    fprintf('Fact : %i %i ms, npiv=%i,Mem%i kb\n',b(69:70)*1000,b(14:15))
    if r1.param(67)==2; ret=mklserv_client('setp',serv,[66 0]); end%
    % 22 number of pos eig, 23 number of neg eig
    sp_util('setinput',r1.param,b(1:64),zeros(1));
end
% sdtweb ofact('.ty')
% [Type FactorNumber size(k,1) msglevel tfact tsolve]
if ~strcmpi(serv{1},'local')||sdtkey('cvsnum','mklserv_client')<145
elseif method.param.isreal; method.Solve=@mklserv_rsolve;
else; method.Solve=@mklserv_csolve;
end
out=struct('ty',double([5  i1 size_k_1 RO.msglvl 0 0]), 'ind',[], ...
    'data',[],'dinv',[],'l',[],'u',[],'method',method);
if isfield(RO,'T') % Possibly TkT solve
 [II,JJ,KK]=find(RO.T);
 if all(KK==1)&&length(KK)==size(RO.T,2)&&isequal(JJ,(1:length(JJ))')
   out.method.TktSolve=II;out.ty(7)=size(RO.T,1);
 else; out.method.TktSolve=RO.T;
 end
end

end

%% #mklserv_csolve : real valued single vector for time ----------
function out=mklserv_csolve(b,ty,method) %#ok<DEFNU>

%% missing solveInPlace 
if ischar(b);eval(iigui(b,'MoveFromCaller'));end;b=full(b);
if isa(b,'omat'); out=mklserv_solve(b,ty,method);
else
 if isreal(b); b=complex(b); end; out=complex(b+0);
 mklserv_client('ssov',double(ty(2)),b,out);
 %ret=mklserv_client('setf',{'local',0},ty(2));
 %ret=mklserv_client('setb',{'local',0},b);
 %ret=mklserv_client('solr',{'local',0});
 %if ret~=2; error('delay issue in mkl_serv_call'); end
 %[ret,out]=mklserv_client('getx',{'local',0});
end
end


%% #mklserv_rsolve : real valued single vector for time ----------
function out=mklserv_rsolve(b,ty,method) %#ok<DEFNU>

%% missing solveInPlace 
if ischar(b);eval(iigui(b,'MoveFromCaller'));end;b=full(b);
if 1==2 % ~isempty(method.TktSolve); actuall dont in ofact/mldivide
 if ~isreal(b);error('Not Implemented');
 else; 
  b=b(method.TktSolve); o1=b+0; 
  mklserv_client('ssov',ty(2),b,o1);
  out=zeros(ty(7),size(b,2));out(method.TktSolve,:)=o1;
 end
elseif isa(b,'omat'); out=mklserv_solve(b,ty,method);
elseif ~isreal(b); 
 bi=imag(b); oi=zeros(size(b)); mklserv_client('ssov',ty(2),b,oi);
 b=real(b); or=zeros(size(b)); mklserv_client('ssov',ty(2),b,or);
 out=complex(oi,or);
else;out=b+0; mklserv_client('ssov',ty(2),b,out);
end

end

%% #mklserv_solve : remote solve ---------------------------------------------
function out=mklserv_solve(b,ty,method)

persistent BufSize curF
if isempty(BufSize)
 [i1,r1]=sdtdef('in','MklServBufSize');
 if i1; BufSize=r1; else; BufSize=0; end
 curF=-1;
end
i1=ty(2);serv=method.param.Server;
if curF~=i1 % update factor index
 ret=mklserv_client('setf',serv,i1);
 if ret~=1; error('Failed to select factor'); end
 curF=i1;
end

if ischar(b);eval(iigui(b,'MoveFromCaller'));end;b=full(b);
if ~isreal(b)&&method.param.isreal % real matrix, complex RHS
  out=complex(mklserv_solve(real(b),ty,method), ...
      mklserv_solve(imag(b),ty,method)); return
elseif isa(b,'omat')||(BufSize&&(size(b,2)>1&&numel(b)>BufSize* 134217728)) %  1024^3/8
  % need to do a memcopy not to erase b at higher levels
  if isa(b,'omat'); b=b.memcpy(); else; b=full(b)+0; end
  if BufSize; n_blocks=floor(BufSize/size(b,1)*134217728); else; n_blocks=size(b,2)-1; end
  n_blocks=max(n_blocks,1);
  ind=1:n_blocks:abs(size(b,2)); 
  if ind(end)~=size(b,2)+1; ind(end+1)=size(b,2)+1;end
  i0=zeros(1); %pmat(zeros(1));
  for j1=1:length(ind)-1;
   i1=ind(j1):ind(j1+1)-1;  r1=b(:,i1);
   r1=mklserv_solve('r1',ty,method);
   if isa(b,'omat'); b(:,i1)=r1;
   else; i0=of_time([-1 i0],b,r1); %sp_util('setinput',b,r1,i0);
   end
  end 
  ret=mklserv_client('setb',serv,zeros(size(b,1),1)); % Deallocate
  out=b; return;
elseif ~method.param.isreal
  b=complex(full(b));
else
 b=full(b);
end

%t=clock;
% TODO : setb send by block : serv,b,[istart iend])
%  ret=mklserv_client('getx',serv,out,istart); % receive by block

ret=mklserv_client('setb',serv,b);%dt1=etime(clock,t)
clear b % remove MATLAB instance of used RHS  (usefull for n_block)
if ret==-1; 
    error('Failed to connect, server %s:%i seems to be down',serv{:});
end
ret=mklserv_client('solr',serv);%dt2=etime(clock,t) start solution
ite=0;
if ty(5);pause(min(ty(5)*.95,.1));dt=min(ty(5)/40,.1);
else;dt=.01; 
end
%dt2b=etime(clock,t)
dt=max(dt,1e-3);
while ite<1e4
    if ret == 2; break; end % done
    if ret==0&&ite>1; error('Failed to start solve');end
    if ret == -1; 
      pause(1);drawnow;disp('wait 1s for reconnect'); 
      ret=mklserv_client('solc',serv);
    end % done
    pause(dt);if rem(ite,100)==0;dt=min(dt*5,.1);end
    ret=mklserv_client('solc',serv);
    ite=ite+1;
end
if ret~=2; error('delay issue in mkl_serv_call'); end
%dt3=etime(clock,t)
% Adjust wait
if ty(5)==0;sp_util('setinput',ty,ite*dt,4*ones(1));
elseif ite==0;sp_util('setinput',ty,ty(5)/2,4*ones(1));
elseif ite==1;sp_util('setinput',ty,max(ty(5)-dt,1e-5),4*ones(1));
end
% set solve time in ty(5)
if ty(4);mklserv_client('sout',serv);fprintf('>%g %g s',ite*dt,ty(5));end
[ret,out]=mklserv_client('getx',serv);%dt4=etime(clock,t)
%ret=mklserv_client('setb',serv,zeros(size(out,1),1)); % Deallocate
% TODO : deallocate on server when done getx
if method.param.param(67)==1;
    mklserv_client('geti',serv,method.param.param);
    %mklserv_utils('defparam',method.param.param)
    fprintf('Solve : %.3f ms\n',method.param.param(71)*1000)
end


end




%% #mklserv_clear ----------------------------------------------------------
function out=mklserv_clear(ty,method)

i1=ty(2);
mklserv_utils('clear',i1,method); % factor index

%mklserv_client('quit',serv);
%mklserv_client('help',serv);
out=ty;out(2)=-1;out(3)=0;
end
