//
// Created by VIRGINIA on 29/04/2023.
//

#include "extendibleHash.h"
#include <chrono>
using namespace std::chrono;

void test1(){
    extendibleHash<Record> ExtendibleHash;
    ExtendibleHash.load("../pruebaHash100.csv");
    vector<string> f1 = {"1", "6", "3", "4", "5", "6", "7", "8"};
    vector<string> f2 = {"11", "8", "31", "41", "51", "61", "71", "81"};
    vector<string> f3 = {"11", "3", "31", "41", "51", "61", "71", "81"};
    vector<string> f4 = {"1", "1", "3", "4", "5", "6", "7", "8"};
    vector<string> f5 = {"11", "2", "31", "41", "51", "61", "71", "81"};
    Record record1,record2,record3,record4,record5,record6,record7,record8,record9;
    record1.load(f1);
    record2.load(f2);
    record3.load(f3);
    record4.load(f4);
    record5.load(f5);;
    ExtendibleHash.insert(record1);
    ExtendibleHash.insert(record2);
    ExtendibleHash.insert(record3);
    ExtendibleHash.insert(record4);
    ExtendibleHash.insert(record5);
    ExtendibleHash.printAllBuckets();
}

void test2(){
    extendibleHash<Record> ExtendibleHash;
    //ExtendibleHash.load("../pruebaHash100.csv");
    int searchKey = 6;
    cout << "------" << endl;
    vector<Record> searchResults = ExtendibleHash.search(searchKey);

    if (searchResults.empty()) {
        cout << "No se encontraron registros con documentID: " << searchKey << endl;
    } else {
        cout << "Registros encontrados con documentID: " << searchKey << endl;
        for (Record &record : searchResults) {
            record.print();
            cout << "------" << endl;
        }
    }

}

int main(){
    test1();
    //test2();
    //test3();;
    extendibleHash<Record> ExtendibleHash;
    //ExtendibleHash.load("../prueba1000.csv");
    /*          100 REGISTROS         */
    /*
    auto start = high_resolution_clock::now();
    ExtendibleHash.load("../prueba1000.csv");
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop-start);
    cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    cout << "Cantidad de accesos: " << countRead + countWrite << '\n';

    auto start = high_resolution_clock::now();
    vector<Record> v1 = ExtendibleHash.search(10);
    vector<Record> v2 = ExtendibleHash.search(23);
    vector<Record> v3 = ExtendibleHash.search(51);
    vector<Record> v4 = ExtendibleHash.search(73);
    vector<Record> v5 = ExtendibleHash.search(97);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop-start);
    cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    cout << "Cantidad de accesos: " << countRead + countWrite << '\n';
    */
    /*          200 REGISTROS       */
    /*
    ExtendibleHash.load("../pruebaHash200.csv");

    auto start = high_resolution_clock::now();
    ExtendibleHash.insert(record1);
    ExtendibleHash.insert(record2);
    ExtendibleHash.insert(record3);
    ExtendibleHash.insert(record4);
    ExtendibleHash.insert(record5);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop-start);
    cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    cout << "Cantidad de accesos: " << countRead + countWrite << '\n';

    auto start = high_resolution_clock::now();
    vector<Record> v1 = ExtendibleHash.search(10);
    vector<Record> v2 = ExtendibleHash.search(23);
    vector<Record> v3 = ExtendibleHash.search(51);
    vector<Record> v4 = ExtendibleHash.search(73);
    vector<Record> v5 = ExtendibleHash.search(97);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop-start);
    cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    cout << "Cantidad de accesos: " << countRead + countWrite << '\n';
     */

    /*          500 REGISTROS        */
    /*
    ExtendibleHash.load("../pruebaHash500.csv");

    auto start = high_resolution_clock::now();
    ExtendibleHash.insert(record1);
    ExtendibleHash.insert(record2);
    ExtendibleHash.insert(record3);
    ExtendibleHash.insert(record4);
    ExtendibleHash.insert(record5);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop-start);
    cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    cout << "Cantidad de accesos: " << countRead + countWrite << '\n';

    auto start = high_resolution_clock::now();
    vector<Record> v1 = ExtendibleHash.search(10);
    vector<Record> v2 = ExtendibleHash.search(23);
    vector<Record> v3 = ExtendibleHash.search(51);
    vector<Record> v4 = ExtendibleHash.search(73);
    vector<Record> v5 = ExtendibleHash.search(97);
    auto stop = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(stop-start);
    cout << "Execution Time: " << time.count() << " milliseconds" << endl;
    cout << "Cantidad de accesos: " << countRead + countWrite << '\n';
    */

}