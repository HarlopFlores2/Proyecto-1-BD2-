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

#include "sequential_file.hpp"

#include "json.hpp"

using json = nlohmann::json;

SequentialFile::RawIterator::RawIterator(
    std::filesystem::path filename, uint64_t index, SequentialFile const* sf)
    : m_filename(std::move(filename)),
      m_file(m_filename, std::ios::in | std::ios::binary),
      m_index(index),
      m_sf(sf)
{
    m_file.exceptions(std::ios::failbit);
}

SequentialFile::RawIterator::RawIterator(RawIterator const& other)
    : RawIterator{other.m_filename, other.m_index, other.m_sf}
{
}

SequentialFile::RawIterator::RawIterator(RawIterator&& other) noexcept
    : m_filename(std::move(other.m_filename)),
      m_file(std::move(other.m_file)),
      m_index(other.m_index),
      m_value_read(std::move(other.m_value_read)),
      m_sf(other.m_sf)
{
}

void SequentialFile::RawIterator::swap(RawIterator& other)
{
    std::swap(m_filename, other.m_filename);
    std::swap(m_file, other.m_file);
    std::swap(m_index, other.m_index);
    std::swap(m_value_read, other.m_value_read);
    std::swap(m_sf, other.m_sf);
}

void SequentialFile::RawIterator::swap(RawIterator&& other)
{
    std::swap(m_filename, other.m_filename);
    std::swap(m_file, other.m_file);
    std::swap(m_index, other.m_index);
    std::swap(m_value_read, other.m_value_read);
    std::swap(m_sf, other.m_sf);
}

auto SequentialFile::RawIterator::operator=(RawIterator const& other) -> RawIterator&
{
    this->swap(RawIterator(other));
    return *this;
}

auto SequentialFile::RawIterator::operator=(RawIterator&& other) noexcept -> RawIterator&
{
    this->swap(other);
    return *this;
}

uint64_t SequentialFile::RawIterator::calculate_offset(uint64_t index) const
{
    return header_size + index * m_sf->m_record_size;
}

auto SequentialFile::RawIterator::operator*() const -> reference
{
    if (!m_value_read.has_value())
    {
        m_file.seekg(calculate_offset(m_index), std::ios::beg);
        m_value_read.emplace(m_sf->read_record(m_file));
    }

    return m_value_read.value();
}

auto SequentialFile::RawIterator::operator->() const -> pointer
{
    return std::addressof(**this);
}

auto SequentialFile::RawIterator::operator+=(difference_type n) -> RawIterator&
{
    m_value_read.reset();
    m_index += n;
    return *this;
}

auto SequentialFile::RawIterator::operator+(difference_type n)
{
    RawIterator ret(*this);
    ret += n;
    return ret;
}

auto SequentialFile::RawIterator::operator-=(difference_type n) -> RawIterator&
{
    (*this) += -n;
    return *this;
}

auto SequentialFile::RawIterator::operator-(difference_type n)
{
    RawIterator ret(*this);
    ret -= n;
    return ret;
}

auto SequentialFile::RawIterator::operator-(RawIterator const& other)
{
    return m_index - other.m_index;
}

auto SequentialFile::RawIterator::operator++() -> RawIterator&
{
    (*this) += 1;

    return *this;
}

auto SequentialFile::RawIterator::operator++(int) -> RawIterator
{
    RawIterator it{*this};
    ++(*this);
    return it;
}

auto SequentialFile::RawIterator::operator--() -> RawIterator&
{
    (*this) -= 1;

    return *this;
}

auto SequentialFile::RawIterator::operator--(int) -> RawIterator
{
    RawIterator it{*this};
    --it;
    return it;
}

auto SequentialFile::RawIterator::operator==(RawIterator const& other) -> bool
{
    if (this->m_filename != other.m_filename)
    {
        return false;
    }

    return this->m_index == other.m_index;
}

auto SequentialFile::RawIterator::operator!=(RawIterator const& other) -> bool
{
    return !(*this == other);
}

SequentialFile::Iterator::Iterator(
    std::filesystem::path data_filename,
    std::filesystem::path aux_filename,
    uint64_t file_index,
    IndexLocation index_location,
    SequentialFile const* sf)
    : m_data_file(data_filename, std::ios::in | std::ios::binary),
      m_aux_file(aux_filename, std::ios::in | std::ios::binary),
      m_data_filename(data_filename),
      m_aux_filename(aux_filename),
      m_index(file_index),
      m_index_location(index_location),
      m_sf(sf)
{
    m_data_file.exceptions(std::ios::failbit);
    m_aux_file.exceptions(std::ios::failbit);

    // Make sure we start with the first valid tuple
    this->advance_until_next_valid();
}

SequentialFile::Iterator::Iterator(Iterator const& it)
    : Iterator{it.m_data_filename, it.m_aux_filename, it.m_index, it.m_index_location, it.m_sf}
{
    m_end = it.m_end;
}

auto SequentialFile::Iterator::operator=(Iterator const& other) -> Iterator&
{
    if (this == &other)
    {
        return *this;
    }

    m_data_filename = other.m_data_filename;
    m_aux_filename = other.m_aux_filename;
    m_index = other.m_index;
    m_index_location = other.m_index_location;
    m_sf = other.m_sf;
    m_end = other.m_end;

    m_value_read.reset();

    m_data_file.close();
    m_data_file.open(m_data_filename, std::ios::in | std::ios::binary);

    m_aux_file.close();
    m_aux_file.open(m_aux_filename, std::ios::in | std::ios::binary);

    return *this;
}

uint64_t
SequentialFile::Iterator::calculate_offset(uint64_t index, IndexLocation index_location) const
{
    if (index_location == IndexLocation::data)
    {
        return header_size + index * m_sf->m_record_size;
    }
    else if (index_location == IndexLocation::aux)
    {
        return index * m_sf->m_record_size;
    }

    throw std::runtime_error("???");
}

auto SequentialFile::Iterator::operator*() const -> reference
{
    if (m_end)
    {
        throw std::runtime_error("Dereferencing iterator past end.");
    }

    if (!m_value_read.has_value())
    {
        IndexRecord temp;

        if (m_index_location == IndexLocation::data)
        {
            m_data_file.seekg(calculate_offset(m_index, m_index_location), std::ios::beg);
            temp = m_sf->read_record(m_data_file);
        }
        else if (m_index_location == IndexLocation::aux)
        {
            m_aux_file.seekg(calculate_offset(m_index, m_index_location), std::ios::beg);
            temp = m_sf->read_record(m_aux_file);
        }
        else
        {
            throw std::runtime_error("???");
        }

        m_value_read.emplace(temp);
    }

    return m_value_read.value();
}

auto SequentialFile::Iterator::operator->() const -> pointer
{
    return std::addressof(**this);
}

void SequentialFile::Iterator::advance()
{
    IndexRecord temp = **this;

    m_index = temp.next_position;
    m_index_location = temp.next_file;

    m_value_read.reset();
}

auto SequentialFile::Iterator::operator++() -> Iterator&
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

auto SequentialFile::Iterator::operator++(int) -> Iterator
{
    Iterator it{*this};
    ++it;
    return it;
}

auto SequentialFile::Iterator::operator==(Iterator const& other) -> bool
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

    return this->m_index == other.m_index && this->m_index_location == other.m_index_location;
}

auto SequentialFile::Iterator::operator!=(Iterator const& other) -> bool
{
    return !(*this == other);
}

void SequentialFile::Iterator::advance_until_next_valid()
{
    while (true)
    {
        if (m_index_location == IndexLocation::no_next)
        {
            m_end = true;
            return;
        }

        IndexRecord ir = **this;

        if (ir.deleted == false)
        {
            return;
        }

        throw std::runtime_error("This shouldn't happen");

        this->advance();
    }
}

SequentialFile::SequentialFile(
    std::filesystem::path data_filename,
    std::filesystem::path aux_filename,
    Attribute key_attribute,
    uint64_t max_aux_size)
    : m_data_filename(std::move(data_filename)),
      m_aux_filename(std::move(aux_filename)),
      m_key_attribute(std::move(key_attribute)),
      m_record_size(m_key_attribute.size() + IndexRecord::size_without_key),
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
        m_data_file.write(reinterpret_cast<char const*>(&next_position), sizeof(next_position));
    }
}

SequentialFile::SequentialFile(SequentialFile const& other)
    : SequentialFile(
        other.m_data_filename,
        other.m_aux_filename,
        other.m_key_attribute,
        other.m_max_aux_size)
{
}

auto SequentialFile::begin() const -> Iterator
{
    auto [next_location, next_position] = get_header();
    return Iterator(m_data_filename, m_aux_filename, next_position, next_location, this);
}

auto SequentialFile::end() const -> Iterator
{
    return Iterator(m_data_filename, m_aux_filename, 0, IndexLocation::no_next, this);
}

void SequentialFile::insert(json const& key, uint64_t relation_index)
{
    m_aux_file.seekg(0, std::ios::end);
    if (m_aux_file.tellg() / m_record_size >= m_max_aux_size)
    {
        merge_data();
    }

    auto it_o = find_location_to_add(key);

    IndexRecord ir;
    ir.key = key;
    ir.relation_index = relation_index;
    ir.deleted = false;

    if (it_o.has_value())
    {
        Iterator it = it_o.value();

        ir.next_file = it->next_file;
        ir.next_position = it->next_position;
    }
    else
    {
        auto [next_location, next_index] = get_header();
        ir.next_file = next_location;
        ir.next_position = next_index;
    }

    m_aux_file.seekp(0, std::ios::end);
    uint64_t index_to_add = m_aux_file.tellp() / m_record_size;

    this->write_record(ir, m_aux_file);

    if (!it_o.has_value())
    {
        this->set_header(IndexLocation::aux, index_to_add);
    }
    else
    {
        Iterator it = it_o.value();

        IndexRecord temp = *it;
        temp.next_file = IndexLocation::aux;
        temp.next_position = index_to_add;

        this->write_record(it.m_index_location, it.m_index, temp);
    }
}

auto SequentialFile::remove_record(json key, uint64_t relation_index) -> bool
{
    auto remove_next = [&](Iterator it) -> void {
        IndexRecord ir = *it;

        if (ir.next_file == IndexLocation::no_next)
        {
            return;
        }

        Iterator it_next = it;
        ++it_next;
        IndexRecord ir_next = *it_next;

        ir_next.deleted = true;
        this->write_record(it_next.m_index_location, it_next.m_index, ir_next);

        ir.next_file = ir_next.next_file;
        ir.next_position = ir_next.next_position;

        this->write_record(it.m_index_location, it.m_index, ir);
    };

    std::optional<Iterator> it_o = this->find_location_to_add(key);

    Iterator it = this->begin();
    if (!it_o.has_value())
    {
        if (it == this->end() || it->key != key)
        {
            return false;
        }

        if (it->relation_index == relation_index)
        {
            IndexRecord ir = *it;

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

void SequentialFile::merge_data()
{
    std::filesystem::path new_data_filename = m_data_filename;
    new_data_filename += ".new";
    std::fstream new_data_file(new_data_filename, std::ios::out | std::ios::binary);

    IndexLocation next_location = IndexLocation::data;
    uint64_t next_position = 0;

    new_data_file.write(reinterpret_cast<char const*>(&next_location), sizeof(next_location));
    new_data_file.write(reinterpret_cast<char const*>(&next_position), sizeof(next_position));

    bool written_one = false;

    uint64_t next_index = 1;
    Iterator it = this->begin();

    if (it != this->end())
    {
        written_one = true;

        Iterator it_next = it;
        ++it_next;

        for (; it_next != this->end(); ++it, ++it_next)
        {
            IndexRecord ir = *it;

            ir.next_file = IndexLocation::data;
            ir.next_position = next_index;

            this->write_record(ir, new_data_file);

            ++next_index;
        }

        IndexRecord ir = *it;
        ir.next_file = IndexLocation::no_next;
        ir.next_position = 0;

        this->write_record(ir, new_data_file);
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
    else
    {
        set_header(IndexLocation::data, 0);
    }
}

auto SequentialFile::find_location_to_add(json const& key) const -> std::optional<Iterator>
{
    /*
    ** Returns the last *active* location with a value less than key.
    */
    IndexRecord temp;

    m_data_file.seekg(0, std::ios::end);
    long n_records = m_data_file.tellg() / m_record_size;

    reverse_raw_iterator r_end(RawIterator(m_data_filename, 0, this));
    reverse_raw_iterator r_begin(RawIterator(m_data_filename, n_records, this));

    reverse_raw_iterator r_it =
        std::upper_bound(r_begin, r_end, key, [](json const& j, IndexRecord const& ir) -> bool {
            return ir.key < j;
        });

    while (r_it != r_end)
    {
        if (r_it->deleted == false)
        {
            Iterator it(
                m_data_filename,
                m_aux_filename,
                (r_it.base() - 1).m_index,
                IndexLocation::data,
                this);
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

    Iterator it(m_data_filename, m_aux_filename, next_position, IndexLocation::aux, this);

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

auto SequentialFile::find_location(nlohmann::json const& key) const -> Iterator
{
    /*
    ** Return an iterator to the first position greater or equal to key. Or end() if no such
    ** position exists.
    */

    std::optional<Iterator> it_o = this->find_location_to_add(key);

    Iterator it = this->begin();
    if (!it_o.has_value())
    {
        auto [next_location, next_position] = get_header();
        it = Iterator(m_data_filename, m_aux_filename, next_position, next_location, this);
    }
    else
    {
        it = it_o.value();
    }

    while (it != this->end() && it->key < key)
    {
        ++it;
    }

    return it;
}

auto SequentialFile::get_header() const -> std::pair<IndexLocation, uint64_t>
{
    m_data_file.seekg(0, std::ios::beg);

    IndexLocation index_location{};
    m_data_file.read(reinterpret_cast<char*>(&index_location), sizeof(index_location));

    uint64_t next_position = 0;
    m_data_file.read(reinterpret_cast<char*>(&next_position), sizeof(next_position));

    return {index_location, next_position};
}

void SequentialFile::set_header(IndexLocation index_location, uint64_t next_position) const
{
    m_data_file.seekp(0, std::ios::beg);

    m_data_file.write(reinterpret_cast<char const*>(&index_location), sizeof(index_location));
    m_data_file.write(reinterpret_cast<char const*>(&next_position), sizeof(next_position));

    m_data_file.flush();
}

auto SequentialFile::calculate_offset(uint64_t index, IndexLocation index_location) -> uint64_t
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

void SequentialFile::write_record(
    IndexLocation index_location, uint64_t index, IndexRecord const& record) const
{
    if (index_location == IndexLocation::data)
    {
        m_data_file.seekp(header_size + index * m_record_size);
        this->write_record(record, m_data_file);
    }
    else if (index_location == IndexLocation::aux)
    {
        m_aux_file.seekp(index * m_record_size);
        this->write_record(record, m_aux_file);
    }
    else
    {
        throw std::runtime_error("???");
    }
}

void SequentialFile::write_record(IndexRecord const& ir, std::ostream& os) const
{
    m_key_attribute.write(os, ir.key);
    os.write(reinterpret_cast<char const*>(&ir.relation_index), sizeof(ir.relation_index));
    os.write(reinterpret_cast<char const*>(&ir.next_file), sizeof(ir.next_file));
    os.write(reinterpret_cast<char const*>(&ir.next_position), sizeof(ir.next_position));
    os.write(reinterpret_cast<char const*>(&ir.deleted), sizeof(ir.deleted));

    os.flush();
}

auto SequentialFile::read_record(std::istream& in) const -> IndexRecord
{
    IndexRecord ir;

    ir.key = m_key_attribute.read(in);
    in.read(reinterpret_cast<char*>(&ir.relation_index), sizeof(ir.relation_index));
    in.read(reinterpret_cast<char*>(&ir.next_file), sizeof(ir.next_file));
    in.read(reinterpret_cast<char*>(&ir.next_position), sizeof(ir.next_position));
    in.read(reinterpret_cast<char*>(&ir.deleted), sizeof(ir.deleted));

    return ir;
}
