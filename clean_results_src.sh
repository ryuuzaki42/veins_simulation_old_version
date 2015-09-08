cd /media/sda4/prog/veins-3.0/
echo -e "Deseja remover os results da pasta de projects?\n"
echo -e "Irá apagar os arquivos projects/*/results e projects/*/.tkenvrc\n (y)es - (rm ...) ou (n)o (exit)"
read resposta
if [ $resposta = y ]
  then
  rm -r projects/*/results/
  rm projects/*/.tkenvrc
fi
if [ $resposta = n ]
  then
  echo -e "Fim do script \n"  
fi