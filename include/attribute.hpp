#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>
#include <utility>
#include <variant>

#include "attribute.hpp"
#include "json.hpp"

struct INTEGER
{
    using type = int64_t;

    auto operator==(INTEGER const& other) const -> bool;

    auto read(std::istream& in) const -> nlohmann::json;
    void write(std::ostream& out, nlohmann::json const& j) const;

    [[nodiscard]] auto to_specifier() const -> std::string;

    [[nodiscard]] auto valid_json(nlohmann::json const& j) const -> bool;

    [[nodiscard]] auto size() const -> uint64_t;

    friend auto operator<<(std::ostream& out, INTEGER const& i) -> std::ostream&;

public:
    static auto from_specifier(std::string const& specifier) -> std::optional<INTEGER>;
};

struct VARCHAR
{
    using type = char;
    uint64_t n_chars;

    auto operator==(VARCHAR const& other) const -> bool;

    auto read(std::istream& in) const -> nlohmann::json;
    void write(std::ostream& out, nlohmann::json const& j) const;

    [[nodiscard]] auto to_specifier() const -> std::string;

    [[nodiscard]] auto valid_json(nlohmann::json const& j) const -> bool;

    [[nodiscard]] auto size() const -> uint64_t;

    friend auto operator<<(std::ostream& out, VARCHAR const& vc) -> std::ostream&;

public:
    static auto from_specifier(std::string const& specifier) -> std::optional<VARCHAR>;
};

using attribute_type = std::variant<INTEGER, VARCHAR>;

struct Attribute
{
    std::string name;
    attribute_type type;

    Attribute(std::string name, attribute_type type);
    Attribute(std::string name, std::string const& type_string);

    auto operator==(Attribute const& other) const -> bool;

    auto read(std::istream& in) const -> nlohmann::json;
    void write(std::ostream& out, nlohmann::json const& j) const;

    [[nodiscard]] auto to_specifier() const -> std::string;

    [[nodiscard]] auto valid_json(nlohmann::json const& j) const -> bool;

    [[nodiscard]] auto size() const -> uint64_t;

    friend auto operator<<(std::ostream& out, Attribute const& vc) -> std::ostream&;
};
