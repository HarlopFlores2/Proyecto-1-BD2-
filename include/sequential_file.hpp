#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace std;

enum class IndexLocation
{
    data,
    aux,
    no_next
};

template<typename Key>
struct IndexRecord
{
    Key key;
    uint64_t relation_index;

    IndexLocation next_file;
    uint64_t next_position = std::numeric_limits<uint64_t>::max();
    bool deleted = false;

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
    std::filesystem::path m_data_filename;
    std::filesystem::path m_aux_filename;

    mutable std::fstream m_data_file;
    mutable std::fstream m_aux_file;

    uint64_t m_max_aux_size;

public:
    constexpr static uint64_t header_size =
        sizeof(IndexRecord<Key>::next_file) + sizeof(IndexRecord<Key>::next_position);

    class RawIterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = IndexRecord<Key>;
        using difference_type = std::ptrdiff_t;
        using reference = IndexRecord<Key> const&;
        using pointer = IndexRecord<Key> const*;

    private:
        std::filesystem::path m_filename;
        mutable std::ifstream m_file;
        uint64_t m_index;

        uint64_t m_record_size;

        mutable std::optional<IndexRecord<Key>> m_value_read;

        RawIterator(std::filesystem::path filename, uint64_t index, uint64_t record_size)
            : m_filename(filename),
              m_file(m_filename, std::ios::in | std::ios::binary),
              m_index(index),
              m_record_size(record_size)
        {
            m_file.exceptions(std::ios::failbit);
        }

        RawIterator(RawIterator const& it)
            : RawIterator{it.m_filename, it.m_index, it.m_record_size}
        {
        }

        uint64_t calculate_offset(uint64_t index) const
        {
            return header_size + index * m_record_size;
        }

        auto operator*() const -> reference
        {
            if (!m_value_read.has_value())
            {
                IndexRecord<Key> temp;
                m_file.seekg(calculate_offset(m_index), std::ios::beg);
                m_file >> temp;
                m_value_read.emplace(temp);
            }

            return m_value_read.value();
        }

        auto operator->() const -> pointer
        {
            return std::addressof(**this);
        }

        auto operator+=(difference_type n) -> RawIterator&
        {
            m_value_read.reset();
            m_index += n;
            return *this;
        }

        auto operator+(difference_type n)
        {
            RawIterator ret(*this);
            ret += n;
            return ret;
        }

        auto operator-=(difference_type n) -> RawIterator&
        {
            (*this) += -n;
            return *this;
        }

        auto operator-(difference_type n)
        {
            RawIterator ret(*this);
            ret -= n;
            return ret;
        }

        auto operator-(RawIterator const& other)
        {
            return m_index - other.m_index;
        }

        auto operator++() -> RawIterator&
        {
            (*this) += 1;

            return *this;
        }

        auto operator++(int) -> RawIterator
        {
            RawIterator it{*this};
            ++it;
            return it;
        }

        auto operator==(RawIterator const& other) -> bool
        {
            if (this->m_filename != other.m_filename)
            {
                return false;
            }

            return this->m_index == other.m_index;
        }

        auto operator!=(RawIterator const& other) -> bool
        {
            return !(*this == other);
        }
    };

    using reverse_raw_iterator = std::reverse_iterator<RawIterator>;

    class Iterator
    {
        friend IndexLocation;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using reference = IndexRecord<Key> const&;
        using pointer = IndexRecord<Key> const*;

    private:
        mutable std::ifstream m_data_file;
        mutable std::ifstream m_aux_file;

        std::filesystem::path m_data_filename;
        std::filesystem::path m_aux_filename;

        uint64_t m_index;
        IndexLocation m_index_location;

        uint64_t m_record_size;

        mutable std::optional<IndexRecord<Key>> m_value_read;
        bool m_end = false;

        Iterator(
            std::filesystem::path data_filename,
            std::filesystem::path aux_filename,
            uint64_t file_index,
            IndexLocation index_location,
            uint64_t record_size)
            : m_data_file(data_filename, std::ios::in | std::ios::binary),
              m_aux_file(aux_filename, std::ios::in | std::ios::binary),
              m_data_filename(data_filename),
              m_aux_filename(aux_filename),
              m_index(file_index),
              m_index_location(index_location),
              m_record_size(record_size)
        {
            m_data_file.exceptions(std::ios::failbit);
            m_aux_file.exceptions(std::ios::failbit);

            // Make sure we start with the first valid tuple
            this->advance_until_next_valid();
        }

        Iterator(Iterator const& it)
            : Iterator{
                it.m_data_filename,
                it.m_aux_filename,
                it.m_index,
                it.m_index_location,
                it.m_record_size}
        {
            m_end = it.m_end;
        }

        uint64_t calculate_offset(uint64_t index, IndexLocation index_location) const
        {
            if (index_location == IndexLocation::data)
            {
                return header_size + index * m_record_size;
            }
            else if (index_location == IndexLocation::aux)
            {
                return index * m_record_size;
            }

            throw std::runtime_error("???");
        }

        auto operator*() const -> reference
        {
            if (m_end)
            {
                throw std::runtime_error("Dereferencing iterator past end.");
            }

            if (!m_value_read.has_value())
            {
                IndexRecord<Key> temp;

                if (m_index_location == IndexLocation::data)
                {
                    m_data_file.seekg(
                        calculate_offset(m_index, m_index_location), std::ios::beg);
                    m_data_file >> temp;
                }
                else if (m_index_location == IndexLocation::aux)
                {
                    m_aux_file.seekg(
                        calculate_offset(m_index, m_index_location), std::ios::beg);
                    m_aux_file >> temp;
                }
                else
                {
                    throw std::runtime_error("???");
                }

                m_value_read.emplace(temp);
            }

            return m_value_read.value();
        }

        auto operator->() const -> pointer
        {
            return std::addressof(**this);
        }

        void advance()
        {
            IndexRecord<Key> temp = **this;

            m_index = temp.next_position;
            m_index_location = temp.next_file;

            m_value_read.reset();
        }

        auto operator++() -> Iterator&
        {
            if (m_end)
            {
                return *this;
            }

            m_value_read.reset();

            IndexRecord<Key> temp = *this;

            this->advance();
            this->advance_until_next_valid();

            return *this;
        }

        auto operator++(int) -> Iterator
        {
            Iterator it{*this};
            ++it;
            return it;
        }

        auto operator==(Iterator const& other) -> bool
        {
            if (this->m_data_filename != other.m_data_filename
                || this->m_aux_filename != other.m_aux_filename)
            {
                return false;
            }

            if (this->m_end == true || other.m_end == true)
            {
                return this->m_end == other.m_end;
            }

            return this->m_file_offset == other.m_file_offset
                   && this->m_index_location == other.m_index_location;
        }

        auto operator!=(Iterator const& other) -> bool
        {
            return !(*this == other);
        }

        void advance_until_next_valid()
        {
            while (true)
            {
                if (m_index_location == IndexLocation::no_next)
                {
                    m_end = true;
                    return;
                }

                IndexRecord<Key> ir = **this;

                if (ir.deleted != 1)
                {
                    return;
                }

                this->advance();
            }
        }
    };

    explicit sequentialFile(
        std::filesystem::path data_filename,
        std::filesystem::path aux_filename,
        uint64_t max_aux_size)
        : m_data_filename(std::move(data_filename)),
          m_aux_filename(std::move(aux_filename)),
          m_max_aux_size(max_aux_size)
    {
        std::ofstream(m_data_filename, std::ios::app | std::ios::binary);
        m_data_file.open(m_data_filename, std::ios::in | std::ios::out | std::ios::binary);
        m_data_file.exceptions(std::ios::failbit);

        std::ofstream(m_aux_filename, std::ios::app | std::ios::binary);
        m_aux_file.open(m_aux_filename, std::ios::in | std::ios::out | std::ios::binary);
        m_aux_file.exceptions(std::ios::failbit);

        if (m_max_aux_size < 1)
        {
            throw std::runtime_error("max_aux_size must be larger than 1");
        }

        // Initialize data file if needed
        m_data_file.seekg(0, std::ios::end);
        if (m_data_file.tellg() == 0)
        {
            IndexLocation index_location = IndexLocation::no_next;
            m_data_file.write(
                reinterpret_cast<char const*>(&index_location), sizeof(index_location));
            uint64_t next_position = 0;
            m_data_file.write(
                reinterpret_cast<char const*>(&next_position), sizeof(next_position));
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

    std::pair<IndexLocation, uint64_t> get_header() const
    {
        m_data_file.seekg(0, std::ios::beg);

        IndexLocation index_location{};
        m_data_file.read(reinterpret_cast<char*>(&index_location), sizeof(index_location));

        uint64_t next_position = 0;
        m_data_file.read(reinterpret_cast<char*>(&next_position), sizeof(next_position));

        return {index_location, next_position};
    }

    int sizeRecord()
    {
        return sizeof(IndexRecord<Key>);
    }
};
