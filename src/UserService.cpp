#include "UserService.hpp"
#include "HashUtility.hpp"
#include "IRepository.hpp"
#include "model.hpp"
#include <iostream>

UserService::UserService(IRepository<User>& user_repo) : user_repo(user_repo)
{
    ;
}

UserResult UserService::addUser(const User& user)
{
    try
    {
        if (user.User_Name.empty() || user.Password.empty())
        {
            return UserResult::invalidInput("addUser: user_name or password is empty");
        }
        if (user_repo.exist(user.User_Name))
        {
            return UserResult::failure("addUser: user already exists");
        }

        User hashed = user;
        //使用用户姓名作为salt加密
        hashed.Password = hash_password(user.Password, user.User_Name);

        if (!user_repo.add(hashed))
        {
            return UserResult::error("addUser: database error, check UserRepository.add()");
        }
        return UserResult::success("addUser: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[UserService::addUser] " << e.what() << '\n';
        return UserResult::error("addUser: internal error");
    }
}

UserResult UserService::removeUser(const std::string& user_name)
{
    try
    {
        if (user_name.empty())
        {
            return UserResult::invalidInput("removeUser: user_name is empty");
        }
        if (!user_repo.exist(user_name))
        {
            return UserResult::failure("removeUser: user not found");
        }
        if (!user_repo.remove(user_name))
        {
            return UserResult::error("removeUser: database error, check UserRepository.remove()");
        }
        return UserResult::success("removeUser: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[UserService::removeUser] " << e.what() << '\n';
        return UserResult::error("removeUser: internal error");
    }
}

UserResult UserService::updateUser(const UserQuery& query)
{
    try
    {
        if (query.name.empty() || query.name == "#")
        {
            return UserResult::invalidInput("updateUser: user_name param is empty");
        }
        if (UserMap.find(query.token) == UserMap.end())
        {
            return UserResult::invalidInput("updateUser: unknown token");
        }
        if (!user_repo.exist(query.name))
        {
            return UserResult::failure("updateUser: user not found");
        }

        std::string update_value = query.value;
        if (query.token == "password")
        {
            update_value = hash_password(query.value, query.name);
        }

        if (!user_repo.update(query.token, update_value, query.name))
        {
            return UserResult::error("updateUser: database error, check UserRepository.update()");
        }
        return UserResult::success("updateUser: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[UserService::updateUser] " << e.what() << '\n';
        return UserResult::error("updateUser: internal error");
    }
}

UserResult UserService::searchUser(const UserQuery& query)
{
    try
    {
        if (query.token.empty() || query.value.empty())
        {
            return UserResult::invalidInput("searchUser: token or value is empty");
        }
        if (UserMap.find(query.token) == UserMap.end())
        {
            return UserResult::invalidInput("searchUser: unknown token");
        }

        std::vector<User> result = user_repo.search(query.token, query.value);
        if (result.empty())
        {
            return UserResult::failure("searchUser: no results");
        }
        // Strip password hashes before crossing layer boundary
        for (auto& user : result)
        {
            user.Password.clear();
        }
        return UserResult::success("searchUser: search success", std::move(result));
    }
    catch (const std::exception& e)
    {
        std::cerr << "[UserService::searchUser] " << e.what() << '\n';
        return UserResult::error("searchUser: internal error");
    }
}

UserResult UserService::authenticateUser(const std::string& user_name,const std::string& password)
{
    try
    {
        if(user_name.empty() || password.empty())
        {
            return UserResult::invalidInput("authenticateUser: user_name or password is empty");
        }
        std::vector<User> result = user_repo.search("user_name", user_name);
        if(result.empty())
        {
            return UserResult::failure("authenticateUser: user not found");
        }
        if(result.size() > 1)
        {
            return UserResult::error("authenticateUser: duplicate user_name in database");
        }

        if(hash_password(password, user_name) != result[0].Password)
        {
            return UserResult::failure("authenticateUser: incorrect password");
        }

        return UserResult::success("authenticateUser: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[UserService::authenticateUser] " << e.what() << '\n';
        return UserResult::error("authenticateUser: internal error");
    }
}