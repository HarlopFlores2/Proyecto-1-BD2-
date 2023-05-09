#include "sequentialFile.h"
#include <chrono>
using namespace std::chrono;

void test1(){
    vector<string> f1 = {"1", "1", "24/04/2023", "2736", "42", "6", "5", "4"};
    vector<string> f2 = {"11", "7", "25/04/2023", "41", "51", "61", "71", "81"};
    vector<string> f3 = {"15", "29", "26/04/2023", "43", "51", "61", "71", "81"};
    vector<string> f4 = {"18", "848", "27/04/2023", "43", "51", "61", "71", "81"};
    vector<string> f5 = {"23", "939", "28/04/2023", "43", "51", "61", "71", "81"};
    fixedRecord<Record, int> fixedRecord1, fixedRecord2, fixedRecord3, fixedRecord4, fixedRecord5;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    fixedRecord3.load(f3);
    fixedRecord4.load(f4);
    fixedRecord5.load(f5);
    sequentialFile<Record, int> SequentialFile(6);
    SequentialFile.load_data("../prueba100.csv");
    SequentialFile.insert(fixedRecord1);
    SequentialFile.insert(fixedRecord2);
    SequentialFile.insert(fixedRecord3);
    SequentialFile.insert(fixedRecord4);
    SequentialFile.insert(fixedRecord5);
    SequentialFile.print_all();
}

void test2(){
    vector<string> f1 = {"1", "1", "24/04/2023", "2736", "42", "6", "5", "4"};
    vector<string> f2 = {"11", "7", "25/04/2023", "41", "51", "61", "71", "81"};
    vector<string> f3 = {"15", "29", "26/04/2023", "43", "51", "61", "71", "81"};
    vector<string> f4 = {"18", "848", "27/04/2023", "43", "51", "61", "71", "81"};
    vector<string> f5 = {"23", "939", "28/04/2023", "43", "51", "61", "71", "81"};
    fixedRecord<Record, int> fixedRecord1, fixedRecord2, fixedRecord3, fixedRecord4, fixedRecord5;
    fixedRecord1.load(f1);
    fixedRecord2.load(f2);
    fixedRecord3.load(f3);
    fixedRecord4.load(f4);
    fixedRecord5.load(f5);
    sequentialFile<Record, int> SequentialFile(3);
    SequentialFile.load_data("../prueba100.csv");
    SequentialFile.insert(fixedRecord1);
    SequentialFile.insert(fixedRecord2);
    SequentialFile.insert(fixedRecord3);
    SequentialFile.insert(fixedRecord4);
    SequentialFile.insert(fixedRecord5);
    SequentialFile.removeRecord(7);
    SequentialFile.removeRecord(848);
    SequentialFile.print_all();
}

void test3(){
    sequentialFile<Record, int> SequentialFile(3);
    SequentialFile.load_data("../prueba1000.csv");
    vector<fixedRecord<Record,int>> records = SequentialFile.search(8);
    for(auto x: records){
        x.print();
    }
}

void test4(){
    sequentialFile<Record, int> SequentialFile(3);
    SequentialFile.load_data("../prueba1000.csv");
    vector<fixedRecord<Record,int>> records = SequentialFile.range_search(7, 12);
    for(auto x: records){
        x.print();
    }
}


int main(){
    //test1();
    //test2();
    //test3();
    //test4();
    /*          1000 REGISTROS         */
    // SequentialFile.load_data("./prueba1000.csv");

    // auto start = high_resolution_clock::now();
    // SequentialFile.insert(fixedRecord1);
    // SequentialFile.insert(fixedRecord2);
    // SequentialFile.insert(fixedRecord3);
    // SequentialFile.insert(fixedRecord4);
    // SequentialFile.insert(fixedRecord5);
    // auto stop = high_resolution_clock::now();
    // auto time = duration_cast<milliseconds>(stop-start);
    // cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    // cout << "Cantidad de accesos: " << countRead + countWrite << '\n';

    // auto start = high_resolution_clock::now();
    // vector<fixedRecord<Record,int>> v1 = SequentialFile.search(0);
    // vector<fixedRecord<Record,int>> v2 = SequentialFile.search(365);
    // vector<fixedRecord<Record,int>> v3 = SequentialFile.search(599);
    // vector<fixedRecord<Record,int>> v4 = SequentialFile.search(666);
    // vector<fixedRecord<Record,int>> v5 = SequentialFile.search(927);
    // auto stop = high_resolution_clock::now();
    // auto time = duration_cast<milliseconds>(stop-start);
    // cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    // cout << "Cantidad de accesos: " << countRead + countWrite << '\n';



    /*          5000 REGISTROS       */
    // SequentialFile.load_data("./prueba5000.csv");

    // auto start = high_resolution_clock::now();
    // SequentialFile.insert(fixedRecord1);
    // SequentialFile.insert(fixedRecord2);
    // SequentialFile.insert(fixedRecord3);
    // SequentialFile.insert(fixedRecord4);
    // SequentialFile.insert(fixedRecord5);
    // auto stop = high_resolution_clock::now();
    // auto time = duration_cast<milliseconds>(stop-start);
    // cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    // cout << "Cantidad de accesos: " << countRead + countWrite << '\n';


    // auto start = high_resolution_clock::now();
    // vector<fixedRecord<Record,int>> v1 = SequentialFile.search(0);
    // vector<fixedRecord<Record,int>> v2 = SequentialFile.search(365);
    // vector<fixedRecord<Record,int>> v3 = SequentialFile.search(599);
    // vector<fixedRecord<Record,int>> v4 = SequentialFile.search(666);
    // vector<fixedRecord<Record,int>> v5 = SequentialFile.search(927);
    // auto stop = high_resolution_clock::now();
    // auto time = duration_cast<milliseconds>(stop-start);
    // cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    // cout << "Cantidad de accesos: " << countRead + countWrite << '\n';



    /*          10000 REGISTROS        */
    // SequentialFile.load_data("./prueba10000.csv");

    // auto start = high_resolution_clock::now();
    // SequentialFile.insert(fixedRecord1);
    // SequentialFile.insert(fixedRecord2);
    // SequentialFile.insert(fixedRecord3);
    // SequentialFile.insert(fixedRecord4);
    // SequentialFile.insert(fixedRecord5);
    // auto stop = high_resolution_clock::now();
    // auto time = duration_cast<milliseconds>(stop-start);
    // cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    // cout << "Cantidad de accesos: " << countRead + countWrite << '\n';


    // auto start = high_resolution_clock::now();
    // vector<fixedRecord<Record,int>> v1 = SequentialFile.search(0);
    // vector<fixedRecord<Record,int>> v2 = SequentialFile.search(365);
    // vector<fixedRecord<Record,int>> v3 = SequentialFile.search(599);
    // vector<fixedRecord<Record,int>> v4 = SequentialFile.search(666);
    // vector<fixedRecord<Record,int>> v5 = SequentialFile.search(13755);
    // auto stop = high_resolution_clock::now();
    // auto time = duration_cast<milliseconds>(stop-start);
    // cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    // cout << "Cantidad de accesos: " << countRead + countWrite << '\n';

    return 0;
}