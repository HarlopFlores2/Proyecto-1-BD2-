//
// Created by VIRGINIA on 29/04/2023.
//

#include "extendibleHash.h"

int main(){
    extendibleHash<Record> ExtendibleHash;
    ExtendibleHash.load();

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