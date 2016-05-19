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
# Script: limpa o histórico de resultados de execução do framework Veins
#
# Última atualização: 23/11/2015
#
# Pasta do Veins (altere se a sua for diferente
cd /media/sda4/prog/simulation_veins/

echo -e "Deseja remover os results da pasta de projects?\n"
echo -e "Irá apagar os arquivos projects/*/results e projects/*/.tkenvrc\n (y)es - (rm ...) ou (n)o (exit)"

read resposta
if [ $resposta = y ]
  then # altera projects para o nome da sua pasta de projetos.
  rm -r projects/*/results/
  rm projects/*/.tkenvrc
fi
if [ $resposta = n ]
  then
  echo -e "Fim do script \n"  
fi
#