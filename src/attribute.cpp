#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <utility>
#include <variant>

#include "attribute.hpp"
#include "json.hpp"

using json = nlohmann::json;

auto INTEGER::read(std::istream& in) const -> json
{
    type ret = 0;
    in.read(reinterpret_cast<char*>(&ret), sizeof(ret));
    return json{ret};
}

void INTEGER::write(std::ostream& out, json const& j) const
{
    type n = j.get<type>();
    out.write(reinterpret_cast<char const*>(&n), sizeof(n));
}

auto INTEGER::to_specifier() const -> std::string
{
    return {"INTEGER"};
}

auto INTEGER::valid_json(json const& j) const -> bool
{
    return j.type() == json::value_t::number_integer;
}

auto INTEGER::size() const -> uint64_t
{
    return sizeof(INTEGER::type);
}

auto VARCHAR::read(std::istream& in) const -> json
{
    auto ret = new type[n_chars];
    in.read(reinterpret_cast<char*>(ret), n_chars * sizeof(type));
    return json{ret};
}

void VARCHAR::write(std::ostream& out, json const& j) const
{
    std::string s = j.get<std::string>();
    auto s_to_write = std::min(s.size(), n_chars);
    out.write(s.data(), s_to_write);

    char null = '\0';
    for (size_t i = 0; i < n_chars - s_to_write; i++)
    {
        out.write(&null, 1);
    }
}

auto VARCHAR::to_specifier() const -> std::string
{
    return {"VARCHAR(" + std::to_string(this->n_chars) + ")"};
}

auto VARCHAR::valid_json(json const& j) const -> bool
{
    return j.type() == json::value_t::string;
}

auto VARCHAR::size() const -> uint64_t
{
    return sizeof(VARCHAR::type) * n_chars;
}

auto Attribute::read(std::istream& in) const -> json
{
    return std::visit([&](auto& a) { return a.read(in); }, type);
}

void Attribute::write(std::ostream& out, json const& j) const
{
    std::visit([&](auto& a) { return a.write(out, j); }, type);
}

auto Attribute::to_specifier() const -> std::string
{
    return std::visit([&](auto& a) { return a.to_specifier(); }, type);
}

auto Attribute::valid_json(json const& j) const -> bool
{
    return std::visit([&](auto& a) { return a.valid_json(j); }, type);
}

auto Attribute::size() const -> uint64_t
{
    return std::visit([&](auto& a) { return a.size(); }, type);
}
