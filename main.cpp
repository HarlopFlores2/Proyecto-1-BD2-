#include "sequentialFile.h"
#include<ctime>

int main(){
    vector<string> f1 = {"1", "2", "3", "4", "5", "6", "7", "8"};
    fixedRecord<Record, int> fixedRecord1;
    fixedRecord1.load(f1);
    sequentialFile<Record, int> SequentialFile(2);

    //Prueba 1 - limpiar campos en data y aux
    SequentialFile.load_data("../file_out.csv");

    unsigned t0, t1;

    t0=clock();
    vector<fixedRecord<Record,int>> v = SequentialFile.search(5022);
    t1 = clock();

    double long time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    cout << "Execution Time: " << time << endl;

}