## Good name for algoritm:
	Message Forwarding by Categorization of Vehicles (MFCV)

## Link do pdf do minicurso
http://www.eripi.ufpi.br/images/Minicursos/minicurso02.pdf

## Link pergunta Kifayat
http://stackoverflow.com/questions/31079102/vehicles-receive-beacon-messages-outside-rsu-range/31079686#31079686

## Formula para calcular o range
https://groups.google.com/forum/#!topic/omnetpp/7MzAR3iqy74

## Contato com um dos autores
Roniel - ronizsds@gmail.com

## criar net
netconvert --node-files=range.nod.xml --edge-files=range.edg.xml --output-file=range.net.xml

## FSPL formula
https://en.wikipedia.org/wiki/Free-space_path_loss

## Calculate transmission power
http://superuser.com/questions/906398/calculate-required-transmit-power-of-wi-fi-router

-69db=10*log10(P1/P2)

6.9 = log10(P1/P2)

6.9 = log10(x)

10^7

## Resposta do e-mail ao Roneil
Realizei um equivoco quando escrevi o capítulo. Na verdade o raio é de aproximadamente 125m (250m seria o diâmetro). Cheguei a esses valores aproximados através de simulações, já que de acordo com o autor do Veins não existe uma formula fechada para calcular a distância exata. É um sistema probabilístico. Vale ressaltar que cheguei a estes valores utilizando um cenário sem obstáculos. Basta alterar o valor de txPower, como você já fez. 

##
I am working on Veins framework, inside OMNET++. I set the property of RSU inside .ned file as follow:

 @display("p=150,140;b=10,10,oval;r=90");

The tkenv shows a circle around RSU, but the vehicles received beacons outside the range (circle).

How can I adjust the transmission range of RSU to the Circle?

  asked Jun 26 at 17:40
   Kifayat Ullah

##
Adding a display string tag of r is just adding a circle to the graphical output; it does not influence the simulation.
If you define the "transmission range" as the point where the probability of reception is zero, you can calculate this point based on the transmission power at the antenna and the sensitivity of the radio (both set in omnetpp.ini), as well as the used path loss and fading models (set in config.xml). If you change these parameters and models, you are changing this "transmission range".
Note, however, that this range has only little relevance to frame reception probability. Veins employs the MiXiM suite and approach to model transmissions as two-dimensional (time and frequency) functions of signal power that are modified by path loss and fading effects (both stochastic and deterministic, e.g., due to buildings). If a frame's ' receive power is above the sensitivity threshold, its reception probability is computed based on dividing these functions for signal, interference, and noise to derive the SINR and, from that, the bit error rate. Even at moderate interference levels, this means that most frames cannot be decoded even though they are well above the sensitivity threshold (simply because their SINR was too low).
Just to repeat: I am warning against calculating a "transmission range" for anything other than purely informational purposes. How far you can send in theory has absolutely no relation to how far you can send on a moderately busy channel. This effect is modeled in Veins!

    answered Jun 26 at 18:18
      Christoph Sommer
##

Thanks for detailed reply. It helps to understand the concept of "transmission range". I need to write in my article the range (in meters). I used the default parameters of Veins. What should I write ? Is it 300 m , 400m or more ? – Kifayat Ullah Jun 27 at 17:45

This really depends on the scenario. If you want to give your readers an idea of the absolute maximum distance you could ever send, or how far you could typically send, my suggestion is to simply record some result data: record the distances of successful and of failed packet transmissions and compute the success rate of transmissions at each distance. Then tell your readers at what distance the success rate was still above, for example, 95%. –  Christoph Sommer Jun 27 at 23:53

## Executar no terminal
 -u = mode, -f = arquivo ini -r = nº de execucão -l = libveins -n = arquivos ned
 # sem GUI
opp_run -u Cmdenv -n .:../../src/ -f app.ini -r 0 -l ../../src/libveins.so
 # com GUI
opp_run -u Tkenv -n .:../../src/ -f app.ini -r 0 -l ../../src/libveins.so

 #sem repeat
opp_run -u Cmdenv -n .:../../src/ -f app.ini -l ../../src/libveins.so
opp_run -u Tkenv -n .:../../src/ -f app.ini -l ../../src/libveins.so
