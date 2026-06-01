#pragma once
#include "IRepository.hpp"
#include"std.hpp"
#include"model.hpp"

struct GroupQuery
{
    std::string token;
    std::string value;
    std::string name = "#";
    
    enum class SearchMode
    {
        NONE,
        EXACT,
        FUZZY
    };
    SearchMode mode = GroupQuery::SearchMode::NONE;
};
struct GroupResult
{
    enum class Status
    {
        SUCCESS,
        FAILURE,
        INVALID_INPUT,
        ERROR
    };
    Status status;
    std::string message;
    std::vector<Group> data;

    static GroupResult success(std::string msg) {
        return { Status::SUCCESS, std::move(msg), {} };
    }
    static GroupResult success(std::string msg, std::vector<Group> data) {
        return { Status::SUCCESS, std::move(msg), std::move(data) };
    }
    static GroupResult failure(std::string msg) {
        return { Status::FAILURE, std::move(msg), {} };
    }
    static GroupResult invalidInput(std::string msg) {
        return { Status::INVALID_INPUT, std::move(msg), {} };
    }
    static GroupResult error(std::string msg) {
        return { Status::ERROR, std::move(msg), {} };
    }
};

class GroupService
{
    private:
    IRepository<Group>& grp_repo;
    public:
    explicit GroupService(IRepository<Group>& grp_repo);

    GroupResult addGroup(const Group& group);
    GroupResult removeGroup(const std::string& name);
    GroupResult updateGroup(const GroupQuery& query);
    GroupResult searchGroup(const GroupQuery& query);
};
