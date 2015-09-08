cd /media/sda4/prog/veins-veins-3.0/
echo -e "Deseja executa sumo com interface gráfica:\n (y)es - sumo-gui ou (n)o sumo"
read resposta
if [ $resposta = y ] 
	then
	echo -e "Running sumo-gui"
	python sumo-launchd.py -vv -c sumo-gui
fi
if [ $resposta = n ]
  then
  echo -e "Running sumo"
  python sumo-launchd.py -vv -c sumo
fi
