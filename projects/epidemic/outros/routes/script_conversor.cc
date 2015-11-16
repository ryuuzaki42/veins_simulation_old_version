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
  
  freopen("test.rou.xml","r",stdin); //Arquivo de entrada gerado com script randomTrips.py

  string line;
  ofstream output;
  output.open("test_end.rou.xml"); //Arquivo que será criado com todas rotas e a definição do tipo de veículo

  //output << "<routes>\n\t<vType id=\"Car\" maxSpeed=\"14.0\"/>\n"; //Escrita da definição do tipo de veículo no arquivo de saída

  //Escrita da definição do tipo de veículo no arquivo de saída
  output << "<routes>\n\t<vType id=\"vtype0\" accel=\"2.6\" decel=\"4.5\" sigma=\"0.5\" length=\"2.5\" minGap=\"2.5\" maxSpeed=\"14\" color=\"1,1,0\"/>\n";


  int count=0;
  
  while (getline(cin,line)){
    if (line.compare(0,15,"        <route ") == 0){ //Edita cada linha do arquivo de entrada que representa rotas
      string to = "\t<route id=\"";
      to +="route" + to_string(count++);
      to +="\"";
      to += line.substr(14) + "\n";
      // 23 no início [        <route edges="] + 3 no final ["/>] => 26; 1 tem 9 [1/2to1/1 ], logo 1 km => 4 *9 => 36; 10 km => 360. 360 + 26 = 386
      // logo to.size tem que ter pelo menos 386.
      if (to.size() > 400 ){
	output << to; //Escrita da rota no arquivo de saída
        //cout << endl << "to.size: "<< to.size() << endl;
        cout << to; //Imprimir rotas geradas no terminal
      }else{
	cout << endl << "to.size, menor que 400: "<< to.size() << endl;
	count--;
      }
    }
  }

  output << "</routes>"; //Finalização do arquivo de saída

  return 0;
}