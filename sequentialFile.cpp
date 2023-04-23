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
        offset += sizeof(temp);
        temp.nextPosition = offset;
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
    return false;
}

bool sequentialFile::remove(int key) {
    return false;
}

void sequentialFile::merge_data() {

}
