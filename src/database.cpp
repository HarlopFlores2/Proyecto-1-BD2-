#include <experimental/filesystem>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "database.hpp"
#include "json.hpp"
#include "relation.hpp"

using json = nlohmann::json;

std::filesystem::path const DataBase::db_file = "db.inf";

DataBase::DataBase(std::string name)
    : m_name{std::move(name)},
      m_path(m_name)
{
    if (std::filesystem::exists(m_path))
    {
        if (std::filesystem::is_directory(m_path))
        {
            // TODO: Read data back
            throw std::runtime_error("Not implemented");
        }
        else
        {
            throw std::runtime_error(m_path.string() + " already exists but is a file.");
        }
    }

    std::filesystem::create_directories(m_path);
    std::ofstream of{m_path / db_file, std::fstream::out | std::fstream::binary};
    if (of.bad())
    {
        throw std::runtime_error("File couldn't be created.");
    }

    m_db_info["name"] = m_name;
    m_db_info["relations"] = json::object();

    of << m_db_info;
}

DataBase::~DataBase()
{
    std::filesystem::create_directories(m_path);
    std::ofstream of{m_path / db_file, std::fstream::out | std::fstream::binary};
    of << m_db_info;
}

auto DataBase::create_relation(
    std::string const& name, std::vector<Attribute> attributes, std::string primary_key)
    -> FileRelation&
{
    if (m_relations.count(name) != 0)
    {
        throw std::runtime_error("Relation " + name + " already exists.");
    }

    auto filename = m_path / generate_relation_filename(name);
    auto [it, inserted_p] = m_relations.insert({
        name, FileRelation{std::move(attributes), {}, name, filename}
    });

    m_db_info["relations"][name]["pk"] = primary_key;
    json j_attributes = json::object();
    for (Attribute a : it->second.m_attributes)
    {
        j_attributes[a.name] = a.to_specifier();
    }
    m_db_info["relations"][name]["attributes"] = j_attributes;
    m_db_info["relations"][name]["indexes"] = json::object();

    // TODO: Create index for primary key

    return it->second;
}

auto DataBase::generate_relation_filename(std::string const& relation_name) -> std::string
{
    return relation_name + ".data";
}
