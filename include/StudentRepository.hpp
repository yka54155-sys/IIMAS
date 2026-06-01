#pragma once
#include"IRepository.hpp"
#include"std.hpp"
#include"model.hpp"
#include"SQLiteCpp/Database.h"
#include <vector>

class StudentRepository : public IRepository<Student>
{
    private:
    SQLite::Database& db;
    public:
    explicit StudentRepository(SQLite::Database& db);
    
    bool add(const Student& entity) override;
    bool remove(const std::string& id) override;
    bool update(const std::string& token,const std::string& value,const std::string& id) override;
    std::vector<Student> search(const std::string& token,const std::string& value) override;
    bool exist(const std::string& id) override;
    bool addBatch(const std::vector<Student>& entities) override;
    std::vector<Student> fuzzySearch(const std::string& token,const std::string& value) override;
    std::optional<int> getDatabaseID(const std::string& value) override;
};