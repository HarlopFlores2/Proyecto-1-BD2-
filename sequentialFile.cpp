//
// Created by VIRGINIA on 22/04/2023.
//

#include "sequentialFile.h"

void readFromConsole(char buffer[], int size) {
    string temp;
    cin >> temp;
    for (int i = 0; i < size; i++)
        buffer[i] = (i < temp.size()) ? temp[i] : ' ';
    buffer[size - 1] = '\0';
    cin.clear();
}

ostream &operator<<(ostream &stream, fixedRecord &p) {
    stream.write((char *) &p, sizeof(p));
    stream << flush;
    return stream;
}

istream &operator>>(istream &stream, fixedRecord &p) {
    stream.read((char *) &p, sizeof(p));
    return stream;
}


void sequentialFile::load_data(const string & csvFile) {
    fixedRecord temp;
    fstream data(dataFile, ios::out | ios::binary);
    fstream aux(auxFile, ios::out | ios::binary);
    if (!data || !aux) return;
    rapidcsv::Document document(csvFile);
    auto len = document.GetRowCount();
    long offset = 0;
    for (int i = 0; i < len; i++) {
        vector<string> row = document.GetRow<string>(i);
        temp.load(row);
        offset++;
        temp.nextPosition = (i == len-1) ? 0 : offset;
        temp.nextFile = (i == len-1) ? 1 : 0;
        data << temp;
    }
    data.close();
    aux.close();

}

void sequentialFile::print_all() {

}

vector<fixedRecord> sequentialFile::search(int key) {
    return vector<fixedRecord>();
}

vector<fixedRecord> sequentialFile::range_search(int keyBegin, int keyEnd) {
    return vector<fixedRecord>();
}

bool sequentialFile::insert(fixedRecord record) {
    fstream data(dataFile, ios::out | ios::in | ios::binary);
    fstream aux(auxFile, ios::out | ios::in | ios::binary);
    if (!data || !aux) return false;
    if (sizeAux == maxAuxSize) {
        //merge_data();
        //data.close();
        //remove("dataFile.dat");
        rename("dataFile2.dat", "dataFile.dat");
        insert(record);
        return true;
    } else {
        // si la key no esta
        pair<int,int> prev = findLocation(record.getKey());
        if (prev.first == 0) {
            cout<<"***** " << prev.second <<endl;
            // si la key esta en data
            data.seekg(prev.second * sizeRecord());
            fixedRecord temp;
            data >> temp;
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
            sizeAux++;
            data.close();
            aux.close();
        } else {
            // si la key esta en aux
            aux.seekg(prev.second * sizeRecord());
            fixedRecord temp;
            aux >> temp;
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
            sizeAux++;
        }
    }


    return false;
}

bool sequentialFile::remove(int key) {
    return false;
}

void sequentialFile::merge_data() {

}

void sequentialFile::readRecordData(int pos) {
    // read record from dataFile
    fstream data(dataFile, ios::in | ios::binary);
    data.seekg(pos * sizeRecord());
    fixedRecord record;
    data >> record;
    data.close();
    record.print();
}

pair<int, int> sequentialFile::findLocation(int key) {
    fstream data(dataFile, ios::in | ios::binary);
    fstream aux(auxFile, ios::in | ios::binary);
    if (!data || !aux) return {-1, -1};
    fixedRecord temp;
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
        index = l-1;
    }
    if (temp.nextFile == 0) return {file,index};
    if (temp.nextFile == 1) {
        int nextFile = temp.nextFile;
        int nextPosition = temp.nextPosition;
        while (nextFile == 1) {
            fixedRecord tempAux;
            aux.seekg(nextPosition * sizeRecord());
            aux >> tempAux;
            if (tempAux.getKey() == key) {
                file = 1;
                index = nextPosition;
                break;
            } else if (temp.getKey() > key) {
                break;
            } else {
                index = nextPosition;
                file = 1;
                nextPosition = tempAux.nextPosition;
                nextFile = tempAux.nextFile;
            }
        }
    }
    return {file, index};
}

void sequentialFile::readRecordAux(int pos) {
    fstream aux(auxFile, ios::in | ios::binary);
    aux.seekg(pos * sizeRecord());
    fixedRecord record;
    aux >> record;
    aux.close();
    record.print();
}



