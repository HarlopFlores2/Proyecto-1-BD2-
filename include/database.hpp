#pragma once

#include <experimental/filesystem>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "file_relation.hpp"
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
        std::string const& name,
        std::vector<Attribute> attributes,
        std::optional<std::string> const& primary_key) -> FileRelation&;

    static auto generate_relation_filename(std::string const& relation_name) -> std::string;
    [[nodiscard]] auto get_relation(std::string const& relation_name) -> FileRelation&;
    [[nodiscard]] auto get_relation(std::string const& relation_name) const
        -> FileRelation const&;

    [[nodiscard]] auto project(
        std::string const& relation_name,
        std::vector<std::string> const& attributes_names) const -> MemoryRelation;
    [[nodiscard]] auto select(
        std::string const& relation_name, std::vector<predicate_type> const& predicates) const
        -> MemoryRelation;
    void insert(std::string const& relation_name, nlohmann::json const& tuple);
    void
    remove(std::string const& relation_name, std::vector<predicate_type> const& predicates);

    auto evaluate(ParsedExpression const& pe) -> std::optional<MemoryRelation>;
};
