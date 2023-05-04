#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace std;

template<typename Key>
struct IndexRecord
{
    Key key;
    uint64_t relation_index;

    int next_file = 0;
    long next_position = -1;
    int deleted = 0;

    void print(std::ostream& out = std::cerr)
    {
        out << "{" << key << ": " << relation_index << ", " << next_file << ", " << deleted
            << "}\n";
    }
};

template<typename Key>
ostream& operator<<(ostream& out, IndexRecord<Key> const& p)
{
    // TODO: Think how key

    out.write(reinterpret_cast<char const*>(&p.relation_index), sizeof(p.relation_index));
    out.write(reinterpret_cast<char const*>(&p.next_file), sizeof(p.next_file));
    out.write(reinterpret_cast<char const*>(&p.next_position), sizeof(p.next_position));
    out.write(reinterpret_cast<char const*>(&p.deleted), sizeof(p.deleted));

    out.flush();
    return out;
}

template<typename Key>
istream& operator>>(istream& in, IndexRecord<Key>& p)
{
    // TODO: Think how key

    in.read(reinterpret_cast<char*>(&p.relation_index), sizeof(p.relation_index));
    in.read(reinterpret_cast<char*>(&p.next_file), sizeof(p.next_file));
    in.read(reinterpret_cast<char*>(&p.next_position), sizeof(p.next_position));
    in.read(reinterpret_cast<char*>(&p.deleted), sizeof(p.deleted));

    return in;
}

template<typename Key>
class sequentialFile
{
    std::string m_data_file;
    std::string m_aux_file;
    int m_max_aux_size;

public:
    explicit sequentialFile(std::string data_file, std::string aux_file, int max_aux_size)
        : m_data_file(std::move(data_file)),
          m_aux_file(std::move(aux_file)),
          m_max_aux_size(max_aux_size)
    {
        if (m_max_aux_size < 1)
        {
            throw std::runtime_error("max_aux_size must be larger than 1");
        }
    };

    int count_deleted()
    {
        int count = 0;
        IndexRecord<Key> rec;

        fstream data(m_data_file, ios::in | ios::binary);
        data.seekg(0, ios::end);

        int data_n_records = data.tellg() / sizeRecord();

        for (int pos = 0; pos < data_n_records; ++pos)
        {
            data.seekg(pos * sizeRecord());
            data >> rec;

            if (rec.deleted)
            {
                ++count;
            }
        }

        fstream aux(m_aux_file, ios::in | ios::binary);
        aux.seekg(0, ios::end);

        int aux_n_records = aux.tellg() / sizeRecord();

        for (int pos = 0; pos < aux_n_records; ++pos)
        {
            aux.seekg(pos * sizeRecord());
            aux >> rec;

            if (rec.deleted)
            {
                count++;
            }
        }

        return count;
    }

    void print_all(string file)
    {
        fstream filet(file, ios::in | ios::binary);
        int pos = 0;
        IndexRecord<Key> temp;
        while (filet >> temp)
        {
            pos++;
            filet.seekg(pos * sizeRecord());
            temp.print();
        }
        cout << "*****************\n";
    }

    vector<IndexRecord<Key>> search(int key)
    {
        // Crear un vector para almacenar los resultados
        vector<IndexRecord<Key>> results;
        IndexRecord<Key> temp;
        fstream data(m_data_file, ios::in | ios::binary);
        fstream aux(m_aux_file, ios::in | ios::binary);

        if (!data || !aux)
            return results; // Si no se pueden abrir los archivos, retorna vacio

        pair<int, int> loc =
            findLocation(key); // Buscar la ubicación del registro con la clave proporcionada

        // Si el registro se encuentra en el archivo de datos...
        if (loc.first == 0)
        {
            data.seekg(loc.second * sizeRecord());
            data >> temp;
            // Si la clave coincide y el registro no está marcado como eliminado, agregar al
            // vector
            if (temp.key == key && !temp.deleted)
            {
                results.push_back(temp);
            }
            // Si el registro se encuentra en el archivo auxiliar
        }
        else if (loc.first == 1)
        {
            aux.seekg(loc.second * sizeRecord());
            aux >> temp;
            // Si la clave coincide y el registro no está marcado como eliminado, agregar al
            // vector
            if (temp.key == key && !temp.deleted)
            {
                results.push_back(temp);
            }
        }
        data.close();
        aux.close();
        return results;
    }

    vector<IndexRecord<Key>> range_search(int keyBegin, int keyEnd)
    {
        vector<IndexRecord<Key>> results;
        IndexRecord<Key> temp;

        // Si el rango no es válido, retornar vacio
        if (keyBegin > keyEnd)
        {
            return results;
        }

        fstream data(m_data_file, ios::in | ios::binary);
        fstream aux(m_aux_file, ios::in | ios::binary);

        if (!data || !aux)
            return results; // Si no se pueden abrir los archivos, retorna nada

        pair<int, int> locBegin =
            findLocation(keyBegin); // Buscar la ubicación del primer registro en el rango
        pair<int, int> locEnd =
            findLocation(keyEnd); // Buscar la ubicación del último registro en el rango

        // Si los registros están en el archivo de datos
        if (locBegin.first == 0 && locBegin.second != -1)
        {
            for (int pos = locBegin.second; pos <= locEnd.second; ++pos)
            {
                data.seekg(pos * sizeRecord());
                data >> temp;
                // Si la clave está en el rango y el registro no está marcado como eliminado,
                // agregar al vector
                if (temp.key >= keyBegin && temp.key <= keyEnd && !temp.deleted)
                {
                    results.push_back(temp);
                }
            }
        }

        // Si alguno de los registros está en el archivo auxiliar
        if (locBegin.first == 1 || locEnd.first == 1)
        {
            aux.seekg(0, ios::end);
            long sizeAux = aux.tellg();
            long numRecords = sizeAux / sizeRecord();
            for (int pos = 0; pos < numRecords; ++pos)
            {
                aux.seekg(pos * sizeRecord());
                aux >> temp;
                // Si la clave está en el rango y el registro no está marcado como eliminado,
                // agregar al vector
                if (temp.key >= keyBegin && temp.key <= keyEnd && !temp.deleted)
                {
                    results.push_back(temp);
                }
            }
        }
        data.close();
        aux.close();
        return results;
    }

    bool insert(IndexRecord<Key> record)
    {
        fstream data(m_data_file, ios::out | ios::in | ios::binary);
        fstream aux(m_aux_file, ios::out | ios::in | ios::binary);
        fstream data2("../dataFile2.dat", ios::out | ios::binary);
        if (!data || !aux)
            return false;
        aux.seekg(0, ios::end);
        long sizeAux = aux.tellg() / sizeRecord();
        if (sizeAux == m_max_aux_size)
        {
            merge_data();
            data.close();
            remove("..//dataFile.dat");
            data2.close();
            rename("..//dataFile2.dat", "..//dataFile.dat");
            ofstream auxTemp;
            auxTemp.open(m_aux_file, ios::out | ios::trunc);
            auxTemp.close();
            insert(record);
            return true;
        }
        else
        {
            // si la key no esta
            pair<int, int> prev = findLocation(record.key);
            // cout << prev.first << " " << prev.second << endl;
            aux.seekg(0, ios::end);
            long sizeAux = aux.tellg() / sizeRecord();
            if (prev.first == 0)
            {
                // si la key esta en data
                data.seekg(prev.second * sizeRecord());
                IndexRecord<Key> temp;
                data >> temp;
                if (temp.key == record.key)
                    return false;
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
            }
            else
            {
                // si la key esta en aux
                aux.seekg(prev.second * sizeRecord());
                IndexRecord<Key> temp;
                aux >> temp;
                if (temp.key == record.key)
                    return false;
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

    bool removeRecord(int key)
    {
        fstream data(m_data_file, ios::in | ios::out | ios::binary);
        fstream aux(m_aux_file, ios::in | ios::out | ios::binary);
        if (!data || !aux)
            return false;

        IndexRecord<Key> tempPrev, temp;
        pair<int, int> loc = findLocation(key); // <file, position>
        if (loc.first == 0)
        {
            data.seekg(loc.second * sizeRecord());
            data >> temp;
        }
        else if (loc.first == 1)
        {
            aux.seekg(loc.second * sizeRecord());
            aux >> temp;
        }

        if (temp.key != key || temp.deleted)
        {
            cerr << "No existe registro con ese key\n";
            return false;
        }

        pair<int, int> locPrev = findLocation(key - 1); // <file, position>
        if (locPrev.first == 0)
        {
            data.seekg(locPrev.second * sizeRecord());
            data >> tempPrev;
        }
        else if (locPrev.first == 1)
        {
            aux.seekg(locPrev.second * sizeRecord());
            aux >> tempPrev;
        }

        tempPrev.nextFile = temp.nextFile;
        tempPrev.nextPosition = temp.nextPosition;
        temp.deleted = 1;

        if (locPrev.first == 0 && loc.first == 0)
        {
            data.seekp(locPrev.second * sizeRecord());
            data << tempPrev;
            data.seekp(loc.second * sizeRecord());
            data << temp;
        }
        else if (locPrev.first == 1 && loc.first == 1)
        {
            aux.seekp(locPrev.second * sizeRecord());
            aux << tempPrev;
            aux.seekp(loc.second * sizeRecord());
            aux << temp;
        }
        else if (locPrev.first == 0 && loc.first == 1)
        {
            data.seekp(locPrev.second * sizeRecord());
            data << tempPrev;
            aux.seekp(loc.second * sizeRecord());
            aux << temp;
        }
        else if (locPrev.first == 1 && loc.first == 0)
        {
            aux.seekp(locPrev.second * sizeRecord());
            aux << tempPrev;
            data.seekp(loc.second * sizeRecord());
            data << temp;
        }

        data.close();
        aux.close();
        return true;
    }

    void merge_data()
    {
        fstream data(m_data_file, ios::in | ios::binary);
        fstream aux(m_aux_file, ios::in | ios::binary);
        fstream data2("../dataFile2.dat", ios::out | ios::binary);
        if (!data || !aux)
            return;
        IndexRecord<Key> header, temp;
        data.seekg(0, ios::end);

        int newSize = m_max_aux_size + (data.tellg() / sizeRecord());
        int countDeleted = count_deleted(m_data_file, m_aux_file);
        cout << countDeleted << '\n';
        newSize -= countDeleted;
        data.seekg(0);
        data >> header;
        cout << header.nextPosition << " " << header.nextFile << endl;
        temp.nextPosition = header.nextPosition;
        temp.nextFile = header.nextFile;
        data2.seekp(0);
        data2 << header;
        int pos = 1;
        cout << temp.nextPosition << " " << temp.nextFile << endl;
        while (temp.nextPosition != -1 and temp.nextFile != -1)
        {
            IndexRecord<Key> curr;
            if (temp.nextFile == 0)
            {
                data.seekg(temp.nextPosition * sizeRecord());
                data >> curr;
                IndexRecord<Key> curr1 = curr;
                curr1.nextPosition = pos + 1 < newSize ? pos + 1 : -1;
                curr1.nextFile = pos + 1 < newSize ? 0 : -1;
                data2.seekp(pos * sizeRecord());
                data2 << curr1;
            }
            else if (temp.nextFile == 1)
            {
                aux.seekg(temp.nextPosition * sizeRecord());
                aux >> curr;
                IndexRecord<Key> curr1 = curr;
                curr1.nextPosition = pos + 1 < newSize ? pos + 1 : -1;
                curr1.nextFile = pos + 1 < newSize ? 0 : -1;
                data2.seekp(pos * sizeRecord());
                data2 << curr1;
            }
            temp.nextFile = curr.nextFile;
            temp.nextPosition = curr.nextPosition;
            pos++;
        }
        cout << pos << endl;
    }

    void readRecordData(int pos)
    {
        // read record from dataFile
        fstream data(m_data_file, ios::in | ios::binary);
        data.seekg(pos * sizeRecord());
        IndexRecord<Key> record;
        data >> record;
        data.close();
        record.print();
    }

    pair<int, int> findLocation(int key)
    {
        fstream data(m_data_file, ios::in | ios::binary);
        fstream aux(m_aux_file, ios::in | ios::binary);
        if (!data || !aux)
            return {-1, -1};
        IndexRecord<Key> temp;
        data.seekg(0, ios::end);
        long sizeData = data.tellg();
        long l = 0;
        long r = (sizeData / sizeRecord()) - 1;
        long index = -1;
        long file = -1;
        while (l <= r)
        {
            long m = l + (r - l) / 2;
            data.seekg(m * sizeRecord());
            data >> temp;
            if (temp.key == key)
            {
                if (!temp.deleted)
                {
                    file = 0;
                    index = m;
                }
            }
            if (temp.key < key)
                l = m + 1;
            else
                r = m - 1;
        }
        if (index != -1)
            return {file, index};
        else
        {
            file = 0;
            index = max(0l, l - 1);
        }
        if (temp.nextFile == 0)
            return {file, index};
        if (temp.nextFile == 1)
        {
            int nextFile = temp.nextFile;
            int nextPosition = temp.nextPosition;
            while (nextFile == 1)
            {
                IndexRecord<Key> tempAux;
                aux.seekg(nextPosition * sizeRecord());
                aux >> tempAux;
                if (tempAux.key > key)
                {
                    break;
                }
                else if (tempAux.key == key)
                {
                    file = 1;
                    index = nextPosition;
                    break;
                }
                else
                {
                    index = nextPosition;
                    file = 1;
                    nextPosition = tempAux.nextPosition;
                    nextFile = tempAux.nextFile;
                }
            }
        }
        return {file, index};
    }

    void readRecordAux(int pos)
    {
        fstream aux(m_aux_file, ios::in | ios::binary);
        aux.seekg(pos * sizeRecord());
        IndexRecord<Key> record;
        aux >> record;
        aux.close();
        record.print();
    }

    int sizeRecord()
    {
        return sizeof(IndexRecord<Key>);
    }
};
