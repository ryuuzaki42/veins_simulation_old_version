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
# Última atualização: 31/05/2016
#
countGrid=$1
if [ "$countGrid" == "" ]; then
    echo "Error, need to pass the grid count to convert (e.g., $0 13)"
    exit 1
elif [ $countGrid -lt 10 ]; then
    echo "Error, count grid need more than 9, (e.g., $0 13)"
    exit 1
elif [ $countGrid -gt 35 ]; then
    echo "Error, can't work with count grid more than 35"
    exit 1
fi

countGridTmp=10
((countGrid++))
echo -e "\nConverting...\n"
while [ $countGridTmp -lt $countGrid ]; do
    case $countGridTmp in
        10) inValue=10; outValue="a" ;;
        11) inValue=11; outValue="b" ;;
        12) inValue=12; outValue="c" ;;
        13) inValue=13; outValue="d" ;;
        14) inValue=14; outValue="e" ;;
        15) inValue=15; outValue="f" ;;
        16) inValue=16; outValue="g" ;;
        17) inValue=17; outValue="h" ;;
        18) inValue=18; outValue="i" ;;
        19) inValue=19; outValue="j" ;;
        20) inValue=20; outValue="k" ;;
        21) inValue=21; outValue="l" ;;
        22) inValue=22; outValue="m" ;;
        23) inValue=23; outValue="n" ;;
        24) inValue=24; outValue="o" ;;
        25) inValue=25; outValue="p" ;;
        26) inValue=26; outValue="q" ;;
        27) inValue=27; outValue="r" ;;
        28) inValue=28; outValue="s" ;;
        29) inValue=29; outValue="t" ;;
        30) inValue=30; outValue="u" ;;
        31) inValue=31; outValue="v" ;;
        32) inValue=32; outValue="x" ;;
        33) inValue=33; outValue="w" ;;
        34) inValue=34; outValue="y" ;;
        35) inValue=35; outValue="z" ;;
    esac

    sed -i 's/to'$inValue'/to'$outValue'/g' vehDist.net.xml
    sed -i 's/_'$inValue'/_'$outValue'/g' vehDist.net.xml
    sed -i 's/\/'$inValue'/\/'$outValue'/g' vehDist.net.xml
    sed -i 's/'$inValue'\//'$outValue'\//g' vehDist.net.xml

    ((countGridTmp++))
    echo -e "\tTurning \"$inValue\" in \"$outValue\""
done

echo -e "\nConverting finished\n"
#