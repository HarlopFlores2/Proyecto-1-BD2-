#include "sequentialFile.h"
#include "SFML/Graphics.hpp"


int main(){
    vector<string> f1 = {"1", "20", "3", "4", "5", "6", "7", "8"};
    vector<string> f2 = {"11", "40", "31", "41", "51", "61", "71", "81"};
    vector<string> f3 = {"11", "3", "31", "41", "51", "61", "71", "81"};
    fixedRecord<Record, int> fixedRecord1, fixedRecord2, fixedRecord3;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    fixedRecord3.load(f3);
    sequentialFile<Record, int> SequentialFile(2);

    //Prueba 1 - limpiar campos en data y aux

    //SequentialFile.load_data("../prueba.csv");

    //SequentialFile.insert(fixedRecord1);
    //SequentialFile.insert(fixedRecord2);
    //SequentialFile.insert(fixedRecord3);
    //SequentialFile.readRecordAux(0);
    //SequentialFile.readRecordAux(1);
    //SequentialFile.readRecordData(0);
    //SequentialFile.print_all("../dataFile.dat");
    SequentialFile.readRecordAux(0);
    //cout << SequentialFile.findLocation(0).second << endl;




}