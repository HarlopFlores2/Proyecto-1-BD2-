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

MemoryRelation::MemoryRelation(
    std::vector<Attribute> attributes,
    std::map<int, int> indexes,
    std::vector<nlohmann::json> tuples)
    : m_attributes(std::move(attributes)),
      m_indexes(std::move(indexes)),
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
    : m_attributes(std::move(attributes)),
      m_indexes(std::move(indexes)),
      m_name(std::move(name)),
      m_filename(std::move(filename))
{
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

    std::vector<json> tuples;

    while (file.peek(), !file.eof())
    {
        json tuple;
        for (Attribute const& a : m_attributes)
        {
            json ja = a.read(file);
            tuple.emplace_back(std::move(ja));
        }
        tuples.emplace_back(std::move(tuple));
    }

    return MemoryRelation{m_attributes, m_indexes, tuples};
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
