#pragma once

#include <experimental/filesystem>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "json.hpp"
#include "memory_relation.hpp"
#include "parser.hpp"
#include "predicates.hpp"
#include "relation.hpp"

struct DataBase
{
    static std::filesystem::path const db_file;

    std::string m_name;
    std::filesystem::path m_path;

    std::map<std::string, FileRelation> m_relations;

    nlohmann::json m_db_info;

    DataBase(std::string name);
    ~DataBase();

    auto create_relation(
        std::string const& name, std::vector<Attribute> attributes, std::string primary_key)
        -> FileRelation&;

    static auto generate_relation_filename(std::string const& relation_name) -> std::string;

    [[nodiscard]] auto project(
        std::string const& relation_name,
        std::vector<std::string> const& attributes_names) const -> MemoryRelation;
    [[nodiscard]] auto select(
        std::string const& relation_name, std::vector<predicate_type> const& predicates) const
        -> MemoryRelation;

    [[nodiscard]] auto evaluate(ParsedExpression const& pe) const
        -> std::optional<MemoryRelation>;
};