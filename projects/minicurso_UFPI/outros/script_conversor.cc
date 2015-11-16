#include <sstream> 
#include <string>
#include <iostream>
#include <fstream>

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
  freopen("MyRoutes-tmp.rou.xml","r",stdin); //Arquivo de entrada gerado com script randomTrips.py
    
  string line;
  ofstream output;
  output.open("MyRoutes.rou.xml"); //Arquivo que será criado com todas rotas e a definição do tipo de veículo
  
  output << "<routes>\n\t<vType id=\"Car\" maxSpeed=\"14.0\"/>\n"; //Escrita da definição do tipo de veículo no arquivo de saída
  
  int count=0;
  while (getline(cin,line)){
    if (line.compare(0,15,"        <route ") == 0){ //Edita cada linha do arquivo de entrada que representa rotas
      string to = "\t<route id=\"";
      to +="route" + to_string(count++);
      to +="\"";
      to += line.substr(14) + "\n";
      output << to; //Escrita da rota no arquivo de saída
      // cout << to; //Imprimir rotas geradas no terminal
    }
  }
 
  output << "</routes>"; //Finalização do arquivo de saída
  
  return 0;
}
