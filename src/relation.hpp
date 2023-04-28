#pragma once

#include <experimental/filesystem>
#include <filesystem>
#include <map>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"

struct Relation
{
    std::vector<Attribute> m_attributes; // Relation schema
    std::map<int, int> m_indexes;

    virtual auto projection(std::vector<std::string> const& attribute_list)
        -> Relation* = delete;
    virtual auto selection(std::vector<int> const& conditions) -> Relation* = delete;

    Relation(std::vector<Attribute> attributes, std::map<int, int> indexes);

    virtual ~Relation() = default;
};

struct MemoryRelation : public Relation
{
    std::vector<nlohmann::json> m_tuples;

    MemoryRelation(
        std::vector<Attribute> attributes,
        std::map<int, int> indexes,
        std::vector<nlohmann::json> tuples);

    void insert(nlohmann::json tuple);
};

struct FileRelation : public Relation
{
    std::string m_name;
    std::filesystem::path m_filename;

    FileRelation(
        std::vector<Attribute> attributes,
        std::map<int, int> indexes,
        std::string name,
        std::filesystem::path filename);

    auto to_memory() -> MemoryRelation;

    void insert(nlohmann::json const& tuple);
};
