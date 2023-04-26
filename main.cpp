#include "sequentialFile.h"
#include <unistd.h>
#include "SFML/Graphics.hpp"

int main(){
    vector<string> f1 = {"1", "2", "3", "4", "5", "6", "7", "8"};
    vector<string> f2 = {"11", "4", "31", "41", "51", "61", "71", "81"};
    fixedRecord fixedRecord1, fixedRecord2;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    sequentialFile SequentialFile(3);
    /*
    Prueba 1 - limpiar campos en data y aux
    SequentialFile.load_data("../prueba.csv");
    SequentialFile.insert(fixedRecord1);
    SequentialFile.insert(fixedRecord2);
    SequentialFile.readRecordAux(0);
    SequentialFile.readRecordAux(1);
    SequentialFile.readRecordData(0);
     */
    //cout << SequentialFile.findLocation(0).second << endl;




}