//#########################################################################################
//                                         To compile:                                    #
// g++ -std=c++0x -o generate_fields_packet_website.out generate_fields_packet_website.cc #
//#########################################################################################

// Scritp to generate the text for generate the packat-image in 
// the website http://interactive.blockdiag.com/packetdiag/

#include <iostream>
#include <fstream>

using namespace std;

int main() {
   int size, countFields, countLimit, fieldsValue;
   string nome;

   cout << "Digite a quantidade de campos: ";
   cin >> countLimit;
   countFields = 1;
   fieldsValue = 0;

   std::ofstream myfile;
   myfile.open("diagram.txt");
   myfile << "packetdiag {" << endl;
   myfile << "    #colwidth = 16" << endl;
   myfile << "    #node_height = 80" << endl;
   myfile << "    #node_width = 20" << endl << endl;

   while (countFields <= countLimit) {
      while (cin.get() != '\n') {
         continue;
      }

      cout << endl << "Nome_" << countFields << ": ";
      getline(cin, nome);
      cout << "Tamanho: ";
      cin >> size;

      myfile << "    " << fieldsValue << "-" << (fieldsValue + size - 1) << ": " << nome << endl;
      fieldsValue += size;

      countFields++;
   }

   myfile << "}";
   cout << endl << "\tDiagram saved in the file diagram.txt" << endl << endl;
   myfile.close();

   return 0;
}