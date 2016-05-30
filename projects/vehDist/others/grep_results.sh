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
# Última atualização: 08/05/2016
#
part=$1
if [ "$part" == '' ]; then
   echo -e "\nError, you need pass the result_part value, e.g., $0 1\n"
   exit 1
fi

num_exp_i=$2
if [ "$num_exp_i" == '' ]; then
    num_exp_i=1
fi

num_exp_f=$3
if [ "$num_exp_f" == '' ]; then
    num_exp_f=8
fi

if [ "$num_exp_f" -lt "$num_exp_i" ]; then
    num_exp_f=$num_exp_i
fi

echo -e "\nNumber of experiments: $num_exp_i to $num_exp_f\n"
((num_exp_f++))

## Local pc folder
cd /media/sda4/prog/simulation_veins/projects/vehDist/others/
# or
## Cluser folder, change de v001 for the veins version
#cd /mnt/nfs/home/luz.marina/0_jonh/veins_v00$part/projects/vehDist$part/others/

echo -e "\n## Script to collect the simulation results in one place\n"
echo -e "\n   ## Results from vehDist $part\n"

pathFolder="../results/vehDist_resultsEnd_$part"
rsu_0_File="rsu\[0\]_Count_Messages_Received.r"
vehs_file="Veh_Messages_Drop.r"

count=1
continue_flag=1

while [ $continue_flag == 1 ]; do
    case $count in
        1) experiment="0001_chosenByDistance" ;;
        2) experiment="0012_chosenByDistance_Speed" ;;
        3) experiment="0013_chosenByDistance_Category" ;;
        4) experiment="0014_chosenByDistance_RateTimeToSend" ;;
        5) experiment="0123_chosenByDistance_Speed_Category" ;;
        6) experiment="1234_chosenByDistance_Speed_Category_RateTimeToSend" ;;
    esac

    echo -e "\n## Values from experiment $experiment\n"
    i=$num_exp_i
    while [ $i -lt $num_exp_f ]; do
        echo -e "\n               ## Experiment $i\n"
        cat $pathFolder/$experiment/E$i_*/$rsu_0_File | grep -E "Exp: $i"
        echo
        cat $pathFolder/$experiment/E$i_*/$vehs_file | grep -E "Exp: $i"
        ((i++))
    done

    ((count++))
    if [ $count == 7 ]; then
        continue_flag=0
    fi
done

echo -e "\n\n               ## End of script\n"

exit 0
#