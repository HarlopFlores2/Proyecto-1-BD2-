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
        if(i==4) break;
    }
    data.close();
    aux.close();

}

void sequentialFile::print_all(string file) {
    fstream filet(file, ios::in | ios::binary);
    int pos = 0;
    fixedRecord temp;
    while(filet >> temp){
        pos++;
        filet.seekg(pos * sizeRecord());
        temp.print();
    }
}

vector<fixedRecord> sequentialFile::search(int key) {
    // Crear un vector para almacenar los resultados
    vector<fixedRecord> results;
    fixedRecord temp;
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

    return results;
}


vector<fixedRecord> sequentialFile::range_search(int keyBegin, int keyEnd) {
    vector<fixedRecord> results;
    fixedRecord temp;

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

    return results;
}


bool sequentialFile::insert(fixedRecord record) {
    fstream data(dataFile, ios::out | ios::in | ios::binary);
    fstream aux(auxFile, ios::out | ios::in | ios::binary);
    if (!data || !aux) return false;
    if (sizeAux == maxAuxSize) {
        merge_data();
        //data.close();
        //remove("dataFile.dat");
        //rename("dataFile2.dat", "dataFile.dat");
        //insert(record);
        return true;
    } else {
        // si la key no esta
        pair<int,int> prev = findLocation(record.getKey());
        if (prev.first == 0) {
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
    fixedRecord temp;
    fstream data(dataFile, ios::in | ios::binary);
    fstream aux(auxFile, ios::in | ios::binary);
    
    if (!data || !aux) return false;

    pair<int, int> loc = findLocation(key);

    if (loc.first == 0) {
        data.seekg(loc.second * sizeRecord());
        data >> temp;
        if (temp.getKey() == key && !temp.deleted) {
            temp.deleted = 1;
            return true;
        }
    } else if (loc.first == 1) {
        aux.seekg(loc.second * sizeRecord());
        aux >> temp;        
        if (temp.getKey() == key && !temp.deleted) {
            temp.deleted = 1;
            return true;
        }
    }

    return false;
}

void sequentialFile::merge_data() {
    fstream data(dataFile, ios::in | ios::binary);
    fstream aux(auxFile, ios::in | ios::binary);
    fstream data2("../dataFile2.dat", ios::out | ios::binary);
    if (!data || !aux) return;
    data.seekg(0, ios::end);
    int newSize = maxAuxSize + (data.tellg()/sizeRecord());
    int pos1 = 0, pos2 = 0;
    data.seekg(0);
    fixedRecord temp;
    data >> temp;
    if (!temp.deleted) {
        fixedRecord temp1 = temp;
        temp1.nextPosition = pos1+pos2+1 < newSize ? pos1+pos2+1: 0;
        temp1.nextFile = pos1+pos2+1 < newSize ? 0 : 1;
        data2.seekp(0);
        data2 << temp1;
        pos1++;
    }
    fixedRecord temp2 = temp;
    while(pos1+pos2 < newSize) {
        if(temp2.deleted) continue;
        else if (temp2.nextFile == 0) {
            data.seekg(temp2.nextPosition * sizeRecord());
            data >> temp2;
            fixedRecord temp3 = temp2;
            temp3.nextPosition = pos1+pos2+1 < newSize ? pos1+pos2+1: 0;
            temp3.nextFile = pos1+pos2+1 < newSize ? 0 : 1;
            data2.seekp((pos1+pos2) * sizeRecord());
            data2 << temp3;
            pos1++;
        } else {
            while (temp2.nextFile == 1) {
                aux.seekg(temp2.nextPosition * sizeRecord());
                aux >> temp2;
                fixedRecord temp3 = temp2;
                temp3.nextPosition = pos1+pos2+1 < newSize ? pos1+pos2+1: 0;
                temp3.nextFile = pos1+pos2+1 < newSize ? 0 : 1;
                data2.seekp((pos1+pos2) * sizeRecord());
                data2 << temp3;
                pos2++;
            }
        }
    }
    data.close();
    aux.close();
    data2.close();
    //remove("dataFile.dat");
    //rename("dataFile2.dat", "dataFile.dat");
    //remove("auxFile.dat");
    //rename("auxFile2.dat", "auxFile.dat");
    //sizeAux = 0;

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


