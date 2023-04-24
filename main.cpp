#include "sequentialFile.h"
#include <unistd.h>

int main(){
    vector<string> f1 = {"1", "2", "3", "4", "5", "6", "7", "8"};
    vector<string> f2 = {"11", "4", "31", "41", "51", "61", "71", "81"};
    fixedRecord fixedRecord1, fixedRecord2;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    sequentialFile SequentialFile(2);
    //SequentialFile.load_data("../prueba.csv");
    //SequentialFile.readRecordData(2);
    SequentialFile.insert(fixedRecord1);
    SequentialFile.readRecordAux(0);
    //cout << SequentialFile.findLocation(0).second << endl;




}