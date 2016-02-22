//#####################################################################
//                  To compile:                                       #
// g++ -std=c++0x -o script_generate_routes script_generate_routes.cc #
//#####################################################################

#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

int main(){
    freopen("test.rou.xml","r",stdin); // Arquivo de entrada gerado com script randomTrips.py

    ofstream output;
    output.open("test_end2.rou.xml"); // Arquivo que será criado com todas rotas

    output << "<routes>\n" << endl; // Escrita da definição do tipo de veículo no arquivo de saída

    bool parte1 = true;
    bool print = false;
    string routePart1, routePart2, to, toTmp, line;
    int count, countVehicleCagegoryA, p1, p2, dist, countVehicleRoutes, routeComp;
    count = 1;
    countVehicleRoutes = 50;
    countVehicleCagegoryA = 40;

    // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], como uma rua tem 250 m, logo 1 km => 4 *9 => 36; 10 km => 360. 360 + 26 = 386
    routeComp = 3700; // logo X km, x * 36 + 26; Para 100 km, 100 * 36 + 26 = 3626, para ter certeza 3700

    cout << endl << "Por favor espere, gerando rotas..." << endl << endl;

    while (getline(cin,line) && count <= countVehicleRoutes) { // count < 50 to create 50 rotas
        if (line.compare(0,15,"        <route ") == 0) { // Edita cada linha do arquivo de entrada que representa rotas
            to = "    <route id=\"";
            to += "route" + to_string(count);
            to += "\"";
            toTmp = to;
            to += line.substr(14) + "\n";

            if (parte1) {
                p1 = 22;
                p2 = 85;
                while ((p2 + 12) < to.size()) {
                    dist = 8;
                    while ((p2 + 12) < to.size()) {
                        routePart1 = line.substr(p1,8); // Pega o primeiro ponto (ponto de partida)
                        routePart2 = line.substr(p2,8); // Pega o oitavo ponto (ponto de partida)
                        if (strcmp(routePart1.c_str(), routePart2.c_str()) == 0) {
//     <route edges="0/3to1/3 1/3to1/2 1/2to1/1 1/1to2/1 2/1to3/1 3/1to4/1 4/1to4/0 4/0to3/0 3/0to2/0 2/0to2/1 2/1to2/2...
                            if (dist == 9) { // dist == 9 para ter 8 ponto na rota
                                cout << line.substr(p1,(p2 - p1 + 8)) << " dist: " << dist << " p1: " << p1 << " p2: "<< p2 << endl;
                                toTmp += " edges=\"";
                                while (toTmp.size() < routeComp) {
                                    toTmp += line.substr(p1,(p2 - p1));
                                }
                                toTmp += "\"/>\n";
                                output << toTmp;
                                p2 += toTmp.size();

                                count++;
                                if (count > countVehicleCagegoryA){
                                    parte1 = false;
                                }
                            }
                        }
                        p2 += 9;
                        dist++;
                    }
                    p1 += 9;
                    p2 = p1 + 9;
                }
            } else {
                if (to.size() > routeComp){
                    output << to; //Escrita da rota no arquivo de saída
                    count++;
                } else {
                    //cout << endl << "to.size, menor que" << routeComp << " : " << to.size() << endl;
                }
            }
        }
    }

    // Um com id=P e outro com id=T
    output << "\n    <!-- P => Carro de Passeio -->\n";
    output << "    <vType id=\"P\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"0,1,0\"/>\n\n";
    count = 1;
    while (count < countVehicleCagegoryA) {
        output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh" << count <<"\" route=\"route" << count << "\" type=\"P\"/>\n";
        count++;
    }

    output << "\n    <!-- T => Taxi -->\n";
    output << "    <vType id=\"T\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"1,1,0\"/>\n\n";
    while (count <= countVehicleRoutes) {
        output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh" << count <<"\" route=\"route" << count << "\" type=\"T\"/>\n";
        count++;
    }
    output << endl << "</routes>"; //Finalização do arquivo de saída
    cout << endl;
    return 0;
}
