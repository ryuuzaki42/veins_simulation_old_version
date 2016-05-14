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
    int categoryP;
    int categoryT;
};

int main() {
    string fileInput, fileOutput, fileDist;
    fileInput = "vehDist_tmp.rou.xml";
    fileOutput = "vehDist.rou.xml";
    fileDist = "distributionCategory.r";
    freopen(fileInput.c_str(), "r", stdin); // Arquivo de entrada gerado com script randomTrips.py

    ofstream output;
    output.open(fileOutput.c_str()); // Arquivo que será criado com todas rotas

    output << "<routes>\n" << endl; // Escrita da definição do tipo de veículo no arquivo de saída

    bool parte1, go_and_back;
    bool validRoute, use_depart_Pos_arrivalPos_departSpeed_as_random, use_left_and_right_road_as_samePlace;
    string routeTmp, to, toTmp, line, fristPart, middlePart, lastPart;
    unsigned short int count, countVehicleCagegoryA, p1, p2, dist, countVehicleRoutes;
    unsigned short int notLoopStreet, notLoopStreetTmp, routeComp, pathComp, pathCompTmp, pTmp;
    unsigned short int insert_by_time, time_to_insert, vehicle_time_depart, countTmp, compare, compare2;

    count = 1;
    pathComp = 4; //4; // Metade do percurso em blocos (quarteirão)
    countVehicleRoutes = 100; //100;
    countVehicleCagegoryA = 20; //20;
    insert_by_time = 10; //10;
    time_to_insert = 60; //60
    parte1 = true;
    go_and_back = true; // true
    use_left_and_right_road_as_samePlace = true;
    use_depart_Pos_arrivalPos_departSpeed_as_random = false;
    notLoopStreet = notLoopStreetTmp = 10; // Em pedaços da rota o veículo não pode dar volta na rua
    p1 = 22; // Início da rota

    // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], como uma rua tem 250 m.
    // logo 1 km => 4 *9 => 36 + 26 + 9 (ponto inicial). x km = x * 36 + 26 + 9
    routeComp = 580; //580 // Para 15 km, 15 * 36 + 26 + 9 = 575, para ter certeza 580
    // Will generate 576 part of 9 (1/2to3/4 ) => (576/9) * 250/1000 => 16 km
    // By 25 m/s or 90 km/h => will move 15 km
    // By 15 m/s or 54 km/h => will move 9 km
    // 16000 m/25 m/s => 640 s

    cout << "Por favor espere, gerando rotas..." << endl << endl;

    while (getline(cin, line) && count <= countVehicleRoutes) { // count < 50 para criar 50 rotas
        if (line.compare(0, 15, "        <route ") == 0) { // Edita cada linha do arquivo de entrada que representa rotas
            to = "    <route id=\"";
            if (count < 10) {
                to += "route00" + to_string(count);
            } else if (count < 100) {
                to += "route0" + to_string(count);
            } else {
                to += "route" + to_string(count);
            }
            to += "\"";
            toTmp = to;
            to += line.substr(14) + "\n";

            compare = pathComp * 9 + p1 - 1 + notLoopStreet * 9;
            if (to.size() > compare) {
                pTmp = p1;
                validRoute = true;
                notLoopStreetTmp = notLoopStreet;
                while(notLoopStreetTmp > 0 && validRoute) {
                    if (line.substr(pTmp, 3) == line.substr(pTmp + 14, 3)) {
                        validRoute = false;
                    }
                    notLoopStreetTmp--;
                    pTmp += 9;
                }

                if (validRoute) {
                    if (parte1) {
                      if (to.size() > routeComp) {
                            toTmp += line.substr(14, 16);
                            p1 = 31;
                            compare2 = routeComp + (pathComp * 9 * 2) - 46;
                            while (toTmp.size() < compare2) {
                                toTmp += " " + line.substr(p1, 8);
                                p1 = p1 + 9;
                            }
                            toTmp += "\"/>\n";

                            output << toTmp; // Escrita da rota no arquivo de saída
                            count++;
                        } //else {
                            //cout << endl << "to.size, menor que" << routeComp << " : " << to.size() << endl;
                        //}

                        if (count > countVehicleCagegoryA) {
                            parte1 = false;
                        }
                    } else {
                        p1 = 22;

                        if (go_and_back) {
                            p2 = pathComp * 9 - 1;
                        } else {
                            p2 = ((pathComp * 9) * 2) -1;
                        }

                        routeTmp = line.substr(p1, p2); // Pega o primeira parte da rota
//     <route edges="0/3to1/3 1/3to1/2 1/2to1/1 1/1to2/1 2/1to3/1 3/1to4/1 4/1to4/0 4/0to3/0 3/0to2/0 2/0to2/1 2/1to2/2 ...

                        toTmp += " edges=\"";
                        pathCompTmp = pathComp;
                        while(pathCompTmp > 0 && go_and_back) {
                            routeTmp += " ";
                            dist = (pathCompTmp - 1) * 9 + p1;

                            fristPart = line.substr(dist, 3);
                            middlePart = "to"; //line.substr((dist + 3), 2);
                            lastPart = line.substr((dist + 5), 3);
                            //cout << line.substr(dist, 8) << " dist: " << dist << " p1: " << p1 << " p2: "<< p2 << " Result: " << lastPart << middlePart << fristPart << endl;

                            dist -= 9;
                            routeTmp += lastPart + middlePart + fristPart;
                            pathCompTmp--;
                        }

                        toTmp += routeTmp;

                        //cout << "Route: " << routeTmp << endl;
                        //while (toTmp.size() < routeComp) {
                        //    toTmp += " " + routeTmp;
                        //}

                        toTmp += "\"/>\n";
                        output << toTmp; // Escrita da rota no arquivo de saída
                        count++;
                    }
                }
            }
        }
    }
    output << "\n    <!-- T => Taxi -->\n";
    output << "    <vType id=\"T\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"1,1,0\"/>\n\n";
    count = 1;
    while (count <= countVehicleCagegoryA) {
        if (count < 10) {
            if (use_depart_Pos_arrivalPos_departSpeed_as_random) {
                output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh00" << count <<"\" route=\"route00" << count << "\" type=\"T\"/>\n";
            } else {
                output << "        <vehicle depart=\"0\" departPos=\"0\" arrivalPos=\"max\" departSpeed=\"0\" id=\"veh00" << count << "\" route=\"route00" << count << "\" type=\"T\"/>\n";
            }
        } else if (count < 100) {
            if (use_depart_Pos_arrivalPos_departSpeed_as_random) {
                output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh0" << count <<"\" route=\"route0" << count << "\" type=\"T\"/>\n";
            } else {
                output << "        <vehicle depart=\"0\" departPos=\"0\" arrivalPos=\"max\" departSpeed=\"0\" id=\"veh0" << count << "\" route=\"route0" << count << "\" type=\"T\"/>\n";
            }
        } else {
            if (use_depart_Pos_arrivalPos_departSpeed_as_random) {
                output << "        <vehicle depart=\"0\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh" << count <<"\" route=\"route" << count << "\" type=\"T\"/>\n";
            } else {
                output << "        <vehicle depart=\"0\" departPos=\"0\" arrivalPos=\"max\" departSpeed=\"0\" id=\"veh" << count << "\" route=\"route" << count << "\" type=\"T\"/>\n";
            }
        }
        count++;
    }


    // Um com id=P e outro com id=T
    output << "\n    <!-- P => Carro de Passeio -->\n";
    output << "    <vType id=\"P\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"0,1,0\"/>\n\n";

    output << "    <!-- http://sumo.dlr.de/wiki/Definition_of_Vehicles,_Vehicle_Types,_and_Routes -->" << endl;
    vehicle_time_depart = 0;
    countTmp = 0;
    while (count <= countVehicleRoutes) {
        if (count < 10) {
            if (use_depart_Pos_arrivalPos_departSpeed_as_random) {
                output << "        <vehicle depart=\"" << vehicle_time_depart << "\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh00" << count <<"\" route=\"route00" << count << "\" type=\"P\"/>\n";
            } else {
                output << "        <vehicle depart=\"" << vehicle_time_depart << "\" departPos=\"0\" arrivalPos=\"max\" departSpeed=\"0\" id=\"veh0" << count << "\" route=\"route00" << count << "\" type=\"P\"/>\n";
            }
        } else if (count < 100) {
            if (use_depart_Pos_arrivalPos_departSpeed_as_random) {
                output << "        <vehicle depart=\"" << vehicle_time_depart << "\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh0" << count <<"\" route=\"route0" << count << "\" type=\"P\"/>\n";
            } else {
                output << "        <vehicle depart=\"" << vehicle_time_depart << "\" departPos=\"0\" arrivalPos=\"max\" departSpeed=\"0\" id=\"veh0" << count << "\" route=\"route0" << count << "\" type=\"P\"/>\n";
            }
        } else {
            if (use_depart_Pos_arrivalPos_departSpeed_as_random) {
                output << "        <vehicle depart=\"" << vehicle_time_depart << "\" departPos=\"random\" arrivalPos=\"random\" departSpeed=\"random\" id=\"veh" << count <<"\" route=\"route" << count << "\" type=\"P\"/>\n";
            } else {
                output << "        <vehicle depart=\"" << vehicle_time_depart << "\" departPos=\"0\" arrivalPos=\"max\" departSpeed=\"0\" id=\"veh" << count << "\" route=\"route" << count << "\" type=\"P\"/>\n";
            }
        }
        count++;
        countTmp++;
        if (countTmp == insert_by_time){ 
            countTmp = 0;
        }

        if (countTmp == 0) {
            vehicle_time_depart += time_to_insert;
        }
    }
    output << endl << "</routes>"; // Finalização do arquivo de saída
    output.close();
    cout << "Rotas salvas no arquivo " << fileOutput << "..." << endl << endl;

    // verifica dispersão de veículo no cenário
    freopen(fileOutput.c_str(), "r", stdin); // Arquivo de gerado na primeira parte

    count = 1;
    map<string, struct distributionCategory> routes;
    map<string, struct distributionCategory>::iterator it;

    output.open(fileDist.c_str()); // Arquivo de saída da distribuição de veículos nos locais
    output << "## Distribuição dos veículos pelos segmentos de ruas" << endl;
    output << "cP = Veículos de Passeio.   cT = Táxi" << endl << endl;

    if (use_left_and_right_road_as_samePlace) {
        output << "Nome do segmento de rua                  Count cP    Count cT    %cP    %cT    cT + cP" << endl << endl;
    } else {
        output << "Nome do segmento de rua    Count cP    Count cT    %cP    %cT    cT + cP" << endl << endl;
    }

    while (getline(cin,line) && count <= countVehicleRoutes) { // count <= 50 to 50 routes
        if (line.compare(0, 11, "    <route ") == 0) {
            p1 = 32;
            to = line.substr(p1);
            p2 = to.size();

            while (p1 < p2) {
                routeTmp = line.substr(p1, 8); // Pega o primeiro ponto (ponto de partida)
                it = routes.find(routeTmp);

                if (use_left_and_right_road_as_samePlace) {
                    if (it == routes.end()) {
                        fristPart = routeTmp.substr(0, 3);
                        middlePart = "to"; //routeTmp.substr(3, 2);
                        lastPart = routeTmp.substr(5, 3);
                        routeTmp = lastPart + middlePart + fristPart;

                        it = routes.find(routeTmp);
                    }
                }

                if (it != routes.end()) { // Testa se ele já foi inserido ou existe
                    if (count <= countVehicleCagegoryA) {
                        it->second.categoryP++;
                    } else {
                        it->second.categoryT++;
                    }
                } else {
                    struct distributionCategory dC;
                    dC.categoryP = dC.categoryT = 0;
                    if (count <= countVehicleCagegoryA) {
                        dC.categoryP++;
                    } else {
                        dC.categoryT++;
                    }
                    routes.insert(make_pair(routeTmp, dC));
                }
                p1 += 9;
            }
            count++;

        }
    }

    count = 1;
    cout.precision(4);
    output.precision(4);
    double percentage;
    int countP, countT, countTandP;
    countP = countT = 0;
    for (it = routes.begin(); it != routes.end(); it++) {
        if (use_left_and_right_road_as_samePlace) {
            routeTmp = it->first;
            fristPart = routeTmp.substr(0, 3);
            middlePart = "to"; //routeTmp.substr(3, 2);
            lastPart = routeTmp.substr(5, 3);
            routeTmp = lastPart + middlePart + fristPart;

            if (count < 10) {
                output << "Street_Segment_0" << count << ": " << it->first << " | " << routeTmp;
            } else {
                output << "Street_Segment_" << count << ": " << it->first << " | " << routeTmp;
            }
        } else {
            if (count < 10) {
                output << "Street_Segment_0" << count << ": " << it->first;
            } else {
                output << "Street_Segment_" << count << ": " << it->first;
            }
        }

        countTandP = it->second.categoryP + it->second.categoryT;
        output << "    cT + cP: " << countTandP;
        if (countTandP < 10) {
            output << "  ";
        } else if (countTandP < 100){
            output << " ";
        }

        output<< "    cP: " << it->second.categoryP;
        if (it->second.categoryP < 10) {
            output << "  ";
        } else if (it->second.categoryP < 100){
            output << " ";
        }

        output << "    cT: " << it->second.categoryT;
        if (it->second.categoryT < 10) {
            output << "  ";
        } else if (it->second.categoryT < 100){
            output << " ";
        }

        countP += it->second.categoryP;
        percentage = (double(it->second.categoryP)/countTandP) * 100;
        output << "   %cP: " << percentage;

        countT += it->second.categoryT;
        percentage = (double(it->second.categoryT)/countTandP) * 100;
        output << "    %cT: " << percentage << endl;

        count++;
     }

    output << endl << "CountTotal: " << (countP + countT) << " countP: " << countP << "    countT: " << countT << endl << endl;
    countTandP = countP + countT;
    percentage = double(countP)/countTandP;
    output << "Porcentagem geral, %%GcP: " << percentage;
    percentage = double(countT)/countTandP;
    output << "    %%GcT: " << percentage << endl;

    output.close();
    cout << "Distribuição de veículos pelos segmentos de rotas salvas no arquivo " << fileDist << "..." << endl << endl;
    return 0;
}
