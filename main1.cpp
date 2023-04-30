//
// Created by VIRGINIA on 29/04/2023.
//

#include "extendibleHash.h"

int main(){
    extendibleHash<Record> ExtendibleHash;
    ExtendibleHash.load();
    // cambien el globalDepth para probar
    /*
    vector<string> f1 = {"1", "6", "3", "4", "5", "6", "7", "8"};
    vector<string> f2 = {"11", "8", "31", "41", "51", "61", "71", "81"};
    vector<string> f3 = {"11", "3", "31", "41", "51", "61", "71", "81"};
    vector<string> f4 = {"1", "1", "3", "4", "5", "6", "7", "8"};
    vector<string> f5 = {"11", "2", "31", "41", "51", "61", "71", "81"};
    vector<string> f6 = {"11", "10", "31", "41", "51", "61", "71", "81"};
    vector<string> f7 = {"1", "11", "3", "4", "5", "6", "7", "8"};
    vector<string> f8 = {"11", "4", "31", "41", "51", "61", "71", "81"};
    vector<string> f9 = {"11", "5", "31", "41", "51", "61", "71", "81"};
    Record record1,record2,record3,record4,record5,record6,record7,record8,record9;
    record1.load(f1);
    record2.load(f2);
    record3.load(f3);
    record4.load(f4);
    record5.load(f5);
    record6.load(f6);
    record7.load(f7);
    record8.load(f8);
    record9.load(f9);
    ExtendibleHash.insert(record1);
    ExtendibleHash.insert(record2);
    ExtendibleHash.insert(record3);
    ExtendibleHash.insert(record4);
    ExtendibleHash.insert(record5);
    ExtendibleHash.insert(record6);
    ExtendibleHash.insert(record7);
    ExtendibleHash.insert(record8);
    ExtendibleHash.insert(record9);
    */
    ExtendibleHash.printBucket("0");
    ExtendibleHash.printBucket("1");
    ExtendibleHash.printBucket("00");
    ExtendibleHash.printBucket("10");
    ExtendibleHash.printBucket("01");
    ExtendibleHash.printBucket("11");
    ExtendibleHash.printBucket("010");
    ExtendibleHash.printBucket("110");
    ExtendibleHash.printBucket("001");
    ExtendibleHash.printBucket("101");
}