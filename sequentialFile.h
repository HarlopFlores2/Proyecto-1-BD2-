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



// 4 + 4 + 11 + 4 + 4 + 4 + 4 + 4 + 4 + 8 = 51
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
    long nextPosition = -1;
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

};

class sequentialFile {
    string dataFile = "../dataFile.dat";
    string auxFile = "../auxFile.dat";
    int maxAuxSize;
public:
    explicit sequentialFile(int maxAuxSize) : maxAuxSize(maxAuxSize) {};
    void load_data(const string&);
    void print_all();
    vector<fixedRecord> search(int key);
    vector<fixedRecord> range_search(int keyBegin, int keyEnd);
    bool insert(fixedRecord record);
    bool remove(int key);
    void merge_data();

};


#endif //PROYECTO_1_BD2_SEQUENTIALFILE_H
