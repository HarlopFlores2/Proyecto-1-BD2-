#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"
#include "relation.hpp"

using json = nlohmann::json;

Relation::Relation(std::vector<Attribute> attributes, std::map<int, int> indexes)
    : m_attributes{std::move(attributes)},
      m_indexes{std::move(indexes)},
      m_tuple_size{0}
{
    for (Attribute const& a : m_attributes)
    {
        m_tuple_size += a.size();
    }
}

MemoryRelation::MemoryRelation(
    std::vector<Attribute> attributes,
    std::map<int, int> indexes,
    std::vector<nlohmann::json> tuples)
    : Relation(std::move(attributes), std::move(indexes)),
      m_tuples(std::move(tuples))
{
}

void MemoryRelation::insert(nlohmann::json tuple)
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

    m_tuples.emplace_back(std::move(tuple));
}

FileRelation::FileRelation(
    std::vector<Attribute> attributes,
    std::map<int, int> indexes,
    std::string name,
    std::filesystem::path filename)
    : Relation(std::move(attributes), std::move(indexes)),
      m_name(std::move(name)),
      m_filename(std::move(filename))
{
    if (std::filesystem::exists(filename))
    {
        return;
    }

    std::ofstream of{filename, std::fstream::out | std::fstream::binary};
    if (of.bad())
    {
        throw std::runtime_error(
            "File " + std::quoted(filename.string())._M_string + " couldn't be created.");
    }

    of.write(
        reinterpret_cast<char const*>(&record_deleted_and_last), sizeof(record_not_deleted));
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

    std::ofstream of{m_filename, std::fstream::app | std::fstream::binary};
    t_it = tuple.begin();
    for (auto a_it = m_attributes.begin(); a_it != m_attributes.end(); a_it++, t_it++)
    {
        a_it->write(of, *t_it);
    }
}
