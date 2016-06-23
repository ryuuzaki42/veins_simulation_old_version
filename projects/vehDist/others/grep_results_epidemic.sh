#!/bin/bash
#
# Autor= João Batista Ribeiro
# Bugs, Agradecimentos, Criticas "construtiva"
# Mande me um e-mail. Ficarei Grato!
# e-mail: joao42lbatista@gmail.com
#
# Este programa é um software livre; você pode redistribui-lo e/ou
# modifica-lo dentro dos termos da Licença Pública Geral GNU como
# publicada pela Fundação do Software Livre (FSF); na versão 2 da
# Licença, ou (na sua opinião) qualquer versão.
#
# Este programa é distribuído na esperança que possa ser útil,
# mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO a
# qualquer MERCADO ou APLICAÇÃO EM PARTICULAR.
#
# Veja a Licença Pública Geral GNU para maiores detalhes.
# Você deve ter recebido uma cópia da Licença Pública Geral GNU
# junto com este programa, se não, escreva para a Fundação do Software
#
# Livre(FSF) Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#
# Script: Script to collect the simulation result in one place
#
# Última atualização: 23/06/2016
#
## Local my pc folder
#cd /media/sda4/prog/simulation_veins/projects/epidemic/others/
cd /media/sda4/prog/simulation_veins/projects/vehDist/others/
# or
## Cluster folder, change de v001 for the veins version
#cd /mnt/nfs/home/luz.marina/0_jonh/veins_v00$part/projects/vehDist$part/others/

part=$1
if [ "$part" == '' ]; then
    echo -e "\nError, you need pass the result_part value, e.g., $0 \"1\" 1 1\n"
    exit 1
fi

numExpI_1to8=$2
if [ "$numExpI_1to8" == '' ]; then
    echo -e "\nError, you need pass the start experiment number (1 to 8) value, e.g., $0 1 \"1\" 1\n"
    exit 1
fi

numExpF_1to8=$3
if [ "$numExpF_1to8" == '' ]; then
    echo -e "\nError, you need pass the finish experiment number (1 to 8) value, e.g., $0 1 1 \"1\"\n"
    exit 1
fi

if [ "$numExpF_1to8" -lt "$numExpI_1to8" ]; then
    numExpF_1to8=$numExpI_1to8
fi

echo -e "\nNumber of experiments: $numExpI_1to8 to $numExpF_1to8"
((numExpF_1to8++))

echo -e "\n## Script to collect the simulation results in one place"
echo -e "\n   ## Results from vehDist $part\n"

pathFolder="../results/vehDist_resultsEnd_$part"
rsu0File="rsu\[0\]_Count_Messages_Received.r"
vehiclesFile="Veh_Messages_Drop.r"

experiment="0099_epidemic"

echo -e "## Values from experiment $experiment"

i=$numExpI_1to8
while [ $i -lt $numExpF_1to8 ]; do
    echo -e "               ## Experiment $i\n"
    cat $pathFolder/$experiment/E$i_*/$rsu0File | grep -E "Exp: $i"
    echo
    cat $pathFolder/$experiment/E$i_*/$vehiclesFile | grep -E "Exp: $i"
    ((i++))
done

echo -e "\n               ## End of script\n"
#