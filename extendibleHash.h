//
// Created by VIRGINIA on 28/04/2023.
//

#ifndef PROYECTO_1_BD2__EXTENDIBLEHASH_H
#define PROYECTO_1_BD2__EXTENDIBLEHASH_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <cstring>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "rapidcsv.h"
#include <vector>
#include <map>
using namespace std;

#define last(k, n) ((k) & ((1 << (n)) - 1))

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

const int globalDepth = 10;
const int globalSize = (1<<10);
const int maxSizeBucket = 2;

template<typename typeRecord>
struct Bucket {
    int size = 0; // size actual
    typeRecord records[globalDepth]={};
    int nextPosition = -1;
};


struct indexDepth{
    char bin[globalDepth];
    int lenLast;
    void print(){
        cout << "bin: ";
        for(int i=0;i<globalDepth;i++){
            cout << bin[i];
        }
        cout << endl;
        cout << "lenLast: " << lenLast << endl;
    }
};

int sizeIndex(){
    return sizeof(indexDepth);
}

template<typename typeRecord>
ostream &operator<<(ostream &stream, Bucket<typeRecord> &p) {
    stream.write((char *) &p, sizeof(p));
    stream << flush;
    return stream;
}


template<typename typeRecord>
istream &operator>>(istream &stream, Bucket<typeRecord> &p) {
    stream.read((char *) &p, sizeof(p));
    return stream;
}

ostream &operator<<(ostream &stream, indexDepth &p) {
    stream.write((char *) &p, sizeof(p));
    stream << flush;
    return stream;
}


istream &operator>>(istream &stream, indexDepth &p) {
    stream.read((char *) &p, sizeof(p));
    return stream;
}

string to_hash(int key, int depth){
    int last = (key & ((1 << depth) - 1));
    int pos = 0;
    string hash = "";
    while(pos < depth and last > 0){
        hash += to_string(last%2);
        last /= 2;
        pos++;
    }
    string ans = "";
    while(pos < depth){
        ans+='0';
        pos++;
    }
    reverse(hash.begin(),hash.end());
    ans += hash;
    return ans;
}



void updateIndex(fstream &f, string h){
    indexDepth temp;
    int j = globalDepth - h.size(), pos = 0;
    while(pos < globalSize){
        int sz = 0;
        bool ok = true;
        f.seekg(pos * sizeIndex());
        f >> temp;
        for(int i=j;i<globalDepth;i++){
            if(temp.bin[i]!=h[sz]) {
                ok = false;
                break;
            }
            sz++;
        }
        if(ok) {
            temp.lenLast++;
            f.seekp(pos * sizeIndex());
            f << temp;
        }
        pos++;
    }
}



template<typename typeRecord>
class extendibleHash{
    map<string,Bucket<typeRecord>> buckets;
    string fileName = "../hashFile.dat";
public:
    extendibleHash(){};
    void load(){
        fstream index("../indexFile.dat",ios::out | ios::binary);
        for(int i=0;i<(1<<globalDepth);i++){
            indexDepth indexD{};
            string temp = to_hash(i,globalDepth);
            for(int j=0;j<globalDepth;j++){
                indexD.bin[j] = temp[j];
            }
            indexD.lenLast = 1;
            index.seekp(i * sizeIndex());
            index << indexD;
        }
    }
    void insert(typeRecord record){
        fstream index("../indexFile.dat",ios::out | ios::in | ios::binary);
        int key = record.getKey() % (1<<globalDepth);
        index.seekg(key * sizeIndex());
        indexDepth temp;
        index >> temp;
        string hashKey = to_hash(key, temp.lenLast);
        if(buckets[hashKey].size == maxSizeBucket){
            // encadenar
            if(hashKey.size() == globalDepth){

            }
            // actualizar index
            string id1 = '0' + to_hash(key, temp.lenLast);
            string id2 = '1' + to_hash(key, temp.lenLast);
            updateIndex(index, id1);
            updateIndex(index, id2);
            // split
            index.seekg(key * sizeIndex());
            indexDepth temp;
            index >> temp;
            for(int i=0;i<buckets[hashKey].size;i++){
                string newHashKey = to_hash( buckets[hashKey].records[i].getKey(), temp.lenLast);
                buckets[newHashKey].records[buckets[newHashKey].size++] = buckets[hashKey].records[i];
            }
            insert(record);
            // vaciar
            buckets[hashKey].size = 0;
        }else{
            index.seekg(key * sizeIndex());
            indexDepth temp;
            index >> temp;
            string hashKey = to_hash(key, temp.lenLast);
            buckets[hashKey].records[buckets[hashKey].size++] = record;
        }
    }

    void readIndex(int pos){
        fstream index("../indexFile.dat",ios::out | ios::in | ios::binary);
        index.seekg(pos * sizeIndex());
        indexDepth temp;
        index >> temp;
        temp.print();
    }

    void printBucket(string s){
        cout << s << endl;
        for(int i=0;i<buckets[s].size;i++){
            cout << buckets[s].records[i].getKey() << " ";
        }
        cout << endl;
    }

    void printall(string f){
        fstream fs(f,ios::out | ios::in | ios::binary);
        indexDepth tmp;
        int pos = 0;
        while(fs >> tmp  and pos <=10) {
            tmp.print();
            pos++;
        }
    }

};


#endif //PROYECTO_1_BD2__EXTENDIBLEHASH_H
