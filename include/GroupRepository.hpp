#pragma once
#include"IRepository.hpp"
#include "SQLiteCpp/Database.h"
#include"model.hpp"
#include <vector>
class GroupRepository : public IRepository<Group>
{
    private:
    SQLite::Database& db;
    public:
    explicit GroupRepository(SQLite::Database& db);
    bool add(const Group& entity) override;
    bool remove(const std::string& name) override;
    bool update(const std::string& token,const std::string& value,const std::string& name) override;
    std::vector<Group> search(const std::string& token,const std::string& value) override;
    std::vector<Group> fuzzySearch(const std::string& token,const std::string& value) override;
    bool exist(const std::string& name) override;
    std::optional<int> getDatabaseID(const std::string& value) override;
};
