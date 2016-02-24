//#######################################################
//                  To compile:                         #
// g++ -std=c++0x -o 1_path_routes.out 1_path_routes.cc #
//#######################################################

#include <iostream>
#include <fstream>
#include <string.h>
#include <map>

using namespace std;

struct distributionCategory {
    int categoryA;
    int categoryB;
};

int main(){
    freopen("test.rou.xml","r",stdin); // Arquivo de entrada gerado com script randomTrips.py

    ofstream output;
    output.open("test_end2.rou.xml"); // Arquivo que será criado com todas rotas

    output << "<routes>\n" << endl; // Escrita da definição do tipo de veículo no arquivo de saída

    bool parte1 = true;
    bool validRoute;
    string routeTmp, to, toTmp, line, fristPart, middlePart, lastPart;
    int count, countVehicleCagegoryA, p1, p2, dist, countVehicleRoutes;
    int notLoopStreet, notLoopStreetTmp, routeComp, pathComp, pathCompTmp, pTmp;
    count = 1;
    pathComp = 4; // Metade do percurso em blocos (quarteirão)
    countVehicleRoutes = 50;
    countVehicleCagegoryA = 40;
    notLoopStreet = notLoopStreetTmp = 10; // Em pedaços da rota o veículo não pode dar volta na rua
    p1 = 22; // Inicia a rota

    // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], como uma rua tem 250 m.
    // logo 1 km => 4 *9 => 36 + 26 + 9 (ponto inicial). x km = x * 36 + 26 + 9
    routeComp = 580; // Para 15 km, 15 * 36 + 26 + 9 = 575, para ter certeza 580

    cout << endl << "Por favor espere, gerando rotas..." << endl << endl;

    while (getline(cin,line) && count <= countVehicleRoutes) { // count < 50 to create 50 rotas
        if (line.compare(0,15,"        <route ") == 0) { // Edita cada linha do arquivo de entrada que representa rotas
            to = "    <route id=\"";
            if (count < 10) {
                to += "route0" + to_string(count);
            } else {
                to += "route" + to_string(count);
            }
            to += "\"";
            toTmp = to;
            to += line.substr(14) + "\n";

            if (to.size() > (pathComp * 9 + p1 -1 + notLoopStreet * 9)) {
                pTmp = p1;
                validRoute = true;
                notLoopStreetTmp = notLoopStreet;
                while(notLoopStreetTmp > 0 && validRoute) {
                    if (line.substr(pTmp, 3) == line.substr(pTmp + 14, 3)) {
                        validRoute = false;
                    }
                    notLoopStreetTmp--;
                    pTmp +=9;
                }

                if (validRoute) {
                    if (parte1) {
                        p2 = pathComp * 9 + p1 -1;
                        routeTmp = line.substr(p1,(p2-p1)); // Pega o primeira parte da rota
//     <route edges="0/3to1/3 1/3to1/2 1/2to1/1 1/1to2/1 2/1to3/1 3/1to4/1 4/1to4/0 4/0to3/0 3/0to2/0 2/0to2/1 2/1to2/2...

                        toTmp += " edges=\"";
                        pathCompTmp = pathComp;
                        while(pathCompTmp > 0){
                            routeTmp += " ";
                            dist = (pathCompTmp - 1) * 9 + p1;

                            fristPart = line.substr(dist, 3);
                            middlePart = line.substr((dist + 3), 2);
                            lastPart = line.substr((dist + 5), 3);
                            //cout << line.substr(dist, 8) << " dist: " << dist << " p1: " << p1 << " p2: "<< p2 << " Result: " << lastPart << middlePart << fristPart << endl;

                            dist = dist - 9;
                            routeTmp += lastPart + middlePart + fristPart;
                            pathCompTmp--;
                        }

                        toTmp += routeTmp;
                        while (toTmp.size() < routeComp) {
                            toTmp += " " + routeTmp;
                        }

                        toTmp += "\"/>\n";
                        output << toTmp;
                        count++;

                        if (count > countVehicleCagegoryA){
                            parte1 = false;
                        }
                    } else {
                        if (to.size() > routeComp){
                            toTmp += line.substr(14, 16);
                            p1 = 31;
                            while (toTmp.size() < (routeComp + pathComp * 9 * 2 - 9)) {
                                toTmp += " " + line.substr(p1, 8);
                                p1 = p1 + 9;
                            }
                            toTmp += "\"/>\n";
                            output << toTmp; // Escrita da rota no arquivo de saída
                            count++;
                        } else {
                            //cout << endl << "to.size, menor que" << routeComp << " : " << to.size() << endl;
                        }
                    }
                }
            }
        }
    }

    // Um com id=P e outro com id=T
    output << "\n    <!-- P => Carro de Passeio -->\n";
    output << "    <vType id=\"P\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"0,1,0\"/>\n\n";
    count = 1;
    while (count < countVehicleCagegoryA) {
        if (count < 10){
            output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh0" << count <<"\" route=\"route0" << count << "\" type=\"P\"/>\n";
        } else {
            output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh" << count <<"\" route=\"route" << count << "\" type=\"P\"/>\n";
        }
        count++;
    }

    output << "\n    <!-- T => Taxi -->\n";
    output << "    <vType id=\"T\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"1,1,0\"/>\n\n";
    while (count <= countVehicleRoutes) {
        if (count < 10){
            output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh0" << count <<"\" route=\"route0" << count << "\" type=\"T\"/>\n";
        } else {
            output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh" << count <<"\" route=\"route" << count << "\" type=\"T\"/>\n";
        }
        count++;
    }
    output << endl << "</routes>"; // Finalização do arquivo de saída
    output.close();

    // verificar dispersão de veículo no cenário
    freopen("test_end2.rou.xml","r",stdin); // Arquivo de entrada gerado com script randomTrips.py

    count = 1;
    map<string, struct distributionCategory> routes;
    map<string, struct distributionCategory>::iterator it;

    output.open("distributionCategory_end.r"); // Arquivo de saída da distribuição de veículos nos locais
    output << "Resultado da distibuição dos veículos pela ruas" << endl;
    output << "cP = Veículos de Passeio.   cT = Táxi" << endl << endl;
    output << "          Nome da rua    Count cP  Count cT   %% cP           %% cT" << endl << endl;

    while (getline(cin,line) && count <= countVehicleRoutes) { // count < 50 to create 50 rotas
        if (line.compare(0,11,"    <route ") == 0) {
            p1 = 31;
            to = line.substr(p1);
            p2 = to.size();

            while (p1 < p2) {
                routeTmp = line.substr(p1,8); // Pega o primeiro ponto (ponto de partida)

                it = routes.find(routeTmp);
                if (it != routes.end()) { // Testa se ele já foi inserido ou existe
                    if (count < countVehicleCagegoryA){
                        it->second.categoryA++;

                    } else {
                        it->second.categoryB++;
                    }
                } else {
                    struct distributionCategory dC;
                    dC.categoryA = 0;
                    dC.categoryB = 0;
                    if (count < countVehicleCagegoryA){
                        dC.categoryA++;
                    } else {
                        dC.categoryB++;
                    }
                    routes.insert(make_pair(routeTmp, dC));
                }
                p1 += 9;
            }
            count++;

        }
    }

    count = 1;
    cout.precision(2);
    output.precision(2);
    double percentage, percentA, percentB;
    percentA = percentB = 0;
    for (it = routes.begin(); it != routes.end(); it++) {
        if (count < 10){
            output << "   Street_0" << count <<": " << it->first << "    cP: " << it->second.categoryA;
        } else {
            output << "   Street_" << count <<": " << it->first << "    cP: " << it->second.categoryA;
        }

        if (it->second.categoryA < 10) {
            output << " ";
        }

        output << "    cT: " << it->second.categoryB;
        if (it->second.categoryB < 10) {
            output << " ";
        }

        percentage = it->second.categoryA + it->second.categoryB;
        percentage = it->second.categoryA/percentage;
        output << "   %%cP: " << percentage;
        percentA +=percentage;

        percentage = it->second.categoryA + it->second.categoryB;
        percentage = it->second.categoryB/percentage;
        percentB +=percentage;
        output << "     %%cT: " << percentage << endl;
        count++;
     }
    percentA = percentA/routes.size();
    percentB = percentB/routes.size();
    output << endl << "Porcentagem geral, %%GcP: " << percentB << "    %%GcT: " << percentA;
    output.close();
    return 0;
}