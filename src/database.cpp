#include <cstdint>
#include <experimental/filesystem>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "attribute.hpp"
#include "csv.hpp"
#include "database.hpp"
#include "file_relation.hpp"
#include "index.hpp"
#include "json.hpp"
#include "memory_relation.hpp"
#include "operations.hpp"
#include "parser.hpp"
#include "relation.hpp"
#include "sequential_file.hpp"
#include "util.hpp"

using json = nlohmann::json;

std::filesystem::path const DataBase::db_file = "db.inf";

DataBase::DataBase(std::string name)
    : m_name{std::move(name)},
      m_path(m_name)
{
    std::filesystem::path info_file = m_path / db_file;

    if (std::filesystem::exists(m_path))
    {
        if (!std::filesystem::is_directory(m_path))
        {
            throw std::runtime_error(m_path.string() + " already exists but is a file.");
        }

        std::ifstream in{info_file, std::ios::in | std::ios::binary};

        if (in.bad())
        {
            throw std::runtime_error(
                "Info file " + info_file.string() + " could not be opened for reading");
        }

        json json_info = json::parse(in);

        // TODO: Not just crash and burn

        for (auto const& [name, j_def] :
             json_info["relations"].get<std::map<std::string, json>>())
        {
            if (m_relations.count(name) != 0)
            {
                throw std::runtime_error("Relation " + name + " repeated in information file.");
            }

            std::optional<std::string> primary_key;
            auto pk_it = j_def.find("pk");
            if (pk_it != j_def.end())
            {
                primary_key = pk_it->get<std::string>();
            }

            std::vector<Attribute> attributes;
            for (auto const& [a_name, a_type] :
                 j_def["attributes"].get<std::vector<std::tuple<std::string, std::string>>>())
            {
                attributes.emplace_back(a_name, a_type);
            }

            FileRelation& fr = this->create_relation(name, attributes, primary_key);

            for (auto const& [attribute, specification] :
                 j_def["indexes"].get<std::map<std::string, json>>())
            {
                auto a_it =
                    std::find_if(attributes.begin(), attributes.end(), [&](Attribute const& a) {
                        return a.name == attribute;
                    });

                if (a_it == attributes.end())
                {
                    throw std::runtime_error("Attribute not in relation");
                }

                for (auto const& [index_type, specification] :
                     specification.get<std::map<std::string, json>>())
                {
                    if (index_type == "sequential")
                    {
                        SequentialFile sf(
                            specification.at("data_file").get<std::string>(),
                            specification.at("aux_file").get<std::string>(),
                            *a_it,
                            specification.at("max_aux").get<uint64_t>());

                        fr.m_indexes.emplace_back(sf);
                    }
                    else
                    {
                        throw std::runtime_error("Not implemented");
                    }
                }
            }
        }

        // TODO: Handle primary key

        return;
    }

    std::filesystem::create_directories(m_path);
    std::ofstream of{info_file, std::fstream::out | std::fstream::binary};
    if (of.bad())
    {
        throw std::runtime_error("File couldn't be created.");
    }

    m_db_info["name"] = m_name;
    m_db_info["relations"] = json::object();

    of << m_db_info;
}

DataBase::~DataBase() = default;

auto DataBase::create_relation(
    std::string const& name,
    std::vector<Attribute> attributes,
    std::optional<std::string> const& primary_key) -> FileRelation&
{
    if (m_relations.count(name) != 0)
    {
        throw std::runtime_error("Relation " + name + " already exists.");
    }

    auto filename = m_path / generate_relation_filename(name);
    auto [it, inserted_p] = m_relations.insert({
        name, FileRelation{std::move(attributes), {}, name, filename}
    });

    if (primary_key.has_value())
    {
        m_db_info["relations"][name]["pk"] = primary_key.value();
    }

    json j_attributes = json::array();
    for (Attribute const& a : it->second.m_attributes)
    {
        j_attributes.emplace_back(std::make_pair(a.name, a.to_specifier()));
    }
    m_db_info["relations"][name]["attributes"] = j_attributes;
    m_db_info["relations"][name]["indexes"] = json::object();

    std::ofstream of{m_path / db_file, std::fstream::out | std::fstream::binary};
    of << m_db_info;

    // TODO: Create index for primary key

    return it->second;
}

void DataBase::remove_relation(std::string const& name)
{
    auto rel_it = m_relations.find(name);
    if (rel_it == m_relations.end())
    {
        throw std::runtime_error("Relation " + name + " does not exist.");
    }

    if (!std::filesystem::remove(rel_it->second.m_filename))
    {
        throw std::runtime_error("Removal of relation file failed.");
    }
    m_relations.erase(rel_it);

    auto db_info_it = m_db_info["relations"].find(name);
    assert(db_info_it != m_db_info["relations"].end());
    m_db_info["relations"].erase(db_info_it);

    std::ofstream of{m_path / db_file, std::fstream::out | std::fstream::binary};
    of << m_db_info;
}

void DataBase::create_index(
    std::string const& relation, IndexType index_type, std::string const& attribute)
{
    FileRelation& fr = get_relation(relation);

    auto a_it =
        std::find_if(fr.m_attributes.begin(), fr.m_attributes.end(), [&](Attribute const& a) {
            return a.name == attribute;
        });

    if (a_it == fr.m_attributes.end())
    {
        throw std::runtime_error(
            "Attribute " + attribute + " does not exist in relation " + relation + ".");
    }

    uint16_t attribute_index = std::distance(fr.m_attributes.begin(), a_it);

    if (index_type == IndexType::SequentialFile)
    {
        std::filesystem::path data_index_file = fr.m_filename;
        data_index_file += ".seq." + attribute + ".data";
        std::filesystem::path aux_index_file = fr.m_filename;
        aux_index_file += ".seq." + attribute + ".aux";

        uint64_t max_aux_size = 100;

        SequentialFile sf(data_index_file, aux_index_file, *a_it, max_aux_size);

        for (FileRelation::Iterator it = fr.begin(); it != fr.end(); ++it)
        {
            sf.insert((*it)[attribute_index], it.calculate_index());
        }

        fr.m_indexes.emplace_back(sf);

        m_db_info["relations"][relation]["indexes"][attribute]["sequential"]["data_file"] =
            data_index_file.string();
        m_db_info["relations"][relation]["indexes"][attribute]["sequential"]["aux_file"] =
            aux_index_file.string();
        m_db_info["relations"][relation]["indexes"][attribute]["sequential"]["max_aux"] =
            max_aux_size;

        std::ofstream of{m_path / db_file, std::fstream::out | std::fstream::binary};
        of << m_db_info;
    }
    else
    {
        throw std::runtime_error("Not implemented");
    }
}

auto DataBase::generate_relation_filename(std::string const& relation_name) -> std::string
{
    return relation_name + ".data";
}

auto DataBase::get_relation(std::string const& relation_name) -> FileRelation&
{
    auto it = m_relations.find(relation_name);

    if (it == m_relations.end())
    {
        throw std::runtime_error("Relation " + relation_name + " does not exist.");
    }

    return it->second;
}

auto DataBase::get_relation(std::string const& relation_name) const -> FileRelation const&
{
    auto it = m_relations.find(relation_name);

    if (it == m_relations.end())
    {
        throw std::runtime_error("Relation " + relation_name + " does not exist.");
    }

    return it->second;
}

auto DataBase::project(
    std::string const& relation_name, std::vector<std::string> const& attributes_names) const
    -> MemoryRelation
{
    return ::project(this->get_relation(relation_name), attributes_names);
}

auto DataBase::select(
    std::string const& relation_name, std::vector<predicate_type> const& predicates) const
    -> MemoryRelation
{
    return ::select(this->get_relation(relation_name), predicates);
}

void DataBase::insert(std::string const& relation_name, nlohmann::json const& tuple)
{
    return this->get_relation(relation_name).insert(tuple);
}

void DataBase::remove(
    std::string const& relation_name, std::vector<predicate_type> const& predicates)
{
    ::remove(this->get_relation(relation_name), predicates);
}

auto DataBase::evaluate(ParsedExpression const& pe) -> std::optional<MemoryRelation>
{
    auto ret = std::visit(
        overloaded{
            [&](SelectExpression const& se) -> std::optional<MemoryRelation> {
                MemoryRelation relation_filtered = this->select(se.relation, se.predicates);
                if (se.attributes.size() == 1 && se.attributes.front() == "*")
                {
                    return {relation_filtered};
                }

                std::vector<std::string> attributes_to_project;
                for (std::string const& a : se.attributes)
                {
                    if (a == "*")
                    {
                        for (Attribute const& a : relation_filtered.m_attributes)
                        {
                            attributes_to_project.push_back(a.name);
                        }
                    }
                    else
                    {
                        attributes_to_project.push_back(a);
                    }
                }

                return {::project(relation_filtered, attributes_to_project)};
            },
            [&](CreateTableExpression const& cte) -> std::optional<MemoryRelation> {
                this->create_relation(cte.name, cte.attributes, cte.primary_key);
                return {};
            },
            [&](InsertValuesExpression const& other) -> std::optional<MemoryRelation> {
                this->insert(other.relation, other.tuple);
                return {};
            },
            [&](InsertCSVExpression const& i_csv_e) -> std::optional<MemoryRelation> {
                FileRelation& r = this->get_relation(i_csv_e.relation);

                csv::CSVReader reader(i_csv_e.filename);

                std::vector<json> tuples_to_add;

                for (csv::CSVRow& row : reader)
                {
                    if (row.size() != r.m_attributes.size())
                    {
                        throw std::runtime_error("Wrong number of elements in CSV row");
                    }

                    json tuple = json::array();

                    auto a_it = r.m_attributes.begin();
                    for (csv::CSVField& field : row)
                    {
                        auto s_field = field.get<>();

                        std::visit(
                            overloaded{
                                [&](INTEGER const& /*i_t*/) {
                                    int64_t i = std::stoll(s_field);
                                    return tuple.push_back(i);
                                },
                                [&](VARCHAR const& /*vc_t*/) { tuple.push_back(s_field); }},
                            a_it->type);

                        ++a_it;
                    }

                    tuples_to_add.emplace_back(std::move(tuple));
                }

                for (auto& tuple : tuples_to_add)
                {
                    r.insert(tuple);
                }

                return {};
            },
            [&](DropTableExpression const& dt_e) -> std::optional<MemoryRelation> {
                remove_relation(dt_e.relation);
                return {};
            },
            [&](CreateIndexExpression const& ci_e) -> std::optional<MemoryRelation> {
                create_index(ci_e.relation, ci_e.index_type, ci_e.attribute);
                return {};
            }},

        pe);

    return ret;
}
