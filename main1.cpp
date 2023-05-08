//
// Created by VIRGINIA on 29/04/2023.
//

#include "extendibleHash.h"

int main(){
    vector<string> f1 = {"1", "2", "3", "4", "5", "6", "7", "8"};
    Record fixedRecord1;
    fixedRecord1.load(f1);
    extendibleHash<Record> ExtendibleHash;

    ExtendibleHash.load("../pruebaHash100.csv");


    ExtendibleHash.printAllBuckets();
    /*
    //Prueba de la funcion search
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
    */

}