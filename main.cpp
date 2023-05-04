#include "sequentialFile.h"
#include <ctime>

int main(){
    vector<string> f1 = {"1", "37", "24/04/2023", "2736", "42.76", "6.92", "5", "4"};
    vector<string> f2 = {"11", "37", "25/04/2023", "41", "51", "61", "71", "81"}; // 637
    vector<string> f3 = {"15", "37", "26/04/2023", "43", "51", "61", "71", "81"};
    fixedRecord<Record, int> fixedRecord1, fixedRecord2, fixedRecord3;
    // fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    // fixedRecord3.load(f3);
    sequentialFile<Record, int> SequentialFile(6);

    unsigned t0, t1;
    double long time;

    /*      100 REGISTROS       */
    // SequentialFile.load_data("./prueba100.csv");

    // t0 = clock();
    // SequentialFile.insert(fixedRecord1);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.006
    
    // cout << countRead + countWrite << '\n';         // 8 + 2 = 10


    // t0 = clock();
    // vector<fixedRecord<Record,int>> v = SequentialFile.search(110);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.001
    
    // cout << countRead + countWrite << '\n';         // 8 + 0 = 8


    // t0 = clock();
    // SequentialFile.removeRecord(2);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.001

    // cout << countRead + countWrite << '\n';         // 14 + 2 = 16


    /*      1000 REGISTROS       */
    // SequentialFile.load_data("./prueba1000.csv");

    // t0 = clock();
    // SequentialFile.insert(fixedRecord2);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.009

    // cout << countRead + countWrite << '\n';         // 10 + 2 = 12


    // t0 = clock();
    // vector<fixedRecord<Record,int>> v = SequentialFile.search(110);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.001

    // cout << countRead + countWrite << '\n';         // 11 + 0 = 11


    // t0 = clock();
    // SequentialFile.removeRecord(2);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.001

    // cout << countRead + countWrite << '\n';         // 20 + 2 = 22


    /*      10000 REGISTROS       */
    // SequentialFile.load_data("./prueba10000.csv");

    // t0 = clock();
    // SequentialFile.insert(fixedRecord2);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.003

    // cout << countRead + countWrite << '\n';         // 14 + 2 = 16


    // t0 = clock();
    // vector<fixedRecord<Record,int>> v = SequentialFile.search(110);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.001

    // cout << countRead + countWrite << '\n';         // 14 + 0 = 14


    // t0 = clock();
    // SequentialFile.removeRecord(2);
    // t1 = clock();
    // time = ((double long)(t1-t0)/CLOCKS_PER_SEC);
    // cout << "Execution Time: " << time << endl;     // 0.002

    // cout << countRead + countWrite << '\n';         // 28 + 2 = 30


    return 0;
}