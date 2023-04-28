//
// Created by VIRGINIA on 28/04/2023.
//

#ifndef PROYECTO_1_BD2__EXTENDIBLEHASH_H
#define PROYECTO_1_BD2__EXTENDIBLEHASH_H


#include <iostream>
#include <vector>
using namespace std;

#define last(k, n) ((k) & ((1 << (n)) - 1))

struct Bucket{

};

template<typename typeRecord, typename typeKey>
class extendibleHash{
    int maxDepth;
    int sizeBucket;
    vector<Bucket*> buckets;
    string fileName = "../hashData.dat";



};

#endif //PROYECTO_1_BD2__EXTENDIBLEHASH_H
