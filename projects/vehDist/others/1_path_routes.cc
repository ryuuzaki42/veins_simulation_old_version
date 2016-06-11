//###########################################################
//                      To compile:
// g++ -std=c++0x -Wall -o 1_path_routes.out 1_path_routes.cc
//###########################################################

#include <map>
#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

struct distributionCategory {
    int categoryP;
    int categoryT;
};

unsigned short int generate_routes (unsigned short int lineStart, unsigned short int routeFileNumber);

int main() {
    unsigned short int countFileRoutesGenerate, routeFileNumber, lineStart;

    countFileRoutesGenerate = 1;

    lineStart = 0;       //e.g., 0, 1319, 2537, 3446, 4763, 5780
    routeFileNumber = 0; //e.g., 0,    1,    2,    3,    4,    5

    while (routeFileNumber < countFileRoutesGenerate) {
        lineStart = generate_routes(lineStart, routeFileNumber);
        routeFileNumber++;
    }

    return 0;
}

unsigned short int generate_routes (unsigned short int lineStart, unsigned short int routeFileNumber) {
    ofstream output;

    string fileInput, fileOutput, fileDist, fileResultPartName;
    string routeTmp, to, toTmp, line, fristPart, middlePart, lastPart;

    bool parte1, goAndBack, validRoute, stopPart;
    bool useDepartPos_ArrivalPos_DepartSpeed_AsRandom, useLeftAndRightRoadAsSamePlace;

    unsigned short int count, countVehicleCagegoryT, p1, p2, dist, countVehicleRoutes, lineCount;
    unsigned short int notLoopStreet, notLoopStreetTmp, routeComp, pathComp, pathCompTmp, pTmp;
    unsigned short int insertByTime, timeToInsert, compare, compare2, stopDurationTime, countPBegin, simulationTimeLimit;
    double sigmaValue;

    count = 1; // route start number
    pathComp = 4; //4 //1 é 250 m de rota e 4 1 km que no final se torna 2 km de rota
    simulationTimeLimit = 600 - 100;
    countVehicleRoutes = 105; //170; //50
    countVehicleCagegoryT = 5; //10
    countPBegin = 20; //40; //10
    insertByTime = 20; //40; //5
    timeToInsert = 120; //60
    sigmaValue = 0; //0.5
    goAndBack = false; //false
    stopPart = false; // Se colocar true coloque valor maior que zero em stopDurationTime, e.g. 20
    // Site configs: http://sumo.dlr.de/wiki/Definition_of_Vehicles,_Vehicle_Types,_and_Routes
    stopDurationTime = 60;
    useLeftAndRightRoadAsSamePlace = true;
    useDepartPos_ArrivalPos_DepartSpeed_AsRandom = false; //false
    notLoopStreet = notLoopStreetTmp = 10; // Em pedaços da rota o veículo não pode dar volta na rua
    p1 = 22; // Início da rota
    parte1 = true;

    if (routeFileNumber < 10) {
        fileResultPartName = "0" + to_string(routeFileNumber);
    } else {
        fileResultPartName = to_string(routeFileNumber);
    }

    fileInput = "vehDist_tmp.rou.xml";
    fileOutput = "vehDist.rou"+ fileResultPartName + ".xml";
    fileDist = "distributionCategory" + fileResultPartName + ".r";

    freopen(fileInput.c_str(), "r", stdin); // Arquivo de entrada gerado com script randomTrips.py

    lineCount = 0;
    while (getline(cin, line) && (lineCount < lineStart)) {
        lineCount++;
    }

    // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], como uma rua tem 250 m
    // logo 1 km => 4 *9 => 36 + 26 + 9 (ponto inicial). x km = x * 36 + 26 + 9
    routeComp = 580; //580 // Para 15 km, 15 * 36 + 26 + 9 = 575, para ter certeza 580
    // Will generate 576 part of 9 (1/2to3/4 ) => (576/9) * 250/1000 => 16 km
    // By 25 m/s or 90 km/h => will move 15 km
    // By 15 m/s or 54 km/h => will move 9 km
    // 16000 m/25 m/s => 640 s
    //routeComp = 280; // 7 km

    output.open(fileOutput.c_str()); // Arquivo que será criado com todas rotas
    output << "<routes>" << endl << endl; // Escrita da definição do tipo de veículo no arquivo de saída

    output << "    <!--" << endl;
    output <<"    File with " << countVehicleRoutes << " (" << countVehicleCagegoryT << " T, ";
    output << (countVehicleRoutes - countVehicleCagegoryT) << " P) routes" << endl;
    output << "    goAndBack: " << boolalpha << goAndBack << endl;
    output << "    Routes T (" << count << " to " << countVehicleCagegoryT <<"): \"random\"" << endl;
    output << "    Routes P (" << (countVehicleCagegoryT + 1) << " to " << countVehicleRoutes << "): ";
    output << ((double(pathComp) * 250)/1000) * 2 << " km (" << ((pathComp * 250) * 2) << " m)" << endl;
    output << "    Begin insert " << countVehicleCagegoryT << " T and " << countPBegin << " P" << endl;
    output << "        After this, insert " << insertByTime << " P by each " << timeToInsert << " seconds"<< endl;
    output << "    fileOutput: " << fileOutput << endl;
    output << "    lineStart: " << lineCount << endl;
    output << "    -->" << endl << endl;

    cout << endl << "Por favor espere, gerando rotas..." << endl << endl;
    while (getline(cin, line) && (count <= countVehicleRoutes)) { // count < 50 para criar 50 rotas
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
                    if (parte1) { // Routes T
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

                        if (count > countVehicleCagegoryT) {
                            parte1 = false;
                            output << endl;
                        }
                    } else { // Routes P
                        p1 = 22;

                        if (goAndBack) {
                            p2 = pathComp * 9 - 1;
                        } else {
                            p2 = ((pathComp * 9) * 2) -1;
                        }

                        string routeStopPart;
                        if (stopPart) {
                            unsigned short int middleRoute = pathComp * 9 - 1;;
                            //cout << "Route " << count << " middleRoute " << middleRoute << " stopPart: " << line.substr((p1 + middleRoute + 1), 9) << endl;
                            //cout << "Full route " << line.substr(p1, p2) << endl;
                            routeStopPart = line.substr((p1 + middleRoute - 8), 8);
                        }

                        routeTmp = line.substr(p1, p2); // Pega o primeira parte da rota
//     <route edges="0/3to1/3 1/3to1/2 1/2to1/1 1/1to2/1 2/1to3/1 3/1to4/1 4/1to4/0 4/0to3/0 3/0to2/0 2/0to2/1 2/1to2/2...

                        toTmp += " edges=\"";
                        pathCompTmp = pathComp;
                        while(pathCompTmp > 0 && goAndBack) {
                            routeTmp += " ";
                            dist = (pathCompTmp - 1) * 9 + p1;

                            fristPart = line.substr(dist, 3);
                            middlePart = "to"; //line.substr((dist + 3), 2);
                            lastPart = line.substr((dist + 5), 3);
                            //cout << line.substr(dist, 8) << " dist: " << dist << " p1: " << p1 << " p2: "<< p2;
                            //cout << " Result: " << lastPart << middlePart << fristPart << endl;

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
                        if (stopPart) {
                            toTmp += "        <stop lane=\"" + routeStopPart +"_0\" parking=\"true\" friendlyPos=\"true\" endPos=\"50\" duration=\"" + to_string(stopDurationTime) + "\"/>\n    </route>\n";
                        }

                        output << toTmp; // Escrita da rota no arquivo de saída
                        count++;
                    }
                }
            }
        }
        lineCount++;
    }
    output << endl << "    <!-- line_end: " << lineCount << " -->" << endl;

    string departPos, departSpeed, arrivalPos;
    if (useDepartPos_ArrivalPos_DepartSpeed_AsRandom) {
        departPos = departSpeed = arrivalPos = "random";
    } else {
        departPos = departSpeed = "0";
        arrivalPos = "max";
    }
    string routeDescripPart = " departPos=\"" + departPos + "\" arrivalPos=\"" + arrivalPos;
    routeDescripPart += "\" departSpeed=\"" + departSpeed + "\" id=\"";

    string vehDescripType = " accel=\"3\" decel=\"5\" sigma=\"" + to_string(sigmaValue);
    vehDescripType += "\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"16.67\" ";

    output << endl << "    <!-- T => Taxi/Táxi -->" << endl;
    // Um com id=T e outro com id=P
    output << "    <vType id=\"T\"" << vehDescripType << "color=\"1,1,0\"/>" << endl << endl;

    count = 1;
    while (count <= countVehicleCagegoryT) {
        output << "        <vehicle depart=\"0\"" << routeDescripPart;

        if (count < 10) {
            output << "veh00" << count <<"\" route=\"route00" << count;
        } else if (count < 100) {
            output << "veh0" << count <<"\" route=\"route0" << count;
        } else {
            output << "veh" << count <<"\" route=\"route" << count;
        }
        output << "\" type=\"T\"/>" << endl;

        count++;
    }

    output << endl << "    <!-- P => Private car/Carro de passeio -->" << endl;
    output << "    <vType id=\"P\"" << vehDescripType << "color=\"0,1,0\"/>" << endl << endl;
    output << "    <!-- http://sumo.dlr.de/wiki/Definition_of_Vehicles,_Vehicle_Types,_and_Routes -->" << endl;

    unsigned short int countTmp, vehicleTimeDepart, countPBeginTmp;
    vehicleTimeDepart = 0;
    countPBeginTmp = countPBegin + count;
    countTmp = insertByTime - 1;
    while (count <= countVehicleRoutes) {
        if (vehicleTimeDepart > simulationTimeLimit) {
            cout << endl << endl <<"Error, vehicleTimeDepart is great than simulationTimeLimit" << endl << endl;
            exit(1);
        }
        output << "        <vehicle depart=\"" << vehicleTimeDepart << "\"" << routeDescripPart;

        if (count < 10) {
            output << "veh00" << count <<"\" route=\"route00" << count;
        } else if (count < 100) {
            output << "veh0" << count <<"\" route=\"route0" << count;
        } else {
            output << "veh" << count <<"\" route=\"route" << count;
        }
        output <<  "\" type=\"P\"/>" << endl;
        count++;

        if (count >= countPBeginTmp) {
            countTmp++;
            if (countTmp == insertByTime){
                countTmp = 0;
                vehicleTimeDepart += timeToInsert;
            }
        }
    }
    output << endl << "</routes>"; // Finalização do arquivo de rotas
    output.close();
    cout << "Rotas salvas no arquivo " << fileOutput << endl << endl;

    // verifica dispersão de veículo no cenário
    freopen(fileOutput.c_str(), "r", stdin); // Arquivo de gerado na primeira parte

    count = 1;
    map <string, struct distributionCategory> routes;
    map <string, struct distributionCategory>::iterator itRoutes;

    while (getline(cin,line) && count <= countVehicleRoutes) { // count <= 50 to 50 routes
        if (line.compare(0, 11, "    <route ") == 0) {
            p1 = 32;
            to = line.substr(p1);
            p2 = to.size();

            while (p1 < p2) {
                routeTmp = line.substr(p1, 8); // Pega o primeiro ponto (ponto de partida)
                itRoutes = routes.find(routeTmp);

                if (useLeftAndRightRoadAsSamePlace) {
                    if (itRoutes == routes.end()) {
                        fristPart = routeTmp.substr(0, 3);
                        middlePart = "to"; //routeTmp.substr(3, 2);
                        lastPart = routeTmp.substr(5, 3);
                        routeTmp = lastPart + middlePart + fristPart;

                        itRoutes = routes.find(routeTmp);
                    }
                }

                if (itRoutes != routes.end()) { // Testa se ele já foi inserido ou existe
                    if (count <= countVehicleCagegoryT) {
                        itRoutes->second.categoryT++;
                    } else {
                        itRoutes->second.categoryP++;
                    }
                } else {
                    struct distributionCategory dC;
                    dC.categoryP = dC.categoryT = 0;
                    if (count <= countVehicleCagegoryT) {
                        dC.categoryT++;
                    } else {
                        dC.categoryP++;
                    }
                    routes.insert(make_pair(routeTmp, dC));
                }
                p1 += 9;
            }
            count++;
        }
    }

    output.open(fileDist.c_str()); // Arquivo de saída da distribuição de veículos nos locais
    output << "## Distribuição dos veículos pelos segmentos de ruas" << endl;
    output << "cP = Veículos de Passeio.   cT = Táxi" << endl << endl;

    output << "Nome do segmento de rua";
    if (useLeftAndRightRoadAsSamePlace) {
        output << "                  Count cP    Count cT    %cP    %cT    cT + cP";
    } else {
        output << "    Count cP    Count cT    %cP    %cT    cT + cP";
    }
    output << endl << endl;

    count = 1;
    cout.precision(4);
    output.precision(4);
    double percentage;
    int countP, countT, countTandP;
    countP = countT = 0;
    for (itRoutes = routes.begin(); itRoutes != routes.end(); itRoutes++) {
        if (useLeftAndRightRoadAsSamePlace) {
            routeTmp = itRoutes->first;
            fristPart = routeTmp.substr(0, 3);
            middlePart = "to"; //routeTmp.substr(3, 2);
            lastPart = routeTmp.substr(5, 3);
            routeTmp = lastPart + middlePart + fristPart;

            if (count < 10) {
                output << "Street_Segment_0" << count << ": " << itRoutes->first << " | " << routeTmp;
            } else {
                output << "Street_Segment_" << count << ": " << itRoutes->first << " | " << routeTmp;
            }
        } else {
            if (count < 10) {
                output << "Street_Segment_0" << count << ": " << itRoutes->first;
            } else {
                output << "Street_Segment_" << count << ": " << itRoutes->first;
            }
        }

        countTandP = itRoutes->second.categoryP + itRoutes->second.categoryT;
        output << "    cT + cP: " << countTandP;
        if (countTandP < 10) {
            output << "  ";
        } else if (countTandP < 100){
            output << " ";
        }

        output<< "    cP: " << itRoutes->second.categoryP;
        if (itRoutes->second.categoryP < 10) {
            output << "  ";
        } else if (itRoutes->second.categoryP < 100){
            output << " ";
        }

        output << "    cT: " << itRoutes->second.categoryT;
        if (itRoutes->second.categoryT < 10) {
            output << "  ";
        } else if (itRoutes->second.categoryT < 100){
            output << " ";
        }

        countP += itRoutes->second.categoryP;
        percentage = (double(itRoutes->second.categoryP)/countTandP) * 100;
        output << "   %cP: " << percentage;

        countT += itRoutes->second.categoryT;
        percentage = (double(itRoutes->second.categoryT)/countTandP) * 100;
        output << "    %cT: " << percentage << endl;

        count++;
     }

    output << endl << "CountTotal: " << (countP + countT) << " countP: " << countP << "    countT: " << countT;
    countTandP = countP + countT;
    percentage = double(countP)/countTandP;
    output << endl << endl << "Porcentagem geral, %%GcP: " << percentage;
    percentage = double(countT)/countTandP;
    output << "    %%GcT: " << percentage << endl;

    output.close();
    cout << "Distribuição de veículos pelos segmentos de rotas salvas no arquivo " << fileDist << endl << endl;
    return lineCount;
}
