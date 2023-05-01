#include <cstdint>
#include <experimental/filesystem>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "base_relation.hpp"
#include "file_relation.hpp"
#include "json.hpp"
#include "memory_relation.hpp"

using json = nlohmann::json;

FileRelation::Iterator::Iterator(FileRelation const* fr, uint64_t file_offset)
    : m_file{fr->m_filename, std::ios::binary | std::ios::in},
      m_file_offset{file_offset},
      m_fr{fr},
      m_value_read{}
{
    m_file.exceptions(std::ios::failbit);

    // Make sure we start with the first valid tuple
    this->advance_until_next_valid();
}

FileRelation::Iterator::Iterator(Iterator const& it)
    : Iterator{it.m_fr, it.m_file_offset}
{
}

auto FileRelation::Iterator::operator*() const -> reference
{
    if (m_end)
    {
        throw std::runtime_error("Dereferencing iterator past end.");
    }

    if (!m_value_read.has_value())
    {
        m_file.seekg(m_file_offset);

        uint64_t deleted_p = 0;
        m_file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

        json tuple;
        for (Attribute const& a : m_fr->m_attributes)
        {
            json ja = a.read(m_file);
            tuple.emplace_back(std::move(ja));
        }
        m_value_read.emplace(std::move(tuple));
    }

    return *m_value_read;
}

auto FileRelation::Iterator::operator->() const -> pointer
{
    return std::addressof(**this);
}

auto FileRelation::Iterator::operator++() -> Iterator&
{
    if (m_end)
    {
        return *this;
    }

    m_file_offset += m_fr->m_record_size;
    this->advance_until_next_valid();

    return *this;
}

auto FileRelation::Iterator::operator++(int) -> Iterator
{
    Iterator it{*this};
    ++it;
    return it;
}

auto FileRelation::Iterator::operator--() -> Iterator&
{
    if (m_end)
    {
        m_file.seekg(0, std::ios::end);
        m_file_offset = m_file.tellg();
    }

    uint64_t original_offset = m_file_offset;
    while (m_file_offset - sizeof(FileRelation::record_deleted_and_last) >= m_fr->m_record_size)
    {
        m_file_offset -= m_fr->m_record_size;

        m_file.seekg(m_file_offset, std::ios::beg);
        uint64_t deleted_p = 0;
        m_file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

        if (deleted_p == FileRelation::record_not_deleted)
        {
            m_end = false;
            return *this;
        }
    }

    // Decrementing a pointer to the first element has no effect.
    m_file_offset = original_offset;

    return *this;
}

auto FileRelation::Iterator::operator--(int) -> Iterator
{
    Iterator it{*this};
    --it;
    return it;
}

auto operator==(FileRelation::Iterator const& a, FileRelation::Iterator const& b) -> bool
{
    if (a.m_fr != b.m_fr)
    {
        return false;
    }
    if (a.m_end == true || b.m_end == true)
    {
        return a.m_end == b.m_end;
    }
    return a.m_file_offset == b.m_file_offset;
}

auto operator!=(FileRelation::Iterator const& a, FileRelation::Iterator const& b) -> bool
{
    return !(a == b);
}

void FileRelation::Iterator::advance_until_next_valid()
{
    uint64_t deleted_p = 0;
    while (m_file.peek(), !m_file.eof())
    {
        m_file.seekg(m_file_offset, std::ios::beg);
        m_file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

        if (deleted_p == FileRelation::record_not_deleted)
        {
            return;
        }

        m_file_offset += m_fr->m_record_size;
    }

    m_end = true;
}

FileRelation::FileRelation(
    std::vector<Attribute> attributes,
    std::map<int, int> indexes,
    std::string name,
    std::filesystem::path filename)
    : Relation(std::move(attributes), std::move(indexes)),
      m_record_size(m_tuple_size + sizeof(record_not_deleted)),
      m_name(std::move(name)),
      m_filename(std::move(filename))
{
    if (std::filesystem::exists(m_filename))
    {
        return;
    }

    std::ofstream of{m_filename, std::fstream::out | std::fstream::binary};
    if (of.bad())
    {
        throw std::runtime_error(
            "File " + std::quoted(m_filename.string())._M_string + " couldn't be created.");
    }

    of.write(
        reinterpret_cast<char const*>(&record_deleted_and_last), sizeof(record_not_deleted));
}

auto FileRelation::begin() const -> Iterator
{
    return {this, sizeof(record_deleted_and_last)};
}

auto FileRelation::end() const -> Iterator
{
    Iterator ret = this->begin();
    ret.m_end = true;

    return ret;
}

auto FileRelation::to_memory() -> MemoryRelation
{
    std::ifstream file{m_filename, std::ios::in};
    if (!file.good())
    {
        throw std::runtime_error(
            std::quoted(m_filename.string())._M_string + " can't be read.\n");
    }

    file.exceptions(std::ios::badbit);

    // Skip over pointer to first deleted
    file.seekg(sizeof(record_not_deleted), std::ios::cur);

    std::vector<json> tuples;
    while (file.peek(), !file.eof())
    {
        uint64_t deleted_p;
        file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

        // If record is active
        if (deleted_p == record_not_deleted)
        {
            json tuple;
            for (Attribute const& a : m_attributes)
            {
                json ja = a.read(file);
                tuple.emplace_back(std::move(ja));
            }
            tuples.emplace_back(std::move(tuple));
        }
        // Else, record is skipped over
        else
        {
            file.seekg(m_tuple_size);
        }
    }

    return MemoryRelation{m_attributes, m_indexes, std::move(tuples)};
}

void FileRelation::insert(nlohmann::json const& tuple)
{
    // TODO: Check if primary key exists

    if (!tuple.is_array())
    {
        throw std::runtime_error("'tuple' given wasn't an array");
    }
    if (m_attributes.size() != tuple.size())
    {
        throw std::runtime_error("'tuple' given was the wrong size");
    }

    auto t_it = tuple.begin();
    for (auto a_it = m_attributes.begin(); a_it != m_attributes.end(); a_it++, t_it++)
    {
        if (!a_it->valid_json(*t_it))
        {
            throw std::runtime_error(
                "'tuple' " + std::to_string(std::distance(tuple.begin(), t_it))
                + " element was not of type " + a_it->to_specifier());
        }
    }

    std::fstream file{m_filename, std::ios::in | std::ios::out | std::ios::binary};

    uint64_t deleted_p = 0;
    file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

    if (deleted_p == record_deleted_and_last)
    {
        file.seekp(0, std::ios::end);
    }
    else
    {
        auto offset_to_write = calculate_offset(deleted_p);

        file.seekg(offset_to_write, std::ios::beg);
        file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<char const*>(&deleted_p), sizeof(deleted_p));

        file.seekg(offset_to_write, std::ios::beg);
    }

    file.write(reinterpret_cast<char const*>(&record_not_deleted), sizeof(record_not_deleted));

    t_it = tuple.begin();
    for (auto a_it = m_attributes.begin(); a_it != m_attributes.end(); a_it++, t_it++)
    {
        a_it->write(file, *t_it);
    }
}

auto FileRelation::remove(uint64_t index) -> bool
{
    std::fstream file{m_filename, std::ios::in | std::ios::out | std::ios::binary};

    uint64_t offset = calculate_offset(index);
    file.seekg(offset, std::ios::beg);

    size_t deleted_p = 0;
    file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));

    if (deleted_p != record_not_deleted)
    {
        return false;
    }

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&deleted_p), sizeof(deleted_p));
    file.seekg(0, std::ios::beg);
    file.write(reinterpret_cast<char const*>(&index), sizeof(index));

    file.seekg(offset, std::ios::beg);
    file.write(reinterpret_cast<char const*>(&deleted_p), sizeof(deleted_p));

    return true;
}

auto FileRelation::read(uint64_t index) const -> json
{
    // Does not verify whether the tuple is deleted

    std::ifstream file{m_filename, std::ios::in | std::ios::binary};

    uint64_t offset = calculate_offset(index);
    file.seekg(offset, std::ios::beg);
    file.seekg(sizeof(record_deleted_and_last), std::ios::cur);

    json tuple;
    for (Attribute const& a : m_attributes)
    {
        json ja = a.read(file);
        tuple.emplace_back(std::move(ja));
    }

    return tuple;
}

auto FileRelation::calculate_offset(uint64_t index) const -> uint64_t
{
    return sizeof(record_deleted_and_last) + index * m_record_size;
}
