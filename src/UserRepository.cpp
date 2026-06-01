#include"UserRepository.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "model.hpp"

UserRepository::UserRepository(SQLite::Database& db) : db(db)
{
    try
    {
        const std::string SQL = "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY AUTOINCREMENT,user_name TEXT UNIQUE NOT NULL,password TEXT NOT NULL);";
        db.exec(SQL);
    }
    catch (SQLite::Exception& e)
    {
        std::string err_msg = "DateBase users table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
    catch (const std::exception& e)
    {
        std::string err_msg = "DateBase users table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
}

bool UserRepository::add(const User& entity)
{
    try
    {
        const std::string Sql = "INSERT OR IGNORE INTO users (user_name,password) VALUES (?,?);";
        SQLite::Statement addUser(db,Sql);

        addUser.bind(1,entity.User_Name);
        addUser.bind(2,entity.Password);
        addUser.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add User Error in Database: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add User Error in std: " << e.what() << std::endl;
        return false;
    }
}

bool UserRepository::remove(const std::string& user_name)
{
    try
    {
        std::string Sql = "DELETE FROM users WHERE user_name = ?;";
        SQLite::Statement deleteUser(db,Sql);

        deleteUser.bind(1,user_name);
        deleteUser.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete User failed in Database: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete User failed in std: " << e.what() << std::endl;
        return false;
    }
}

bool UserRepository::update(const std::string& token,const std::string& value,const std::string& user_name)
{
    try
    {
        std::string Sql;

        auto it = UserMap.find(token);
        if(it == UserMap.end())
        {
            return false;
        }
        UserToken Token = it->second;

        switch(Token)
        {
            case UserToken::USER_NAME:
                Sql = "UPDATE users SET user_name = ? WHERE user_name = ?;";
                break;
            case UserToken::PASSWORD:
                Sql = "UPDATE users SET password = ? WHERE user_name = ?;";
                break;
            default:
                throw(SQLite::Exception("Update User failed , undefined Token"));
        }

        SQLite::Statement update(db,Sql);
        update.bind(1,value);
        update.bind(2,user_name);
        update.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Update User failed in Database: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Update User failed in std" << e.what() << std::endl;
        return false;
    }
}

std::vector<User> UserRepository::search(const std::string& token,const std::string& value)
{
    std::vector<User> Return;

    try
    {
        std::string Sql;

        auto it = UserMap.find(token);
        if(it == UserMap.end())
        {
            return Return;
        }
        UserToken Token = it->second;

        switch(Token)
        {
            case UserToken::USER_NAME:
                Sql = "SELECT id,user_name,password FROM users WHERE user_name = ?;";
                break;
            case UserToken::PASSWORD:
                Sql = "SELECT id,user_name,password FROM users WHERE password = ?;";
                break;
            default:
                throw std::runtime_error("Undefined Token in searchUser()");
        }

        SQLite::Statement findUser(db,Sql);
        findUser.bind(1,value);

        while(findUser.executeStep())
        {
            User temp;
            temp.Id = findUser.getColumn("id").getInt();
            temp.User_Name = findUser.getColumn("user_name").getText();
            temp.Password = findUser.getColumn("password").getText();

            Return.emplace_back(temp);
        }

        return Return;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in searchUser: " << e.what() << std::endl;
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in searchUser and is 'std' Error: " << e.what() << std::endl;
        return Return;
    }
}

bool UserRepository::exist(const std::string& user_name)
{
    try
    {
        std::string Sql = "SELECT COUNT(*) FROM users WHERE user_name = ?;";
        SQLite::Statement user_exist(db,Sql);
        user_exist.bind(1,user_name);
        user_exist.executeStep();

        if(user_exist.getColumn(0).getInt() > 0)
        {
            return true;
        }

        return false;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Exist function failed in Database: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exist function failed in std" << e.what() << std::endl;
        return false;
    }
}
