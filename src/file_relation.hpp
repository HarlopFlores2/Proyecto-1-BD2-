#pragma once

#include <cstdint>
#include <experimental/filesystem>
#include <filesystem>
#include <limits>
#include <map>
#include <string>

#include "attribute.hpp"
#include "base_relation.hpp"
#include "json.hpp"
#include "memory_relation.hpp"

struct FileRelation : public Relation
{
    constexpr static uint64_t const record_not_deleted = std::numeric_limits<uint64_t>::max();
    constexpr static uint64_t const record_deleted_and_last =
        std::numeric_limits<uint64_t>::max() - 1;

    std::string m_name;
    std::filesystem::path m_filename;

    FileRelation(
        std::vector<Attribute> attributes,
        std::map<int, int> indexes,
        std::string name,
        std::filesystem::path filename);

    auto to_memory() -> MemoryRelation;

    void insert(nlohmann::json const& tuple);
    auto remove(uint64_t index) -> bool;

    [[nodiscard]] auto calculate_offset(uint64_t index) const -> uint64_t;
};
