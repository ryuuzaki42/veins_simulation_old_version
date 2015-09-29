cd /media/sda4/prog/veins-3.0/
echo -e "Deseja executar sumo com interface gráfica:\n (y)es - sumo-gui ou (n)o sumo"
read resposta
if [ $resposta = y ]
   then
   echo -e "Running sumo-gui\n"
   python sumo-launchd.py -vv -c sumo-gui
fi
if [ $resposta = n ]
  then
  echo -e "Running sumo\n"
  python sumo-launchd.py -vv -c sumo
fi
