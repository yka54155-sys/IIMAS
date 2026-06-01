#pragma once
#include"IRepository.hpp"
#include "SQLiteCpp/Database.h"
#include"model.hpp"
class UserRepository : public IRepository<User>
{
    private:
    SQLite::Database& db;
    public:
    explicit UserRepository(SQLite::Database& db);
    bool add(const User& entity) override;
    bool remove(const std::string& id) override;
    bool update(const std::string& token,const std::string& value,const std::string& id) override;
    std::vector<User> search(const std::string& token,const std::string& value) override;
    bool exist(const std::string& id) override;
};
