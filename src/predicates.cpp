#include <algorithm>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>

#include "json.hpp"
#include "predicates.hpp"

using json = nlohmann::json;

auto find_attribute(
    std::vector<Attribute> const& attributes,
    std::string const& attribute_name,
    json const& j_constant) -> std::optional<uint64_t>
{
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](Attribute const& a) {
        return a.name == attribute_name;
    });

    if (it == attributes.end())
    {
        return {};
    }
    if (!it->valid_json(j_constant))
    {
        return {};
    }

    return {std::distance(attributes.begin(), it)};
}

auto is_valid_binary_attribute_constant(
    std::vector<Attribute> const& attributes,
    std::string const& attribute_name,
    json const& j_constant) -> bool
{
    return find_attribute(attributes, attribute_name, j_constant).has_value();
}

auto P_A_LESS_C::canonicalize() const -> predicate_type
{
    return {*this};
}

auto P_A_LESS_C::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_A_LESS_C::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return j[index] < this->m_c; };
}

auto P_C_LESS_A::canonicalize() const -> predicate_type
{
    return {
        P_A_GREATER_C{m_a, m_c}
    };
}

auto P_C_LESS_A::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_C_LESS_A::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return this->m_c < j[index]; };
}

auto P_A_GREATER_C::canonicalize() const -> predicate_type
{
    return {*this};
}

auto P_A_GREATER_C::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_A_GREATER_C::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return j[index] > this->m_c; };
}

auto P_C_GREATER_A::canonicalize() const -> predicate_type
{
    return {
        P_A_LESS_C{m_a, m_c}
    };
}

auto P_C_GREATER_A::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_C_GREATER_A::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return this->m_c > j[index]; };
}

auto P_A_EQUAL_C::canonicalize() const -> predicate_type
{
    return {*this};
}

auto P_A_EQUAL_C::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_A_EQUAL_C::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return j[index] == this->m_c; };
}

auto P_C_EQUAL_A::canonicalize() const -> predicate_type
{
    return {
        P_A_EQUAL_C{m_a, m_c}
    };
}

auto P_C_EQUAL_A::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_C_EQUAL_A::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return this->m_c == j[index]; };
}

auto P_A_UNEQUAL_C::canonicalize() const -> predicate_type
{
    return {*this};
}

auto P_A_UNEQUAL_C::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_A_UNEQUAL_C::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return j[index] != this->m_c; };
}

auto P_C_UNEQUAL_A::canonicalize() const -> predicate_type
{
    return {
        P_A_UNEQUAL_C{m_a, m_c}
    };
}

auto P_C_UNEQUAL_A::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    return is_valid_binary_attribute_constant(attributes, m_a, m_c);
}

auto P_C_UNEQUAL_A::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    return [index, this](json const& j) { return this->m_c != j[index]; };
}

auto P_A_BETWEEN_C1_C2::canonicalize() const -> predicate_type
{
    return {*this};
}

auto P_A_BETWEEN_C1_C2::is_valid(std::vector<Attribute> const& attributes) const -> bool
{
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](Attribute const& a) {
        return a.name == m_a;
    });

    if (it == attributes.end())
    {
        return false;
    }

    return it->valid_json(m_c1) && it->valid_json(m_c2);
}

auto P_A_BETWEEN_C1_C2::generate_validator(std::vector<Attribute> const& attributes) const
    -> std::function<bool(json const&)>
{
    auto maybe_index = find_attribute(attributes, m_a, m_c1);

    if (!maybe_index.has_value())
    {
        throw std::runtime_error("Predicate not valid.");
    }

    auto index = maybe_index.value();

    if (!attributes.at(index).valid_json(m_c2))
    {
        throw std::runtime_error("Predicate not valid.");
    }

    return [index, this](json const& j) {
        return this->m_c1 <= j[index] && j[index] <= this->m_c2;
    };
}
