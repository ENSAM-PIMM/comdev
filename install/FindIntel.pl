#!/usr/bin/perl -w

# ./FindIntel.pl mexa64 bash 904 
# ./FindIntel.pl lms_elan2 bash
# ./FindIntel.pl lms_elan2 make
# ./FindIntel.pl canada make
# Used by t_mkl('tbbpath'); 

use Cwd; 
use Sys::Hostname;

#foreach $key (sort keys(%ENV)) {
#      print STDERR "$key = $ENV{$key}\n";
#   }
 
################################################################

$a = @ARGV;
#$HOST=$ENV{"HOST"};if  (!defined($HOST)) {$HOST=$ENV{"USERDOMAIN"};}
$HOST = hostname;
if ($a==0) {$_=$HOST;} else {$_=shift(@ARGV);}
if (@ARGV==0) {$out="matlab";} else {$out=shift(@ARGV);}

if ((/orchis/) || (/chene/) || (/mexglx/) ) {  # Configure output directory
}
if ((/mexmaci/) || (/mexmaci64/) ) {  # Configure the output directories
 $TbbInc="/opt/intel/tbb/include";
 $TbbLib="/opt/intel/tbb/lib";
 $MklRoot="/opt/intel/mkl";
 $OmpLib="";
}
if ((-d "/opt/intel/Compiler/11.0") ) {  # elan/canada  

 $TbbInc="/opt/intel/Compiler/11.0/083/tbb/include";
 $TbbLib="/opt/intel/Compiler/11.0/083/tbb/lib/em64t";
 $MklRoot="/opt/intel/Compiler/11.0/083/mkl";
} 
if ((-d "/opt/intel/tbb")) {
 $TbbRoot="/opt/intel/tbb";
 $TbbLib="/opt/intel/tbb/lib/em64t";
 $MklRoot="/opt/intel/mkl";
 $MklL=" -lmkl_intel_lp64 -lmkl_tbb_thread -lmkl_core -ltbb ";
}

if (/mexa64/) { #elan2/canada3
    #$TbbInc="/opt/intel/linux/tbb/include";
    $TbbInc="/opt/intel/tbb/include/tbb";
 if (-d "/opt/intel/compilers_and_libraries_2020") {
  $TbbRoot="/opt/intel/tbb";
  $MklRoot="/opt/intel/mkl";
 } else {
  $TbbRoot="/opt/intel/tbb";
  $MklRoot="/opt/intel/mkl";
 }
 if (-d "/opt/intel/clck") {
  $OmpLib="/opt/intel/clck/2017.1.016/provider/share/common/lib/intel64";
 } else {
  $OmpLib="/o/balmes/sdt.cur/mklserv/64";
 }
}
#if ((-d "/opt/intel/compilers_and_libraries_2017") || (/mexa64/)  ) {  # elan2 
# $TbbRoot="/opt/intel/compilers_and_libraries_2017/linux/tbb";
# $MklRoot="/opt/intel/compilers_and_libraries_2017/linux/mkl";
# if (-d "/opt/intel/clck") {
#  $OmpLib="/opt/intel/clck/2017.1.016/provider/share/common/lib/intel64";
# } else {
#     $OmpLib="/o/balmes/sdt.cur/mklserv/64";
# }
#}
#if ((-d "/opt/intel/compilers_and_libraries_2020") || (/mexa64/)  ) {  # elan2 
# $TbbInc="/opt/intel/compilers_and_libraries_2017/linux/tbb/include";
# $TbbRoot="/opt/intel/compilers_and_libraries_2020/linux/tbb";
# $MklRoot="/opt/intel/compilers_and_libraries_2020/linux/mkl";
# if (-d "/opt/intel/clck") {
#  $OmpLib="/opt/intel/clck/2017.1.016/provider/share/common/lib/intel64";
# } else {
#   $OmpLib="/o/balmes/sdt.cur/mklserv/64";
# }
#}
if ((-d "d:\\APP\\win64\\intelx") ) {  # elan/canada  
 $Lroot="D:\\APP\\win64\\intel\\compilers_and_libraries_2017\\windows";
 $TbbInc= "$Lroot\\tbb\\include";
 $TbbLib="$Lroot\\tbb\\lib\\intel64\\vc10";
 #$TbbLib="D:\\share\\APP\\win64\\M90\\bin\\win64";
 $MklInc="$Lroot\\mkl\\include";
 $MklLib="$Lroot\\mkl\\lib\\intel64";
 $MklL=" -lmkl_intel_lp64 -lmkl_tbb_thread -lmkl_core -ltbb ";
 $OmpLib="D:\\APP\\win64\\intel\\compilers_and_libraries_2017\\windows\\compiler\\lib\\intel64_win";

} elsif ((/epicea3/) || (/mexw64/)) {  # Configure the output directories
 $TbbInc="\\\\lms-bigsave.paris.ensam.fr\\publications\\win32\\Composer\\tbb\\include";
 $TbbLib="\\\\lms-bigsave.paris.ensam.fr\\publications\\win32\\Composer\\tbb\\intel64\\vc8\\lib";
 #$MklLib="d:\\APP\\intel\\mkl\\9.1.021\\em64t\\lib";
 #$MklInc="d:\\APP\\intel\\mkl\\9.1.021\\include";
 $Lroot="C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1";
 $Lroot="C:\\APP\\Intel";
 $TbbInc="C:\\APP\\Intel\\tbb\\include";
 $MklL=" -lmkl_intel_lp64 -lmkl_tbb_thread -lmkl_core -ltbb ";
 #$TbbLib="$Lroot\\tbb\\lib\\intel64\\vc8";
 # mklink /d mkl compilers_and_libraries_2019\windows\mkl
 # mklink /d  latest ../tbb
 # mklink /d  latest ../mkl
 $TbbLib="$Lroot\\tbb\\latest\\lib\\intel64\\vc_mt";
 $MklInc="$Lroot\\mkl\\latest\\include";
 $MklLib="$Lroot\\mkl\\latest\\lib\\intel64";
 $OmpLib="d:\\APP\\win64\\M98\\bin\\win64";
}

if  ((/mexw32/)) {  # Configure the output directories

 $TbbInc="C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\tbb\\include";
 $TbbLib="C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\tbb\\lib\\ia32\\vc9";
 $Lroot="c:\\Program Files (x86)\\IntelSWTools\\compilers_and_libraries_2019\\windows";
 #$TbbInc="\\\\lms-bigsave.paris.ensam.fr\\publications\\win32\\Composer\\tbb\\include";
 #$TbbLib="\\\\lms-bigsave.paris.ensam.fr\\publications\\win32\\Composer\\tbb\\ia32\\vc8\\lib";
 #$MklInc="d:\\sdtools_h\\dis_sdt\\intel\\mkl9\\include";
 #$MklInc="P:\\publications\\win32\\MKL\\10.2.2.025\\include";
 #$MklLib="P:\\publications\\win32\\MKL\\10.2.2.025\\ia32\\lib";
 $TbbInc="C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\tbb\\include";
 #$TbbLib="C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\tbb\\lib\\ia32\\vc8";
 $TbbInc="$Lroot\\tbb\\include";
 $TbbLib="$Lroot\\tbb\\lib\\ia32_win\\vc_mt";
 $MklInc="$Lroot\\mkl\\include";
 $MklLib="$Lroot\\mkl\\lib\\ia32";


 #d:\\sdtools_h\\dis_sdt\\intel\\MKL9\\ia32\\lib";
}
if (defined($TbbRoot)) {
 $TbbLib="$TbbRoot/lib/em64t"; 
 if (!(-d $TbbLib)) {$TbbLib="$TbbRoot/lib/intel64"; }
 if (!(-d $TbbLib)) {$TbbLib="$TbbRoot/lib"; }
 if ($HOST =~/lms-elan2/) {$TbbLib="$TbbRoot/lib/intel64/gcc4.1"}
 elsif (($HOST =~/canada/) || ($HOST =~/lms-elan/)) {
     $TbbLib="$TbbRoot/lib/intel64/gcc4.8"}
     if (!(-d $TbbLib)) {$TbbLib="$TbbRoot/lib/intel64/gcc4.7"}
 elsif (!(-f "$TbbLib/libtbb.so")) {
    $T2="$TbbLib/cc3.4.3_libc2.3.4_kernel2.6.9"; 
   if ((-f "$T2")) {$TbbLib=$T2;}
 }
 if (defined($TbbInc)) { } else {$TbbInc="$TbbRoot/include";}
}
if (defined($MklRoot)) {
 $MklLib="$MklRoot/lib/em64t"; 
 if (!(-d $MklLib)) {$MklLib="$MklRoot/lib/intel64"; }
 if (!(-d $MklLib)) {$MklLib="$MklRoot/lib"; }
 $MklInc="$MklRoot/include";
}

if ( $out =~ /bash/ ) {
    print STDOUT "\nexport TbbLib=\"$TbbLib\";";
    print STDOUT "\nexport TbbInc=\"$TbbInc\";";
    if (defined($OmpLib)) { 
    print STDOUT "\nexport OmpLib=\"$OmpLib\";";
    }
    print STDOUT "\nexport MklLib=\"$MklLib\";";
    print STDOUT "\nexport MklInc=\"$MklInc\";";
    print STDOUT "\nexport MklL=\"$MklL\";";
    print STDOUT "\n";
} elsif ( $out =~ /make/) {
    print STDOUT "\nexport MKL_LIBRARY_PATH=\"$MklLib\";";
    print STDOUT "\nexport TBB_LIBRARY_PATH=\"$TbbLib\";";
    if (defined($OmpLib)) { 
    print STDOUT "\nexport OMP_LIBRARY_PATH=\"$OmpLib\";";
    }
    print STDOUT "\nexport MKL_INCLUDE_PATH=\"$MklInc\";";
    print STDOUT "\nexport TBB_INCLUDE_PATH=\"$TbbInc\";";
    print STDOUT "\n";

} elsif ( $out =~ /matlab/) {
    print STDOUT "\nout=struct('TbbLib','$TbbLib', ...";
    print STDOUT "\n    'TbbInc','$TbbInc', ...";
    print STDOUT "\n    'MklLib','$MklLib', ...";
    if (defined($OmpLib)) { 
      print STDOUT "\n    'OmpLib','$OmpLib', ...";
    }
    print STDOUT "\n    'MklInc','$MklInc');\n";
}


