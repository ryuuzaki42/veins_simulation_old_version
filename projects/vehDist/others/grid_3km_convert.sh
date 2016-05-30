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
# Script: Convert the values from a grid with 3 km (12 edges 0 to 12)
# to use just one character in the edges
#
# Última atualização: 24/05/2016

# 10 to a
sed -i 's/to10/toa/g' vehDist.net.xml
sed -i 's/_10/_a/g' vehDist.net.xml
sed -i 's/\/10/\/a/g' vehDist.net.xml
sed -i 's/10\//a\//g' vehDist.net.xml

# 11 to b
sed -i 's/to11/tob/g' vehDist.net.xml
sed -i 's/_11/_b/g' vehDist.net.xml
sed -i 's/\/11/\/b/g' vehDist.net.xml
sed -i 's/11\//b\//g' vehDist.net.xml

# 12 to c
sed -i 's/to12/toc/g' vehDist.net.xml
sed -i 's/_12/_c/g' vehDist.net.xml
sed -i 's/\/12/\/c/g' vehDist.net.xml
sed -i 's/12\//c\//g' vehDist.net.xml

exit 0
#