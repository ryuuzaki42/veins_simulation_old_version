//##################################################
//        To compile:
//  g++ -std=c++0x -o script_conversor script_conversor.cc
//##################################################

#include <sstream> 
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <algorithm>    // std::find

using namespace std;

template <typename T>
std::string to_string(T value){
    //create an output string stream
    std::ostringstream os ;
    //throw the value into the string stream
    os << value ;
    //convert the string stream into a string and return
    return os.str() ;
}

int main(){
    freopen("test.rou.xml","r",stdin); //Arquivo de entrada gerado com script randomTrips.py

    string line;
    ofstream output;
    output.open("test_end.rou.xml"); //Arquivo que será criado com todas rotas e a definição do tipo de veículo

    //Escrita da definição do tipo de veículo no arquivo de saída
    output << "<routes>\n    <vType id=\"vtype0\" accel=\"3\" decel=\"5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"15\" color=\"1,1,0\"/>\n\n";

    int count = 0;
    int countRoutes = 80;
    string route;
    unordered_map<string, int>::iterator it;
    unordered_map<string, int> routes;
    bool print = false;;
    while (getline(cin,line)){
        if (line.compare(0,15,"        <route ") == 0){ //Edita cada linha do arquivo de entrada que representa rotas
            string to = "    <route id=\"";
            to +="route" + to_string(count);
            to +="\"";
            to += line.substr(14) + "\n";
            // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], logo 1 km => 4 *9 => 36; 10 km => 360. 360 + 26 = 386
            // logo X km, x * 36 + 26
            // 100 * 36 + 26 = 3626
            if (to.size() > 3700){
                route = line.substr(22,8);
                if (routes.empty()){
                    routes.insert(make_pair(route, 1));
                    print = true;
                } else {
                    it = routes.find(route);
                    if (it != routes.end()){
                        if(it->second < countRoutes) {
                            it->second += 1;
                            print = true;
                        }
                    } else{
                        routes.insert(make_pair(route, 1));
                       print = true;
                    }
                }
            } else {
                cout << endl << "to.size, menor que 3700: "<< to.size() << endl;
            }

            if(print){
                output << to; //Escrita da rota no arquivo de saída
                cout << to; //Imprimir rotas geradas no terminal
                count++;
                print = false;
            }
        }
    }
    output << endl;

    //count=0;
    //int quantVeh = 50;
    //for (int i = 0; i < quantVeh; i++){
    //    output << "    <vehicle depart=\"0\" type=\"vtype0\" id=\"veh" << i << "\" route=\"route" << i << "\" />\"" << endl;
    //}
    //output << endl;

    output << "</routes>"; //Finalização do arquivo de saída
    cout << endl;
    for (it = routes.begin(); it != routes.end(); it++){
        if (it->second == 80){
            cout << it->first << " " << it->second << endl;
        } else {
          cout <<"    " << it->first << " " << it->second << endl;
        }
    }

    cout << endl << "Sum start points: " << routes.size() << endl;
    return 0;
}
