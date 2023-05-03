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

const int globalDepth = 5;
const int globalSize = (1<<globalDepth);
const int maxSizeBucket = 5;

template<typename typeRecord>
struct Bucket {
    int size = 0; // size actual
    typeRecord records[globalDepth]={};
    int nextPosition = -1;
};

template<typename typeRecord>
int sizeBucket(){
    return sizeof(Bucket<typeRecord>);
}


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

int btoi(string s){
    int ans = 0, temp = 1, n = s.size();
    for(int i=0;i<n;i++){
        ans+=(temp*(s[n-i-1]-'0'));
        temp*=2;
    }
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
    f.close();
}



template<typename typeRecord>
class extendibleHash{
    string hashFile = "../hashFile.dat";
    string indexFile = "../indexFile.dat";
    string csvFile = "../pruebaHash.csv";
public:
    extendibleHash(){};
    void load(){
        fstream index(indexFile, ios::out | ios::binary);
        fstream data(hashFile, ios::out | ios::binary);
        for(int i=0;i<(1<<globalDepth);i++){
            indexDepth indexD{};
            Bucket<typeRecord> BucketT;
            string temp = to_hash(i,globalDepth);
            for(int j=0;j<globalDepth;j++){
                indexD.bin[j] = temp[j];
            }
            indexD.lenLast = 1;
            index.seekp(i * sizeIndex());
            index << indexD;
            data.seekp(i * sizeBucket<typeRecord>());
            data << BucketT;
        }
        data.close();
        index.close();
        // setear hashFile

        rapidcsv::Document document(csvFile);
        auto len = document.GetRowCount();
        for (int i = 0; i < len; i++) {
            vector<string> row = document.GetRow<string>(i);
            Record temp{};
            temp.load(row);
            this->insert(temp);
        }
    }
    void insert(typeRecord record){
        fstream index(indexFile,ios::out | ios::in | ios::binary);
        fstream data(hashFile,ios::out | ios::in | ios::binary);
        int key = record.getKey() % (1<<globalDepth);
        index.seekg(key * sizeIndex());
        indexDepth temp;
        index >> temp;
        string hashKey = to_hash(key, temp.lenLast);
        Bucket<typeRecord> oldBucket;
        data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
        data >> oldBucket;
        if(oldBucket.size == maxSizeBucket){
            if(hashKey.size() == globalDepth) {
                // encadenar
                Bucket<typeRecord> tempBucket;
                Bucket<typeRecord> lastBucket;

                int lastPos = oldBucket.nextPosition;
                if (lastPos==-1){
                    data.seekg(0,ios::end);
                    oldBucket.nextPosition = data.tellg()/ sizeBucket<typeRecord>();
                    data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
                    data << oldBucket;
                    Bucket<typeRecord> lastBucket1;
                    data.seekg(0,ios::end);
                    data.seekp(data.tellg());
                    lastBucket1.records[0] = record;
                    lastBucket1.size++;
                    data << lastBucket1;
                }else {
                    bool ok = false;
                    int auxLastPos = lastPos;
                    while(lastPos != -1){
                        data.seekg(lastPos * sizeBucket<typeRecord>());
                        if(lastPos != -1) auxLastPos = lastPos;
                        data >> tempBucket;
                        if(tempBucket.size<maxSizeBucket){
                            tempBucket.records[tempBucket.size++] = record;
                            data.seekp(lastPos * sizeBucket<typeRecord>());
                            data << tempBucket;
                            ok = true;
                            break;
                        }
                        lastPos = tempBucket.nextPosition;
                    }
                    if(!ok){
                        data.seekg(0,ios::end);
                        tempBucket.nextPosition = data.tellg()/ sizeBucket<typeRecord>();
                        data.seekp(auxLastPos * sizeBucket<typeRecord>());
                        data << tempBucket;
                        data.seekg(0,ios::end);
                        data.seekp(data.tellg());
                        lastBucket.records[0] = record;
                        lastBucket.size++;
                        data << lastBucket;
                    }
                }

            }else{
                // actualizar index
                string id1 = '0' + to_hash(key, temp.lenLast);
                string id2 = '1' + to_hash(key, temp.lenLast);
                updateIndex(index, id1);
                updateIndex(index, id2);
                // split
                index.seekg(key * sizeIndex());
                indexDepth temp{};
                index >> temp;
                data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
                data >> oldBucket;
                int oldSize = oldBucket.size;
                oldBucket.size = 0;
                data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
                data << oldBucket;
                for(int i=0;i<oldSize;i++){
                    Bucket<typeRecord> newBucket;
                    string newHashKey = to_hash( oldBucket.records[i].getKey(), temp.lenLast);
                    data.seekg(btoi(newHashKey) * sizeBucket<typeRecord>());
                    data >> newBucket;
                    newBucket.records[newBucket.size++] = oldBucket.records[i];
                    //buckets[newHashKey].records[buckets[newHashKey].size++] = buckets[hashKey].records[i];
                    data.seekp(btoi(newHashKey) * sizeBucket<typeRecord>());
                    data << newBucket;
                }
                insert(record);
            }
        }else{
            // insertion
            index.seekg(key * sizeIndex());
            indexDepth temp;
            index >> temp;
            string hashKey = to_hash(key, temp.lenLast);
            data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
            data >> oldBucket;
            oldBucket.records[oldBucket.size++] = record;
            data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
            data << oldBucket;
        }
    }

    void readIndex(int pos){
        fstream index(indexFile,ios::out | ios::in | ios::binary);
        index.seekg(pos * sizeIndex());
        indexDepth temp;
        index >> temp;
        temp.print();
    }

    void readHash(int pos){
        fstream data(hashFile,ios::out | ios::in | ios::binary);
        data.seekg(pos * sizeBucket<typeRecord>());
        Bucket<typeRecord> temp;
        data >> temp;
        for(int i=0;i<temp.size;i++){
            temp.records[i].print();
        }
    }

    void printBucket(string s){
        fstream index(indexFile, ios::out | ios::in | ios::binary);
        fstream data(hashFile,ios::out | ios::in | ios::binary);
        cout << s << endl;
        indexDepth tempIndex;
        index.seekg(btoi(s) * sizeIndex());
        index >> tempIndex;
        if (s.size() == tempIndex.lenLast) {
            Bucket<typeRecord> tempBucket;
            data.seekg(btoi(s) * sizeBucket<typeRecord>());
            data >> tempBucket;
            for (int i = 0; i < tempBucket.size; i++) {
                tempBucket.records[i].print();
            }
            int nxtPos = tempBucket.nextPosition;
            while (nxtPos != -1){
                cout<<"encadenamiento\n";
                data.seekg(nxtPos * sizeBucket<typeRecord>());
                data >> tempBucket;
                for (int i = 0; i < tempBucket.size; i++) {
                    tempBucket.records[i].print();
                }
                nxtPos = tempBucket.nextPosition;
            }
        }
        cout << endl;
    }

};


#endif //PROYECTO_1_BD2__EXTENDIBLEHASH_H
