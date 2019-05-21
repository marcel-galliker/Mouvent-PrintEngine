#/bin/sh
all_installed=1;
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
printf "\n### WARNING ###\n"
    echo "$program is not installed"
else
  printf "# $program is already installed\n"
fi

if [ $all_installed -eq 0 ]; then
  printf "\n####################################################################\n"
  printf "### There are errors, please correct them and rerun this script! ###"
  printf "\n####################################################################\n"
else
  printf "\n########################################\n"
  printf "### Preparation passed successfully! ###"
  printf "\n########################################\n"
  
  PROJECT_FILE=( $(ls *.qsys) )

  ## qsys-edit $PROJECT_FILE
  qsys-generate $PROJECT_FILE --synthesis=VHDL
fi
