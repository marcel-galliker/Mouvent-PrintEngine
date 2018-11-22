#/bin/sh
all_installed=1;
ime_lib_path="../../2_hdldesigner/ime_lib/hdl"
printf "\n#############################\n### Check installed Tools ###\n#############################\n"

program="quartus_map"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program, set PATH or open nios shell\n\n"
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="quartus_fit"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program, set PATH or open nios shell\n\n"
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="quartus_asm"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program, set PATH or open nios shell\n\n"
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="quartus_sta"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program, set PATH or open nios shell\n\n"
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="quartus_cpf"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program, set PATH or open nios shell\n\n"
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="qsys-edit"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program, add <install dir>\altera\quartus\sopc_builder\bin to PATH\n\n"
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="dos2unix"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program :\n\n"
    sudo apt-get install $program
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="tar"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# install $program :\n\n"
    sudo apt-get install $program
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program="gtar"
condition=$(which $program 2>/dev/null | grep -v "not found" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    echo "$program is not installed"
    printf "# set symbolic link $program to tar:\n\n"
    sudo ln /bin/tar /bin/gtar
    all_installed=0;
else
  printf "# $program is already installed\n"
fi

program=$ime_lib_path
condition=$(ls $program | grep ime_functions.vhd | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### ERROR ###\n"
    printf "# Please set a correct ime_lib_path on line 3 of this script,\n which contain the file ime_functions.vhd\n\n"
    all_installed=0;
else
  printf "# Correct ime_lib_path recogniced\n"
fi

program="fhnw licence server"
echo "# checking acces to $program:"
condition=$(ping -c 1 adm.ds.fhnw.ch | grep -v "0% packet loss" | wc -l)
if [ $condition -eq 0 ] ; then
printf "\n### WARNING ###\n"
    echo "no acces to $program"
else
  printf "# acces to $program \n"
fi

if [ $all_installed -eq 0 ]; then
  printf "\n####################################################################\n"
  printf "### There are errors, please correct them and rerun this script! ###"
  printf "\n####################################################################\n"
else
  printf "\n########################################\n"
  printf "### Preparation passed successfully! ###"
  printf "\n########################################\n"
  profile=$(env)
  questa=$(echo $profile | grep ../../3_questasim/ | wc -l)
  design_digital=$(echo $profile | grep ../../ | wc -l)
  hdldesigner=$(echo $profile | grep ../../2_hdldesignerl/ | wc -l)

  if [ $questa -eq 0 ]; then
    echo "# DESIGN_DIGITAL_QUESTA_PATH is not set, will set it now #"
    export DESIGN_DIGITAL_QUESTA_PATH="../../3_questasim"
  else 
    echo "# DESIGN_DIGITAL_QUESTA_PATH is set #"
  fi

  if [ $design_digital -eq 0 ]; then
    echo "# DESIGN_DIGITAL_PATH is not set, will set it now #"
    export DESIGN_DIGITAL_PATH="../.."
  else 
    echo "# DESIGN_DIGITAL_PATH is set #"
  fi

  if [ $hdldesigner -eq 0 ]; then
    echo "# DESIGN_DIGITAL_HDS_PATH is not set, will set it now #"
    export DESIGN_DIGITAL_HDS_PATH="../../2_hdldesigner"
  else 
    echo "# DESIGN_DIGITAL_HDS_PATH is set #"
  fi
  
  PROJECT_FILE=( $(ls *.qsys) )
  
  qsys-edit $PROJECT_FILE
  
fi
