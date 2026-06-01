#include"GroupService.hpp"
#include "IRepository.hpp"
#include "model.hpp"
#include <iostream>
#include <vector>

GroupService::GroupService(IRepository<Group>& grp_repo) : grp_repo(grp_repo)
{
    ;
}

GroupResult GroupService::addGroup(const Group& group)
{
    try
    {
        if(group.Group_Name.empty())
        {
            return GroupResult::invalidInput("addGroup: group name is empty");
        }
        if(!grp_repo.add(group))
        {
            return GroupResult::error("addGroup: database error, check GroupRepository.add()");
        }
        return GroupResult::success("addGroup: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[GroupService::addGroup] " << e.what() << '\n';
        return GroupResult::error("addGroup: internal error");
    }
}

GroupResult GroupService::removeGroup(const std::string& name)
{
    try
    {
        if(name.empty())
        {
            return GroupResult::invalidInput("removeGroup: name is empty");
        }
        if(!grp_repo.exist(name))
        {
            return GroupResult::failure("removeGroup: group not found");
        }
        if(!grp_repo.remove(name))
        {
            return GroupResult::error("removeGroup: database error, check GroupRepository.remove()");
        }
        return GroupResult::success("removeGroup: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[GroupService::removeGroup] " << e.what() << '\n';
        return GroupResult::error("removeGroup: internal error");
    }
}

GroupResult GroupService::updateGroup(const GroupQuery& query)
{
    try
    {
        if(query.name.empty() || query.name == "#")
        {
            return GroupResult::invalidInput("updateGroup: name param is empty");
        }
        if (GroupMap.find(query.token) == GroupMap.end())
        {
            return GroupResult::invalidInput("updateGroup: unknown token");
        }
        if (!grp_repo.exist(query.name))
        {
            return GroupResult::failure("updateGroup: group not found");
        }
        if (!grp_repo.update(query.token, query.value, query.name))
        {
            return GroupResult::error("updateGroup: database error, check GroupRepository.update()");
        }
        return GroupResult::success("updateGroup: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[GroupService::updateGroup] " << e.what() << '\n';
        return GroupResult::error("updateGroup: internal error");
    }
}

GroupResult GroupService::searchGroup(const GroupQuery& query)
{
    try
    {
        if(query.token.empty() || query.value.empty())
        {
            return GroupResult::invalidInput("searchGroup: token or value is empty");
        }
        if (GroupMap.find(query.token) == GroupMap.end())
        {
            return GroupResult::invalidInput("searchGroup: unknown token");
        }

        std::vector<Group> result;
        switch (query.mode)
        {
            case GroupQuery::SearchMode::EXACT:
                result = grp_repo.search(query.token, query.value);
                break;
            case GroupQuery::SearchMode::FUZZY:
                result = grp_repo.fuzzySearch(query.token, query.value);
                break;
            default:
                return GroupResult::invalidInput("searchGroup: invalid search mode");
        }
        if(result.empty())
        {
            return GroupResult::failure("searchGroup: no results");
        }
        return GroupResult::success("searchGroup: search success", std::move(result));
    }
    catch (const std::exception& e)
    {
        std::cerr << "[GroupService::searchGroup] " << e.what() << '\n';
        return GroupResult::error("searchGroup: internal error");
    }
}
