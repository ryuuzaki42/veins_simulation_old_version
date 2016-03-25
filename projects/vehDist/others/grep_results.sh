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
# Última atualização: 25/03/2016
#
echo -e "\n## Scrpit to collect the simulation results in one place\n"

pathFolder="../results/resultsEnd"
rsu_0_File="rsu\[0\]_Count_Messages_Received.r"
vehs_file="Veh_Messages_Drop.r"
experiment1="1_chosenByDistance"
experiment2="2_chosenByDistance_Speed"
experiment3="3_chosenByDistance_Speed_Category"
experiment4="4_chosenByDistance_Speed_Category_RateTimeToSend"

echo -e "\n## Values from experiment $experiment1\n"
i=1
while [ $i -lt 9 ]; do
    echo -e "\n               ## Experiment $i\n"
    cat $pathFolder/$experiment1/E*/$rsu_0_File | grep -E "Exp: $i"
    echo
    cat $pathFolder/$experiment1/E*/$vehs_file | grep -E "Exp: $i"
    i=$((i+1))
done

echo -e "\n\n## Values from experiment $experiment2\n"
i=1
while [ $i -lt 9 ]; do
    echo -e "\n               ## Experiment $i\n"
    cat $pathFolder/$experiment2/E*/$rsu_0_File | grep -E "Exp: $i"
    echo
    cat $pathFolder/$experiment2/E*/$vehs_file | grep -E "Exp: $i"
    i=$((i+1))
done

echo -e "\n\n## Values from experiment $experiment3\n"
i=1
while [ $i -lt 9 ]; do
    echo -e "\n               ## Experiment $i\n"
    cat $pathFolder/$experiment3/E*/$rsu_0_File | grep -E "Exp: $i"
    echo
    cat $pathFolder/$experiment3/E*/$vehs_file | grep -E "Exp: $i"
    i=$((i+1))
done

echo -e "\n\n## Values from experiment $experiment4\n"
i=1
while [ $i -lt 9 ]; do
    echo -e "\n               ## Experiment $i\n"
    cat $pathFolder/$experiment4/E*/$rsu_0_File | grep -E "Exp: $i"
    echo
    cat $pathFolder/$experiment4/E*/$vehs_file | grep -E "Exp: $i"
    i=$((i+1))
done

echo -e "\n\n               ## End of script\n"
#end