//###################################################################
//                           To compile:                            #
// g++ -std=c++0x -Wall -o polygon_generate.out polygon_generate.cc #
//###################################################################

#include <iostream>
#include <fstream>

using namespace std;

int printObstaclesAxis(int idPolygon, int ya, int xa, int xb, int countKm) {
    int xc, xd, yb, yc, yd;
    ofstream myfile2;
    myfile2.open ("vehDist.poly.xml", std::ios_base::app);

    for(int i = 0; i < 5; i++) {
        xd = xc = xb+38;
        yc = yb = ya+38;
        yd = ya;

        myfile2 << "    <poly id=\"" << idPolygon << "\" type=\"building\" color=\"190,190,190\" fill=\"1\" layer=\"1\" shape=\"";
        myfile2 << xa << "," << ya << " " << xb << "," << yb << " " << xc << "," << yc;
        myfile2 << " " << xd << "," << yd << " " << xa << "," << ya << "\"/>" << endl;

        ya = yb + 10;
        idPolygon++;
    }

    myfile2.close();
    return yb;
}

int corraParaAsColinas(int idPolygon, int ya, int xa, int xb, int countKm) {
    int tmpInt, forEnd;
    forEnd = 4 * countKm;
    for(int i = 0; i < forEnd; i++) {
        tmpInt = printObstaclesAxis(idPolygon, ya, xa, xb, countKm);

        idPolygon += countKm * 5;
        ya = tmpInt + 20;
    }
    return idPolygon;
}

int main() {
    int xa, xb, ya, idPolygon, count;
    int countKm, countLimt;
    ofstream myfile;
    string fileOutput = "vehDist.poly.xml";
    count = idPolygon = 1;

    countKm = 1; // 1 km of grid

    myfile.open (fileOutput);
    myfile << "<shapes>" << endl << endl;
    myfile << "    <!-- File with " << countKm << " km^2 of grid polygons -->" << endl << endl;
    myfile.close();

    cout << endl << "Por favor espere, gerando polygons..." << endl;

    countLimt = countKm * 20;
    ya = xa = xb = 10;
    for (int i = 1; i < 6; i++) {
        idPolygon = corraParaAsColinas(idPolygon, ya, xa, xb, countKm);
        xb += 48;
        xa = xb;

         if (i == 5) {
            xb += 10;
            xa = xb;
            i = 0;
         }

         if (count == countLimt) {
             break;
         }
         count++;
    }

    myfile.open ("vehDist.poly.xml", std::ios_base::app);
    myfile << endl << "</shapes>";
    myfile.close();

    cout << endl << "Polygons saved in the file " << fileOutput << endl << endl;
}
