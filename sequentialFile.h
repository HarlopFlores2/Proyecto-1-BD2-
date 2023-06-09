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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "rapidcsv.h"
#include "Record.h"

using namespace std;


template<typename typeRecord, typename typeKey>
struct fixedRecord{
    typeRecord record;
    int nextFile = 0;
    long nextPosition = -1;
    int  deleted = 0;
    void load(vector<string> data){
       record.load(data);
    }
    typeKey getKey() {return record.getKey();}
    void print(){
        record.print();
        cout << "nextFile: " << nextFile << endl;
        cout << "nextPosition: " << nextPosition << endl;
        cout << "deleted: " << deleted << endl;
    }

};

int countWrite = 0;
int countRead = 0;

template<typename typeRecord, typename typeKey>
ostream &operator<<(ostream &stream, fixedRecord<typeRecord,typeKey> &p) {
    stream.write((char *) &p, sizeof(p));
    stream << flush;
    countWrite++;
    return stream;
}


template<typename typeRecord, typename typeKey>
istream &operator>>(istream &stream, fixedRecord<typeRecord,typeKey> &p) {
    stream.read((char *) &p, sizeof(p));
    countRead++;
    return stream;
}


template<typename typeRecord, typename typeKey>
class sequentialFile {
    const char *dataFile = "../dataFile.dat";
    const char *auxFile = "../auxFile.dat";
    int maxAuxSize;
public:
    explicit sequentialFile(int maxAuxSize) : maxAuxSize(maxAuxSize) {};

    void load_data(const string & csvFile) {
        fixedRecord<typeRecord, typeKey> temp, header;
        fstream data(dataFile, ios::out | ios::binary);
        fstream aux(auxFile, ios::out | ios::binary);
        if (!data || !aux) return;
        rapidcsv::Document document(csvFile);
        auto len = document.GetRowCount();
        long offset = 1;
        header.nextFile = 0;
        header.nextPosition = 1;
        data.seekp(0);
        data << header;
        for (int i = 0; i < len; i++) {
            vector<string> row = document.GetRow<string>(i);
            temp.load(row);
            offset++;
            temp.nextPosition = (i == len - 1) ? -1 : offset;
            temp.nextFile = (i == len - 1) ? -1 : 0;
            data.seekp((i + 1) * sizeRecord());
            data << temp;
        }
        data.close();
        aux.close();
    }

    void print_all() {
        fstream filet1(dataFile, ios::in | ios::binary);
        int pos = 0;
        fixedRecord<typeRecord,typeKey> temp1;
        cout << "---- dataFile ---- " << endl;
        while(filet1 >> temp1){
            pos++;
            cout << "-- registro: " << pos << "--" << endl;
            filet1.seekg(pos * sizeRecord());
            temp1.print();
        }
        filet1.close();
        fstream filet2(auxFile, ios::in | ios::binary);
        pos = 0;
        fixedRecord<typeRecord,typeKey> temp2;
        cout << "---- auxFile ---- " << endl;
        while(filet2 >> temp2){
            pos++;
            cout << "-- registro: " << pos << "--" << endl;
            filet2.seekg(pos * sizeRecord());
            temp2.print();
        }
        filet2();
    }

    vector<fixedRecord<typeRecord,typeKey>> search(int key) {
        // Crear un vector para almacenar los resultados
        vector<fixedRecord<typeRecord,typeKey>> results;
        fixedRecord<typeRecord,typeKey> temp;
        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);

        if (!data || !aux) return results; // Si no se pueden abrir los archivos, retorna vacio

        pair<int, int> loc = findLocation(key); // Buscar la ubicación del registro con la clave proporcionada

        // Si el registro se encuentra en el archivo de datos...
        if (loc.first == 0) {
            data.seekg(loc.second * sizeRecord());
            data >> temp;
            // Si la clave coincide y el registro no está marcado como eliminado, agregar al vector
            if (temp.getKey() == key && !temp.deleted) {
                results.push_back(temp);
            }
            // Si el registro se encuentra en el archivo auxiliar
        } else if (loc.first == 1) {
            aux.seekg(loc.second * sizeRecord());
            aux >> temp;
            // Si la clave coincide y el registro no está marcado como eliminado, agregar al vector
            if (temp.getKey() == key && !temp.deleted) {
                results.push_back(temp);
            }
        }
        data.close();
        aux.close();
        return results;
    }

    vector<fixedRecord<typeRecord, typeKey>> range_search(int keyBegin, int keyEnd) {
        vector<fixedRecord<typeRecord, typeKey>> results;
        fixedRecord<typeRecord, typeKey> temp;

        // Si el rango no es válido, retornar vacio
        if (keyBegin > keyEnd) {
            return results;
        }

        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);

        if (!data || !aux) return results; // Si no se pueden abrir los archivos, retorna nada

        pair<int, int> locBegin = findLocation(keyBegin); // Buscar la ubicación del primer registro en el rango
        pair<int, int> locEnd = findLocation(keyEnd); // Buscar la ubicación del último registro en el rango

        // Si los registros están en el archivo de datos
        if (locBegin.first == 0 && locBegin.second != -1) {
            for (int pos = locBegin.second; pos <= locEnd.second; ++pos) {
                data.seekg(pos * sizeRecord());
                data >> temp;
                // Si la clave está en el rango y el registro no está marcado como eliminado, agregar al vector
                if (temp.getKey() >= keyBegin && temp.getKey() <= keyEnd && !temp.deleted) {
                    results.push_back(temp);
                }
            }
        }

        // Si alguno de los registros está en el archivo auxiliar
        if (locBegin.first == 1 || locEnd.first == 1) {
            aux.seekg(0, ios::end);
            long sizeAux = aux.tellg();
            long numRecords = sizeAux / sizeRecord();
            for (int pos = 0; pos < numRecords; ++pos) {
                aux.seekg(pos * sizeRecord());
                aux >> temp;
                // Si la clave está en el rango y el registro no está marcado como eliminado, agregar al vector
                if (temp.getKey() >= keyBegin && temp.getKey() <= keyEnd && !temp.deleted) {
                    results.push_back(temp);
                }
            }
        }
        data.close();
        aux.close();
        return results;
    }

    bool insert(fixedRecord<typeRecord, typeKey> record) {
        fstream data(dataFile, ios::out | ios::in | ios::binary);
        fstream aux(auxFile, ios::out | ios::in | ios::binary);
        fstream data2("../dataFile2.dat", ios::out | ios::binary);
        if (!data || !aux) return false;
        aux.seekg(0, ios::end);
        long sizeAux = aux.tellg() / sizeRecord();
        if (sizeAux == maxAuxSize) {
            merge_data();
            data.close();
            remove("../dataFile.dat");
            data2.close();
            rename("../dataFile2.dat", "../dataFile.dat");
            ofstream auxTemp;
            auxTemp.open(auxFile, ios::out | ios::trunc);
            auxTemp.close();
            insert(record);
            return true;
        } else {
            // si la key no esta
            pair<int,int> prev = findLocation(record.getKey());
            //cout << prev.first << " " << prev.second << endl;
            aux.seekg(0, ios::end);
            long sizeAux = aux.tellg() / sizeRecord();
            if (prev.first == 0) {
                // si la key esta en data
                data.seekg(prev.second * sizeRecord());
                fixedRecord<typeRecord, typeKey> temp;
                data >> temp;
                if (temp.getKey() == record.getKey()) return false;
                // actualizar puntero de record
                record.nextPosition = temp.nextPosition;
                record.nextFile = temp.nextFile;
                // actualizar puntero del anterior
                temp.nextPosition = sizeAux;
                temp.nextFile = 1;
                data.seekp(prev.second * sizeRecord());
                data << temp;
                aux.seekp(sizeAux * sizeRecord());
                aux << record;
                data.close();
                aux.close();
            } else {
                // si la key esta en aux
                aux.seekg(prev.second * sizeRecord());
                fixedRecord<typeRecord, typeKey> temp;
                aux >> temp;
                if (temp.getKey() == record.getKey()) return false;
                // actualizar puntero de record
                record.nextPosition = temp.nextPosition;
                record.nextFile = temp.nextFile;
                // actualizar puntero del anterior
                temp.nextPosition = sizeAux;
                temp.nextFile = 1;
                aux.seekp(prev.second * sizeRecord());
                aux << temp;
                aux.seekp(sizeAux * sizeRecord());
                aux << record;
            }
        }
        data.close();
        aux.close();
    }

    bool removeRecord(int key) {
        fstream data(dataFile, ios::in | ios::out | ios::binary);
        fstream aux(auxFile, ios::in | ios::out | ios::binary);
        if (!data || !aux) return false;
        
        fixedRecord<typeRecord, typeKey> tempPrev, temp;
        pair<int, int> loc = findLocation(key); // <file, position>
        if(loc.first == 0){
            data.seekg(loc.second * sizeRecord());
            data >> temp;
        }else if(loc.first == 1){
            aux.seekg(loc.second * sizeRecord());
            aux >> temp;
        }
        
        if(temp.getKey() != key || temp.deleted){
            cerr << "No existe registro con ese key\n";
            return false;
        }
        
        pair<int, int> locPrev = findLocation(key-1); // <file, position>
        if (locPrev.first == 0) {
            data.seekg(locPrev.second * sizeRecord());
            data >> tempPrev;
        } else if (locPrev.first == 1) {
            aux.seekg(locPrev.second * sizeRecord());
            aux >> tempPrev;
        }

        tempPrev.nextFile = temp.nextFile;
        tempPrev.nextPosition = temp.nextPosition;
        temp.deleted = 1;
        
        if (locPrev.first == 0 && loc.first == 0) {
            data.seekp(locPrev.second * sizeRecord());
            data << tempPrev;            
            data.seekp(loc.second * sizeRecord());
            data << temp;
        } else if (locPrev.first == 1 && loc.first == 1) {
            aux.seekp(locPrev.second * sizeRecord());
            aux << tempPrev;
            aux.seekp(loc.second * sizeRecord());
            aux << temp;
        } else if (locPrev.first == 0 && loc.first == 1){            
            data.seekp(locPrev.second * sizeRecord());
            data << tempPrev;            
            aux.seekp(loc.second * sizeRecord());
            aux << temp;
        } else if (locPrev.first == 1 && loc.first == 0){            
            aux.seekp(locPrev.second * sizeRecord());
            aux << tempPrev;
            data.seekp(loc.second * sizeRecord());
            data << temp;
        }

        data.close();
        aux.close();
        return true;
    }


    void merge_data() {
        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);
        fstream data2("../dataFile2.dat", ios::out | ios::binary);
        if (!data || !aux) return;
        fixedRecord<typeRecord, typeKey> header, temp;
        data.seekg(0, ios::end);

        int newSize = maxAuxSize + (data.tellg()/sizeRecord());
        int countDeleted = countD(dataFile,auxFile);
        newSize -= countDeleted;
        data.seekg(0);
        data >> header;
        temp.nextPosition = header.nextPosition;
        temp.nextFile = header.nextFile;
        data2.seekp(0);
        data2 << header;
        int pos = 1;
        while(temp.nextPosition!=-1 and temp.nextFile!=-1){
            fixedRecord<typeRecord, typeKey> curr;
            if (temp.nextFile == 0) {
                data.seekg(temp.nextPosition * sizeRecord());
                data >> curr;
                fixedRecord<typeRecord, typeKey> curr1 = curr;
                curr1.nextPosition = pos+1 < newSize ? pos+1: -1;
                curr1.nextFile = pos+1 < newSize ? 0 : -1;
                data2.seekp(pos * sizeRecord());
                data2 << curr1;
            }else if (temp.nextFile == 1) {
                aux.seekg(temp.nextPosition * sizeRecord());
                aux >> curr;
                fixedRecord<typeRecord, typeKey> curr1 = curr;
                curr1.nextPosition = pos+1 < newSize ? pos+1: -1;
                curr1.nextFile = pos+1 < newSize ? 0 : -1;
                data2.seekp(pos * sizeRecord());
                data2 << curr1;
            }
            temp.nextFile = curr.nextFile;
            temp.nextPosition = curr.nextPosition;
            pos++;
        }
        data.close();
        aux.close();
    }

    void readRecordData(int pos) {
        // read record from dataFile
        fstream data(dataFile, ios::in | ios::binary);
        data.seekg(pos * sizeRecord());
        fixedRecord<typeRecord, typeKey> record;
        data >> record;
        data.close();
        record.print();
    }

    pair<int, int> findLocation(int key) {
        fstream data(dataFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);
        if (!data || !aux) return {-1, -1};
        fixedRecord<typeRecord, typeKey> temp;
        data.seekg(0, ios::end);
        long sizeData = data.tellg();
        long l = 0;
        long r = (sizeData / sizeRecord()) - 1;
        long index = -1;
        long file = -1;
        while (l <= r) {
            long m = l + (r - l) / 2;
            data.seekg(m * sizeRecord());
            data >> temp;
            if (temp.getKey() == key) {
                if (!temp.deleted){
                    file = 0;
                    index = m;
                }
            }
            if (temp.getKey() < key) l = m + 1;
            else r = m - 1;
        }
        if (index != -1) return {file, index};
        else {
            file = 0;
            index = max(0l, l-1);
        }
        if (temp.nextFile == 0) return {file,index};
        if (temp.nextFile == 1) {
            int nextFile = temp.nextFile;
            int nextPosition = temp.nextPosition;
            while (nextFile == 1) {
                fixedRecord<typeRecord, typeKey> tempAux;
                aux.seekg(nextPosition * sizeRecord());
                aux >> tempAux;
                if (tempAux.getKey() > key){
                    break;
                }else if (tempAux.getKey() == key) {
                    file = 1;
                    index = nextPosition;
                    break;
                } else {
                    index = nextPosition;
                    file = 1;
                    nextPosition = tempAux.nextPosition;
                    nextFile = tempAux.nextFile;
                }
            }
        }
        data.close();
        aux.close();
        return {file, index};
    }

    void readRecordAux(int pos) {
        fstream aux(auxFile, ios::in | ios::binary);
        aux.seekg(pos * sizeRecord());
        fixedRecord<typeRecord, typeKey> record;
        aux >> record;
        aux.close();
        record.print();
    }

    int countD(const char* d, const char* a){
        fstream data(d, ios::in | ios::binary);
        fstream aux(a, ios::in | ios::binary);
        int countDeleted = 0;
        fixedRecord<typeRecord,typeKey> recIt;
        data.seekg(0,ios::end);
        int pos = 0, cant = data.tellg() / sizeRecord();
        while(pos<=cant){
            data.seekg(pos * sizeRecord());
            data >> recIt;
            if (recIt.deleted) countDeleted++;
            pos++;
        }
        aux.seekg(0,ios::end);
        pos = 0, cant = aux.tellg() / sizeRecord();
        while(pos<=cant){
            aux.seekg(pos*sizeRecord());
            aux >> recIt;
            if (recIt.deleted) countDeleted++;
            pos++;
        }
        data.close();
        aux.close();
        return countDeleted;
    }

    int sizeRecord(){
        return sizeof(fixedRecord<typeRecord,typeKey>);
    }
};


#endif //PROYECTO_1_BD2_SEQUENTIALFILE_H