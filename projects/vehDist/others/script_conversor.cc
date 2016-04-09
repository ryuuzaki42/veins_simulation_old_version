//#############################################################
//                      To compile:                           #
// g++ -std=c++0x -o script_conversor.out script_conversor.cc #
//#############################################################

#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

int main(){
    freopen("test.rou.xml","r",stdin); //Arquivo de entrada gerado com script randomTrips.py

    ofstream output;
    output.open("test_end.rou.xml"); //Arquivo que será criado com todas rotas

    //Escrita da definição do tipo de veículo no arquivo de saída
    output << "<routes>\n    <vType id=\"vtype0\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"1,1,0\"/>\n\n";

    int count = 1;
    int countRoutes = 80;
    string route, line;
    unordered_map<string, int>::iterator it;
    unordered_map<string, int> routes;
    bool print = false;

    cout  << endl << "Por favor espere, gerando rotas..." << endl << endl;

    while (getline(cin,line)){
        if (line.compare(0,15,"        <route ") == 0){ // Edita cada linha do arquivo de entrada que representa rotas
            string to = "    <route id=\"";
            to +="route" + to_string(count);
            to +="\"";
            to += line.substr(14) + "\n";
            // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], como uma rua tem 250 m, logo 1 km => 4 *9 => 36; 10 km => 360. 360 + 26 = 386
            if (to.size() > 3700){ // logo X km, x * 36 + 26; Para 100 km, 100 * 36 + 26 = 3626, para ter certeza 3700
                route = line.substr(22,8); // Pega o primeiro ponto (ponto de partida)
                it = routes.find(route);
                if (it != routes.end()){ // Testa se ele já foi inserido ou existe
                    if (it->second < countRoutes) {
                        it->second += 1; // Incrementa a quantidade desta rota
                        print = true;
                    }
                } else{
                    routes.insert(make_pair(route, 1));
                    print = true;
                }
            } else {
                //cout << endl << "to.size, menor que 3700: "<< to.size() << endl;
            }

            if(print){
                output << to; //Escrita da rota no arquivo de saída
                //cout << to; //Imprime rotas geradas no terminal
                count++;
                print = false;
            }
        }
    }
    output << endl;

    output << "</routes>"; //Finalização do arquivo de saída
    cout << endl;
    int routesCount = 0;
    for (it = routes.begin(); it != routes.end(); it++){
        if (it->second == 80){
            cout << it->first << " " << it->second << endl;
        } else {
            cout <<"    " << it->first << " " << it->second << endl;
        }
        routesCount += it->second;
    }

    cout << endl << "Sum start points: " << routes.size() << endl;
    cout << "Sum routes: " << routesCount << endl << endl;
    return 0;
}