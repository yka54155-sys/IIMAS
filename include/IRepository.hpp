#pragma once
#include"std.hpp"
#include <stdexcept>

template <typename Entity>
class IRepository
{
    public:
    virtual ~IRepository() = default;
    [[nodiscard]] virtual bool add(const Entity& entity) = 0;
    
    [[nodiscard]] virtual bool remove(const std::string& identifier) = 0;
    [[nodiscard]] virtual bool update(const std::string& token,const std::string& value,const std::string& id) = 0;
    [[nodiscard]] virtual std::vector<Entity> search(const std::string& token,const std::string& value) = 0;
    
    [[nodiscard]] virtual bool exist(const std::string& identifier)
    {
        throw std::logic_error("exist() NOT implemented for this repository type");
    }

    [[nodiscard]] virtual bool addBatch(const std::vector<Entity>& entities)
    {
        throw std::logic_error("addBatch() NOT implemented for this repository type");
    }
    [[nodiscard]] virtual std::vector<Entity> exactSearch(const std::string& token,const std::string& value)
    {
        throw std::logic_error("exactSearch() NOT implemented for this repository type");
    }
    [[nodiscard]] virtual std::vector<Entity> fuzzySearch(const std::string& token,const std::string& value)
    {
        throw std::logic_error("fuzzySearch() NOT implemented for this repository type");
    }
    [[nodiscard]] virtual std::optional<int> getDatabaseID(const std::string& value)
    {
        throw std::logic_error("getDatabaseID() NOT implemented for this repository type");
    }
};