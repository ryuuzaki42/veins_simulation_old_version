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
# Script: Scrpit to collect the simulation result in one place
#
# Última atualização: 06/02/2016
#
echo -e "\nScrpit to collect the simulation result in one place\n"

echo -e "\t## rsu[0] count messages received ##"
cat ../results/resultsEnd/*/rsu\[0\]_Count_Messages_Received.r | grep -E "#####|Execution|### Count|### avg" | sed 's/#####*//g'
echo

echo -e "\t## veh[*] messages droped ##"
cat ../results/resultsEnd/*/Veh_Messages_Drop.r | grep -E "#####|Execution|## Final" | sed 's/#####*//g'

echo -e "\n\nEnd of script\n"
#end
