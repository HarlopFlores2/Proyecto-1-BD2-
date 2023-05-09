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
#include <set>
#include "Record.h"
using namespace std;

#define last(k, n) ((k) & ((1 << (n)) - 1))

int countWrite = 0;
int countRead = 0;

const int globalDepth = 5;
const int globalSize = (1<<globalDepth);
const int maxSizeBucket = 4;

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
    countWrite++;
    return stream;
}


template<typename typeRecord>
istream &operator>>(istream &stream, Bucket<typeRecord> &p) {
    stream.read((char *) &p, sizeof(p));
    countRead++;
    return stream;
}

ostream &operator<<(ostream &stream, indexDepth &p) {
    stream.write((char *) &p, sizeof(p));
    stream << flush;
    countWrite++;
    return stream;
}


istream &operator>>(istream &stream, indexDepth &p) {
    stream.read((char *) &p, sizeof(p));
    countRead++;
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
}

set<string> generar(){
    set<string> ans;
    for(int i=0;i<(1<<globalDepth);i++){
        for(int j=1;j<=globalDepth;j++){
            ans.insert(to_hash(i,j));
        }
    }
    return ans;
}


template<typename typeRecord>
class extendibleHash{
    string hashFile = "../hashFile.dat";
    string indexFile = "../indexFile.dat";
public:
    extendibleHash(){};
    void load(const string & csvFile){
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

        // setear hashFile

        rapidcsv::Document document(csvFile);
        auto len = document.GetRowCount();
        for (int i = 0; i < len; i++) {
            vector<string> row = document.GetRow<string>(i);
            Record temp{};
            temp.load(row);
            this->insert(temp);
        }
        data.close();
        index.close();
    }
    bool insert(typeRecord record){
        fstream index(indexFile,ios::out | ios::in | ios::binary);
        fstream data(hashFile,ios::out | ios::in | ios::binary);
        if (!data || !index) return false;
        vector<typeRecord> searchKey = search(record.getKey());
        if(!searchKey.empty()) {
            cerr << "Ya existe la key insertada\n";
            return false;
        }
        int key = record.getKey() % (1<<globalDepth);
        index.seekg(key * sizeIndex());
        indexDepth temp;
        index >> temp;
        string hashKey = to_hash(key, temp.lenLast);
        Bucket<typeRecord> oldBucket;
        data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
        data >> oldBucket;
        int tlenLast = temp.lenLast;
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
                string id1 = '0' + to_hash(key, tlenLast);
                //cout<<"lenLast: " << temp.lenLast <<endl;
                string id2 = '1' + to_hash(key, tlenLast);
                updateIndex(index, id1);
                updateIndex(index, id2);
                // split
                index.seekg(key * sizeIndex());
                indexDepth temp{};
                index >> temp;
                //cout<<"lenLast: " << temp.lenLast <<endl;
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
        data.close();
        index.close();
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
        data.close();
    }


    void printAllBuckets(){
        set<string> gen = generar();
        for(auto x:gen){
            printBucket(x);
        }
        //cout<<gen.size()<<endl;
    }

    void printBucket(string s){
        fstream index(indexFile, ios::out | ios::in | ios::binary);
        fstream data(hashFile,ios::out | ios::in | ios::binary);
        if (!data || !index) return;
        indexDepth tempIndex;
        index.seekg(btoi(s) * sizeIndex());
        index >> tempIndex;
        //cout << tempIndex.lenLast << endl;
        if (s.size() == tempIndex.lenLast) {
            Bucket<typeRecord> tempBucket;
            data.seekg(btoi(s) * sizeBucket<typeRecord>());
            data >> tempBucket;
            if(tempBucket.size!=0){
                cout<<"--- " << s << " ---" <<endl;
                for (int i = 0; i < tempBucket.size; i++) {
                    tempBucket.records[i].print();
                }
                int nxtPos = tempBucket.nextPosition;
                while (nxtPos != -1){
                    cout<<"-- encadenamiento --\n";
                    data.seekg(nxtPos * sizeBucket<typeRecord>());
                    data >> tempBucket;
                    for (int i = 0; i < tempBucket.size; i++) {
                        tempBucket.records[i].print();
                    }
                    nxtPos = tempBucket.nextPosition;
                }
            }
        }
        data.close();
        index.close();
    }

    vector<typeRecord> search(int key) {
        fstream index(indexFile, ios::in | ios::binary);
        fstream data(hashFile, ios::in | ios::binary);
        if (!data || !index) return {};

        // Calcular el index key
        int indexKey = key % (1 << globalDepth);

        // Buscar la entrada correspondiente en el index
        index.seekg(indexKey * sizeIndex());
        indexDepth temp;
        index >> temp;

        // Obtener la clave hash de los últimos 'lenLast' bits
        string hashKey = to_hash(key, temp.lenLast);

        // Dirigirse al bucket correspondiente en el archivo de datos
        data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
        Bucket<typeRecord> bucket;
        data >> bucket;

        // Crear un vector para almacenar los registros que coincidan con la key
        vector<typeRecord> result;
        // Se busca dentro del bucket correspondiente
        for (int i = 0; i < bucket.size; i++) {
            // Si el registro coincide con la key, agregarlo al vector result
            if (bucket.records[i].getKey() == key)
                result.push_back(bucket.records[i]);
        }

        // Si no se encuentra en el bucket, buscar en los siguientes (solo si hay encadenamiento)
        int nextPosition = bucket.nextPosition;
        while (nextPosition != -1) {
            // Nos dirigimos al siguiente bucket en la cadena
            data.seekg(nextPosition * sizeBucket<typeRecord>());
            data >> bucket;
            // Buscar registros coincidentes en el bucket
            for (int i = 0; i < bucket.size; i++) {
                if (bucket.records[i].getKey() == key)
                    result.push_back(bucket.records[i]);
            }
            // Actualizar la posición
            nextPosition = bucket.nextPosition;
        }
        data.close();
        index.close();
        return result;
    }

    bool remove(int key) {
        fstream index(indexFile, ios::in | ios::binary);
        fstream data(hashFile, ios::in | ios::binary);
        
        int indexKey = key % (1 << globalDepth);

        index.seekg(indexKey * sizeIndex());
        indexDepth temp;
        index >> temp;

        string hashKey = to_hash(key, temp.lenLast);

        data.seekg(btoi(hashKey) * sizeBucket<typeRecord>());
        Bucket<typeRecord> bucket;
        data >> bucket;
        int pos, notFound = 0;

        for (int i = 0; i < bucket.size; i++) {
            if (bucket.records[i].getKey() == key) {
                pos = i;
                break;
            } else {
                notFound++;
            }
        }

        if (notFound == bucket.size && bucket.nextPosition == -1) {
            cerr << "Registro no existe";
            return false;
        } else {
            typeRecord emptyRec;
            bucket.records[pos] = emptyRec;
            bucket.size -= 1;
            data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
            data << bucket;
        }
        
        int newNotFound = -1;
        int nextPosition = bucket.nextPosition;
        while (nextPosition != -1) {
            newNotFound = 0;
            data.seekg(nextPosition * sizeBucket<typeRecord>());
            data >> bucket;            
            for (int i = 0; i < bucket.size; i++) {
                if (bucket.records[i].getKey() == key) {
                    pos = i;
                    break;
                } else {
                    newNotFound++;
                }
            }            
            nextPosition = bucket.nextPosition;
        }

        if (newNotFound == bucket.size) {
            cerr << "Registro no existe";
            return false;
        } else {
            typeRecord emptyRec;
            bucket.records[pos] = emptyRec;
            bucket.size -= 1;
            data.seekp(btoi(hashKey) * sizeBucket<typeRecord>());
            data << bucket;
        }

        index.close();
        data.close();
        return true;
    }
};


#endif //PROYECTO_1_BD2__EXTENDIBLEHASH_H