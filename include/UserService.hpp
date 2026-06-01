#pragma once
#include "IRepository.hpp"
#include "model.hpp"
#include <utility>
#include <vector>

struct UserQuery
{
    std::string token;
    std::string value;
    std::string name = "#";  // user_name identifier for update
};

struct UserResult
{
    enum class UserStatus
    {
        SUCCESS,
        FAILURE,
        ERROR,
        INVALID_INPUT
    };
    UserStatus status;
    std::string message;
    std::vector<User> data;

    static UserResult success(std::string message)
    {
        return
        {
            .status = UserStatus::SUCCESS,
            .message = std::move(message),
            .data = {}
        };
    }
    static UserResult success(std::string message, std::vector<User> data)
    {
        return { UserStatus::SUCCESS, std::move(message), std::move(data) };
    }
    static UserResult failure(std::string message)
    {
        return
        {
            .status = UserStatus::FAILURE,
            .message = std::move(message),
            .data = {}
        };
    }
    static UserResult invalidInput(std::string message)
    {
        return
        {
            .status = UserStatus::INVALID_INPUT,
            .message = std::move(message),
            .data = {}
        };
    }
    static UserResult error(std::string message)
    {
        return
        {
            .status = UserStatus::ERROR,
            .message = std::move(message),
            .data = {}
        };
    }
};

class UserService
{
private:
    IRepository<User>& user_repo;

public:
    explicit UserService(IRepository<User>& user_repo);

    UserResult addUser(const User& user);
    UserResult removeUser(const std::string& user_name);
    UserResult updateUser(const UserQuery& query);
    UserResult searchUser(const UserQuery& query);
    UserResult authenticateUser(const std::string& user_name,const std::string& password);
};
