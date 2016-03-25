//#############################################################
//                        To compile:                         #
// g++ -std=c++0x -o polygon_generate.out polygon_generate.cc #
//#############################################################

#include <iostream>
#include <fstream>

using namespace std;

int printObstaclesAxis(int idPolygon, int ya, int xa, int xb){
    int xc, xd, yb, yc, yd;
    ofstream myfile2;
    myfile2.open ("vehDist.poly.xml", std::ios_base::app);

    for(int i=0; i<5; i++) {
        xd = xc = xb+38;
        yc = yb = ya+38;
        yd = ya;

        myfile2 << "    <poly id=\"" << idPolygon << "\" type=\"building\" color=\"190,190,190\" fill=\"1\" layer=\"1\" shape=\"" << xa << "," << ya << " " << xb << "," << yb << " " << xc << "," << yc << " " << xd << "," << yd << " " << xa << "," << ya << "\"/>" << endl;

        ya = yb + 10;
        idPolygon++;
    }

    myfile2.close();
    return yb;
}

int corraParaAsColinas(int idPolygon, int ya, int xa, int xb) {
    int tmpInt;
    for(int i = 0; i < 4; i++) {
        tmpInt = printObstaclesAxis(idPolygon, ya, xa, xb);

        idPolygon += 5;
        ya = tmpInt + 20;
    }
    return idPolygon;
}

int main() {
    int xa, xb, ya, idPolygon, count;
    ofstream myfile;
    count = idPolygon = 1;

    myfile.open ("vehDist.poly.xml");
    myfile << "<shapes>" << endl;
    myfile.close();

    ya = xa = xb = 10;
    for (int i = 1; i < 6;) {
        idPolygon = corraParaAsColinas(idPolygon, ya, xa, xb);
        xb += 48;
        xa = xb;

         if (i == 5){
            xb += 10;
            xa = xb;
            i = 0;
         }

         if (count == 20){
             break;
         }
         i++;
         count++;
    }

    myfile.open ("vehDist.poly.xml", std::ios_base::app);
    myfile << "</shapes>";
    myfile.close();
}
