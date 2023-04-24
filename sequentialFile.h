//
// Created by VIRGINIA on 22/04/2023.
//

#ifndef PROYECTO_1_BD2_SEQUENTIALFILE_H
#define PROYECTO_1_BD2_SEQUENTIALFILE_H


#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <cstring>
#include <fstream>
#include "rapidcsv.h"

using namespace std;



// 4 + 4 + 12 + 4 + 4 + 4 + 4 + 4 + 4 + 8 = 48
struct fixedRecord{
    int line;
    int documentID;
    char date[11];
    int productID;
    float price;
    float discount;
    int customer;
    int quantity;
    int whatFile = 0; // 0 = data, 1 = aux
    int nextFile = 0;
    long nextPosition = -1;
    int  deleted = 0;
    void load(vector<string> data){
        line = stoi(data[0]);
        documentID = stoi(data[1]);
        strcpy(date, data[2].c_str());
        productID = stoi(data[3]);
        price = stof(data[4]);
        discount = stof(data[5]);
        customer = stoi(data[6]);
        quantity = stoi(data[7]);
    }
    int getKey() {return documentID;}
    void print(){
        cout << "documentID: " << documentID << endl;
        cout << "date: " << date << endl;
        cout << "productID: " << productID << endl;
        cout << "price: " << price << endl;
        cout << "discount: " << discount << endl;
        cout << "customer: " << customer << endl;
        cout << "quantity: " << quantity << endl;
        cout << "whatFile: " << whatFile << endl;
        cout << "nextFIle: " << nextFile << endl;
        cout << "nextPosition: " << nextPosition << endl;
        cout << "deleted: " << deleted << endl;
    }

};

class sequentialFile {
    string dataFile = "../dataFile.dat";
    string auxFile = "../auxFile.dat";
    int maxAuxSize;
    int sizeAux = 0;
public:
    explicit sequentialFile(int maxAuxSize) : maxAuxSize(maxAuxSize) {};
    void load_data(const string&);
    void readRecordData(int pos);
    void readRecordAux(int pos);
    void print_all();
    pair<int,int> findLocation(int key);
    vector<fixedRecord> search(int key);
    vector<fixedRecord> range_search(int keyBegin, int keyEnd);
    bool insert(fixedRecord record);
    bool remove(int key);
    void merge_data();
    int sizeRecord(){
        return sizeof(int) * 8 + sizeof(float) * 2 + sizeof(long) + 12;
    }
};


#endif //PROYECTO_1_BD2_SEQUENTIALFILE_H
