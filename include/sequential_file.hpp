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

                throw std::runtime_error("This shouldn't happen");

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

    Iterator begin() const
    {
        auto [next_location, next_position] = get_header();
        return Iterator(
            m_data_filename, m_aux_filename, next_position, next_location, sizeRecord());
    }

    Iterator end() const
    {
        return Iterator(
            m_data_filename, m_aux_filename, 0, IndexLocation::no_next, sizeRecord());
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

    void insert(Key const& key, uint64_t relation_index)
    {
        m_aux_file.seekg(0, std::ios::end);
        if (m_aux_file.tellg() / sizeRecord() >= m_max_aux_size)
        {
            merge_data();
        }

        auto it_o = find_location_to_add(key);

        IndexRecord<Key> ir;
        ir.key = key;
        ir.relation_index = relation_index;
        ir.deleted = false;

        if (it_o.has_value())
        {
            ir.next_file = it_o.m_index_location;
            ir.next_position = it_o.m_index;
        }
        else
        {
            auto [next_location, next_index] = get_header();
            ir.next_file = next_location;
            ir.next_position = next_index;
        }

        m_aux_file.seekp(0, std::ios::end);
        uint64_t index_to_add = m_aux_file.tellp() / sizeRecord();

        m_aux_file << ir;

        if (!it_o.has_value())
        {
            this->set_header(IndexLocation::aux, index_to_add);
        }
        else
        {
            Iterator it = it_o.value();

            IndexRecord<Key> temp = *it;
            temp.next_file = IndexLocation::aux;
            temp.next_position = index_to_add;

            this->write_record(it.m_index_location, it.m_index, temp);
        }
    }

    bool remove_record(Key key, uint64_t relation_index)
    {
        auto remove_next = [&](Iterator it) -> void {
            IndexRecord<Key> ir = *it;

            if (ir.next_file == IndexLocation::no_next)
            {
                return;
            }

            Iterator it_next = it;
            ++it_next;
            IndexRecord<Key> ir_next = *it_next;

            ir_next.deleted = true;
            this->write_record(it_next.m_index_location, it_next.m_index, ir_next);

            ir.next_file = ir_next.next_file;
            ir.next_position = ir_next.next_position;

            this->write_record(it.m_index_location, it.m_index, ir);
        };

        std::optional<Iterator> it_o = this->find_location_to_add(key);

        Iterator it;
        if (!it_o.has_value())
        {
            it = this->begin();

            if (it == this->end() || it->key != key)
            {
                return false;
            }

            if (it->relation_index == relation_index)
            {
                IndexRecord<Key> ir = *it;

                set_header(ir.next_file, ir.next_position);

                ir.deleted = true;
                this->write_record(it.m_index_location, it.m_index, ir);

                return true;
            }
        }
        else
        {
            it = it_o.value();
        }

        Iterator it_next = it;
        ++it_next;

        while (it_next != this->end() && it_next->key == key)
        {
            if (it_next->relation_index == relation_index)
            {
                remove_next(it);
                return true;
            }

            ++it;
            ++it_next;
        }

        return false;
    }

    void merge_data()
    {
        std::filesystem::path new_data_filename = m_data_filename;
        new_data_filename += ".new";
        fstream new_data_file(new_data_filename, ios::out | ios::binary);

        IndexLocation next_location = IndexLocation::data;
        uint64_t next_position = 0;

        new_data_file.write(
            reinterpret_cast<char const*>(&next_location), sizeof(next_location));
        new_data_file.write(
            reinterpret_cast<char const*>(&next_position), sizeof(next_position));

        bool written_one = false;

        for (IndexRecord<Key> const& ir : *this)
        {
            written_one = true;
            new_data_file << ir;
        }

        m_data_file.close();
        new_data_file.close();

        std::filesystem::rename(new_data_filename, m_data_filename);
        m_data_file.open(m_data_filename, std::ios::in | std::ios::out | std::ios::binary);

        std::filesystem::resize_file(m_aux_filename, 0);

        if (!written_one)
        {
            set_header(IndexLocation::no_next, 0);
        }
    }

    std::optional<Iterator> find_location_to_add(Key const& key)
    {
        /*
        ** Returns the last *active* location with a value less than key.
        */

        fstream data_file(m_data_filename, ios::in | ios::binary);

        if (!data_file)
        {
            throw std::runtime_error("Data file couldn't be opened.");
        }

        IndexRecord<Key> temp;

        data_file.seekg(0, ios::end);
        long n_records = data_file.tellg() / sizeRecord();

        long l = 0;
        long r = n_records - 1;

        long index = -1;
        long file = -1;

        reverse_raw_iterator r_end(RawIterator(m_data_filename, 0, sizeRecord()));
        reverse_raw_iterator r_begin(RawIterator(m_data_filename, n_records, sizeRecord()));

        reverse_raw_iterator r_it = std::upper_bound(
            r_begin, r_end, [=](IndexRecord<Key> ir) -> bool { return ir.key < key; });

        while (r_it != r_end)
        {
            if (r_it->deleted == 0)
            {
                Iterator it(
                    m_data_filename,
                    m_aux_filename,
                    r_it.base().m_index,
                    IndexLocation::data,
                    sizeRecord());
                Iterator it_next(it);
                ++it_next;

                while (it_next != this->end() && it_next->key < key)
                {
                    ++it;
                    ++it_next;
                }
                return {it};
            }

            ++r_it;
        }

        auto [next_location, next_position] = get_header();

        if (next_location != IndexLocation::aux)
        {
            return {};
        }

        Iterator it(
            m_data_filename, m_aux_filename, next_position, IndexLocation::aux, sizeRecord());

        if (!(it->key < key))
        {
            return {};
        }

        Iterator it_next = it;
        ++it_next;

        while (it_next != this->end() && it_next->key < key)
        {
            ++it;
            ++it_next;
        }

        return {it};
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

    void set_header(IndexLocation index_location, uint64_t next_position) const
    {
        m_data_file.seekp(0, std::ios::beg);

        m_data_file.write(
            reinterpret_cast<char const*>(&index_location), sizeof(index_location));
        m_data_file.write(reinterpret_cast<char const*>(&next_position), sizeof(next_position));
    }

    uint64_t calculate_offset(uint64_t index, IndexLocation index_location)
    {
        if (index_location == IndexLocation::data)
        {
            return header_size + index * sizeRecord();
        }
        else if (index_location == IndexLocation::aux)
        {
            return index * sizeRecord();
        }

        throw std::runtime_error("???");
    }

    void
    write_record(IndexLocation index_location, uint64_t index, IndexRecord<Key> const& record)
    {
        if (index_location == IndexLocation::data)
        {
            m_data_file.seekp(header_size + index * sizeRecord());
            m_data_file << record;
        }
        else if (index_location == IndexLocation::aux)
        {
            m_aux_file.seekp(index * sizeRecord());
            m_aux_file << record;
        }

        throw std::runtime_error("???");
    }

    int sizeRecord()
    {
        return sizeof(IndexRecord<Key>);
    }
};
