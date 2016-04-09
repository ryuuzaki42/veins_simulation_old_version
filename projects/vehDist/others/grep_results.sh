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
# Última atualização: 09/04/2016
#
cd /media/sda4/prog/simulation_veins/projects/vehDist/others/
echo -e "\n## Scrpit to collect the simulation results in one place\n"

pathFolder="../results/vehDist_resultsEnd"
rsu_0_File="rsu\[0\]_Count_Messages_Received.r"
vehs_file="Veh_Messages_Drop.r"

count=1
continue=1

while [ $continue == 1 ]; do
    case $count in
        1) experiment="1_chosenByDistance" ;;
        2) experiment="12_chosenByDistance_Speed" ;;
        3) experiment="13_chosenByDistance_Category" ;;
        4) experiment="14_chosenByDistance_RateTimeToSend" ;;
        5) experiment="123_chosenByDistance_Speed_Category" ;;
        6) experiment="1234_chosenByDistance_Speed_Category_RateTimeToSend" ;;
    esac

    echo -e "\n## Values from experiment $experiment\n"
    i=1
    while [ $i -lt 9 ]; do
        echo -e "\n               ## Experiment $i\n"
        cat $pathFolder/$experiment/E*/$rsu_0_File | grep -E "Exp: $i"
        echo
        cat $pathFolder/$experiment/E*/$vehs_file | grep -E "Exp: $i"
        i=$((i+1))
    done

    ((count+=1))

    if [ $count == 7 ]; then
        continue=0
    fi
done

echo -e "\n\n               ## End of script\n"
exit 0
#end