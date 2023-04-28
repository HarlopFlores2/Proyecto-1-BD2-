#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <utility>
#include <variant>

#include "attribute.hpp"
#include "json.hpp"

struct INTEGER
{
    using type = int64_t;

    auto read(std::istream& in) const -> nlohmann::json;
    void write(std::ostream& out, nlohmann::json const& j) const;

    [[nodiscard]] auto to_specifier() const -> std::string;

    [[nodiscard]] auto valid_json(nlohmann::json const& j) const -> bool;

    [[nodiscard]] auto size() const -> uint64_t;
};

struct VARCHAR
{
    using type = char;
    uint64_t n_chars;

    auto read(std::istream& in) const -> nlohmann::json;
    void write(std::ostream& out, nlohmann::json const& j) const;

    [[nodiscard]] auto to_specifier() const -> std::string;

    [[nodiscard]] auto valid_json(nlohmann::json const& j) const -> bool;

    [[nodiscard]] auto size() const -> uint64_t;
};

using attribute_type = std::variant<INTEGER, VARCHAR>;

struct Attribute
{
    std::string name;
    attribute_type type;

    auto read(std::istream& in) const -> nlohmann::json;
    void write(std::ostream& out, nlohmann::json const& j) const;

    [[nodiscard]] auto to_specifier() const -> std::string;

    [[nodiscard]] auto valid_json(nlohmann::json const& j) const -> bool;

    [[nodiscard]] auto size() const -> uint64_t;
};
