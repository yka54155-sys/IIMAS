#pragma once
#include"IRepository.hpp"
#include "SQLiteCpp/Database.h"
#include"model.hpp"
class SAGRepository : public IRepository<SAG>
{
    private:
    SQLite::Database& db;
    public:
    explicit SAGRepository(SQLite::Database& db);
    bool add(const SAG& entity) override;
    bool remove(const std::string& id) override;
    bool update(const std::string& token,const std::string& value,const std::string& id) override;
    std::vector<SAG> search(const std::string& token,const std::string& value) override;
    bool exist(const std::string& stu_id,const std::string& grp_id);
    [[nodiscard]]bool exist(const std::string& identifier) override;
};
