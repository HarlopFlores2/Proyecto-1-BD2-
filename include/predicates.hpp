#pragma once

#include <functional>
#include <string>
#include <variant>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"

/*
** All predicates
** a < c
** a > c
** a == c
** a != c
** c < a
** c > a
** c == a
** c != a
** a BETWEEN c1 AND c2
*/

struct P_A_LESS_C; // Canonical
struct P_C_LESS_A;
struct P_A_GREATER_C; // Canonical
struct P_C_GREATER_A;
struct P_A_EQUAL_C; // Canonical
struct P_C_EQUAL_A;
struct P_A_UNEQUAL_C; // Canonical
struct P_C_UNEQUAL_A;
struct P_A_BETWEEN_C1_C2; // Canonical

using predicate_type = std::variant<
    P_A_LESS_C,
    P_C_LESS_A,
    P_A_GREATER_C,
    P_C_GREATER_A,
    P_A_EQUAL_C,
    P_C_EQUAL_A,
    P_A_UNEQUAL_C,
    P_C_UNEQUAL_A,
    P_A_BETWEEN_C1_C2>;

using tuple_validator_type = std::function<bool(nlohmann::json const&)>;

struct P_A_LESS_C

{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_C_LESS_A
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_A_GREATER_C
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_C_GREATER_A
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_A_EQUAL_C
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_C_EQUAL_A
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_A_UNEQUAL_C
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_C_UNEQUAL_A
{
    std::string m_a;
    nlohmann::json m_c;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};

struct P_A_BETWEEN_C1_C2
{
    std::string m_a;
    nlohmann::json m_c1;
    nlohmann::json m_c2;

    [[nodiscard]] auto canonicalize() const -> predicate_type;
    [[nodiscard]] auto is_valid(std::vector<Attribute> const& attributes) const -> bool;
    [[nodiscard]] auto generate_validator(std::vector<Attribute> const& attributes) const
        -> std::function<bool(nlohmann::json const&)>;
};
