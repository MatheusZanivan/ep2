#!/bin/bash

#https://cpl.li/posts/2019-03-12-mbrfat/
#https://www.youtube.com/watch?v=jRj_HzbHeWU
#https://en.wikipedia.org/wiki/Master_boot_record

#Maximum capacity = sector size x cylinders (10 bits) x heads (8 bits) x sectors per track (6 bits)
#512 x 1024 x 256 x 63 (or 512 x 2^24) = 8,455,716,864 bytes or 7.8 GB

MBR=mbr.bin
PROG=ep
SENSEI=$(whoami)
PAR=sda
GCC_ERROR="\nFalhas ao compilar.\nAbortando..\n"
MBR_ERROR="\nFalha ao gerar arquivo $MBR\n"

[ "$1" == "-c" ] || [ "$1" == "--compile" ] && [ -f "$PROG" ] && rm $PROG

function error_exit {
  echo -e "$1" 
  exit
}
echo -e "\nBem vindo $SENSEI!\n"

if ! [ -f "$MBR" ]; then
  #se precisar rodar o dd, entao precisa ser root.
  [ "$EUID" -ne 0 ] && echo -e "Permissao negada!\nPrograma dd precisa ser executado como root\nNao foi possivel gerar o $MBR" && \
  exit || echo -e "\nCriando $MBR.." && dd if=/dev/$PAR of=$MBR bs=512 count=1 && echo -e "\nArquivo $MRB criado com sucesso!" || error_exit "$MBR_ERROR" 
fi

if ! [ -f "$PROG" ]; then
  echo -e "\nCompilando EP.."
  gcc $PROG.c -o $PROG && chmod +x $PROG && echo "Compilado com sucesso!" || error_exit "$GCC_ERROR"
fi

echo -e "\n== EP02 ==\n"

./$PROG

[ "$EUID" -eq 0 ] && echo -e "\n\n== OUTPUT DO FDISK ==\n" && fdisk -l

echo -e "\n\nAluno: Joao Barradas"

cat << 'EOF'
 ______________________ 
< Boa noite professor! >
 ---------------------- 
   \
    \
        .--.
       |o_o |
       |:_/ |
      //   \ \
     (|     | )
    /'\_   _/`\
    \___)=(___/

EOF
