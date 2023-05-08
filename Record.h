#ifndef PROYECTO_1_BD2__RECORD_H
#define PROYECTO_1_BD2__RECORD_H

#include <iostream>
#include <vector>
using namespace std;

struct Record {
    int line;
    long documentID;
    char date[11];
    int productID;
    float price;
    float discount;
    int customer;
    int quantity;
    void load(vector<string> data){
        line = stoi(data[0]);
        documentID = stol(data[1]);
        strcpy(date, data[2].c_str());
        productID = stoi(data[3]);
        price = stof(data[4]);
        discount = stof(data[5]);
        customer = stoi(data[6]);
        quantity = stoi(data[7]);
    }
    long getKey() {return documentID;}
    void print(){
        cout << "documentID: " << documentID << endl;
        cout << "date: " << date << endl;
        cout << "productID: " << productID << endl;
        cout << "price: " << price << endl;
        cout << "discount: " << discount << endl;
        cout << "customer: " << customer << endl;
        cout << "quantity: " << quantity << endl;
    }
};

#endif //PROYECTO_1_BD2__RECORD_H
