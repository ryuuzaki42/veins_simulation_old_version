#!/bin/bash
#
# Autor= João Batista Ribeiro
# Bugs, Agradecimentos, Criticas "construtiva"
# Mande me um e-mail. Ficarei Grato!
# e-mail  joao42lbatista@gmail.com
#
# Este programa é um software livre; você pode redistribui-lo e/ou 
# modifica-lo dentro dos termos da Licença Pública Geral GNU como 
# publicada pela Fundação do Software Livre (FSF); na versão 2 da 
# Licença, ou (na sua opinião) qualquer versão.
#
# Este programa é distribuído na esperança que possa ser  útil, 
# mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO a 
# qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. 
#
# Veja a Licença Pública Geral GNU para maiores detalhes.
# Você deve ter recebido uma cópia da Licença Pública Geral GNU
# junto com este programa, se não, escreva para a Fundação do Software
#
# Livre(FSF) Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#
# Script: inicia o SUMO (sumo ou sumo-gui) como requisitado pelo framework Veins
#
# Última atualização: 19/03/2016
#
# Pasta do Veins (altere se a sua for diferente
cd /media/sda4/prog/veins/

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
#