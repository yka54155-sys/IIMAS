#include"GroupRepository.hpp"
#include "SQLiteCpp/Database.h"
#include"SQLiteCpp/Transaction.h"
#include "model.hpp"
#include <vector>

GroupRepository::GroupRepository(SQLite::Database& db) : db(db)
{
    try
    {
        const std::string SQL = "CREATE TABLE IF NOT EXISTS groups(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT NOT NULL UNIQUE,description TEXT,create_time DATETIME DEFAULT CURRENT_TIMESTAMP);";
        db.exec(SQL);
    }
    catch (SQLite::Exception& e)
    {
        std::string err_msg = "DateBase groups table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
    catch (const std::exception& e)
    {
        std::string err_msg = "DateBase groups table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
}

bool GroupRepository::add(const Group& entity) 
{
    try
    {
        std::string Sql = "INSERT INTO groups (name,description) VALUES (?,?);";
        
        SQLite::Statement addGroup(db,Sql);
       
        
        addGroup.bind(1,entity.Group_Name);
        addGroup.bind(2,entity.Description);

        addGroup.exec();
        

            
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add Group failed in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add Group failed in std: " << e.what() << std::endl;
        
        return false;
    }
}
bool GroupRepository::remove(const std::string& name) 
{
    try
    {
        std::string Sql = "DELETE FROM groups WHERE name = ?;";
        SQLite::Statement deleteGroup(db,Sql);
        
        deleteGroup.bind(1,name);

        deleteGroup.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete Group failed in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete Group failed in std: " << e.what() << std::endl;
        
        return false;
    }
}
bool GroupRepository::update(const std::string& token,const std::string& value,const std::string& name) 
{
    try
    {
        std::string Sql;
        auto it  = GroupMap.find(token);
        if(it == GroupMap.end())
        {
            return false;
        }
        GroupToken Token = it->second;
        switch(Token)
        {
            case GroupToken::GROUP_NAME:
                Sql = "UPDATE groups SET name = ? WHERE name = ?;";
                break;
            case GroupToken::DESCRIPTION:
            Sql = "UPDATE groups SET description = ? WHERE name = ?;";
                break;
            case GroupToken::CREATE_TIME:
             Sql = "UPDATE groups SET create_time = ? WHERE name = ?;";
                break;
            default:
                throw(SQLite::Exception("Update Group failed , undefined Token"));
        }

        SQLite::Statement update(db,Sql);
         update.bind(1,value);
        
        update.bind(2,name);
        update.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Update Group failed in Database: " << e.what() << std::endl;
       
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Update Group failed in std" << e.what() << std::endl;
       
        return false;
    }
}
std::vector<Group> GroupRepository::search(const std::string& token,const std::string& value) 
{
     //返回值
    std::vector<Group> Return;
    try
    {
        
        std::string Sql;//sql语句

        
        int Value_Copy = 0;//条件为整数时的复制载体

        auto it = GroupMap.find(token);
        if(it == GroupMap.end())
        {
            return Return;
        }
        GroupToken Token = it->second;

        //根据搜索条件初始化sql语句，精确条件搜索
        switch(Token)
        {
            case GroupToken::GROUP_NAME:
                Sql = "SELECT id,name,description,create_time FROM groups WHERE name = ?;";
                break;
            case GroupToken::CREATE_TIME:
                
                Sql = "SELECT id,name,description,create_time FROM groups WHERE create_time = ?;";
                break;
            case GroupToken::DESCRIPTION:
                Sql = "SELECT id,name,description,create_time FROM groups WHERE description = ?;";
                break;
            case GroupToken::ID:
                
                Sql = "SELECT id,name,description,create_time FROM groups WHERE id = ?;";
                break;

            default:
                throw std::runtime_error("Undefined Token in exactSearchGroup()");
        }

        //将sql语句与具体数据库绑定
        SQLite::Statement findGroup(db,Sql);

        //sql语句的参数绑定
         findGroup.bind(1,value);

        //执行查询操作，可能有多条结果，所以用while
        while(findGroup.executeStep())
        {

            Group temp;//行内容暂存变量

            //读取行内容，按列读取
            temp.Id = findGroup.getColumn(0).getInt();
            temp.Group_Name = findGroup.getColumn(1).getText();
            temp.Description = findGroup.getColumn(2).getText();
            temp.Create_Time = findGroup.getColumn(3).getText();

            //将行内容放入最终返回变量中
            Return.emplace_back(temp);
        }

        return Return;

    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in exactSearchGroup(): " << e.what() << std::endl;
        
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in exactSearchGroup() and is 'std' Error: " << e.what() << std::endl;
        
        return Return;
    }
}
std::vector<Group> GroupRepository::fuzzySearch(const std::string& token,const std::string& value)
{
     //返回值
    std::vector<Group> Return;
    std::set<Group> Unique_Result;
    try
    {
        
        std::string Sql;//sql语句
        std::string Value_Copy = value;//复制条件值，便于模糊搜索
         auto it = GroupMap.find(token);
        if(it == GroupMap.end())
        {
            return Return;
        }
        GroupToken Token = it->second;

        //根据搜索条件初始化sql语句，模糊条件搜索
        switch(Token)
        {
            case GroupToken::GROUP_NAME:
                Sql = "SELECT id,name,description,create_time FROM groups WHERE name LIKE ?;";
                break;
            case GroupToken::CREATE_TIME:
                
                Sql = "SELECT id,name,description,create_time FROM groups WHERE create_time LIKE ?;";
                break;
            case GroupToken::DESCRIPTION:
                Sql = "SELECT id,name,description,create_time FROM groups WHERE description LIKE ?;";
                break;
            case GroupToken::ID:
                
                Sql = "SELECT id,name,description,create_time FROM groups WHERE id LIKE ?;";
                break;

            default:
                throw std::runtime_error("Undefined Token in fuzzySearchGroup()");
        }

        //将sql语句与具体数据库绑定
        SQLite::Statement fuzzySearchGroup(db,Sql);

        //前缀匹配搜索。条件值参数化并与sql语句绑定参数
        Value_Copy = value + "%";
        fuzzySearchGroup.bind(1,Value_Copy);

        
        //执行查询操作
        while(fuzzySearchGroup.executeStep())
        {

            Group temp;//行内容暂存变量

            //读取每行内容，按列读取
            temp.Id = fuzzySearchGroup.getColumn(0).getInt();
            temp.Group_Name = fuzzySearchGroup.getColumn(1).getText();
            temp.Description = fuzzySearchGroup.getColumn(2).getText();
            temp.Create_Time = fuzzySearchGroup.getColumn(3).getText();

            //存入已读取内容
            Unique_Result.insert(temp);
        }

        //重置sql语句，便于进行其他搜索操作
        fuzzySearchGroup.reset();

        //中缀匹配搜索。条件值参数化并与sql语句绑定参数
        Value_Copy = "%" + value + "%";
        fuzzySearchGroup.bind(1,Value_Copy);

        //执行查询操作，可能有多条结果，所以用while
        while(fuzzySearchGroup.executeStep())
        {
            Group temp;//行内容暂存变量

             //读取每行内容，按列读取
            temp.Id = fuzzySearchGroup.getColumn(0).getInt();
            temp.Group_Name = fuzzySearchGroup.getColumn(1).getText();
            temp.Description = fuzzySearchGroup.getColumn(2).getText();
            temp.Create_Time = fuzzySearchGroup.getColumn(3).getText();

            //存入已读取内容
            Unique_Result.insert(temp);
        }

        //将去重结果放入最终返回容器Return
        Return.assign(Unique_Result.begin(),Unique_Result.end());

        
        return Return;

    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in fuzzySearchGroup(): " << e.what() << std::endl;
        
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in fuzzySearchGroup() and is 'std' Error: " << e.what() << std::endl;
        
        return Return;
    }
}
bool GroupRepository::exist(const std::string& name) 
{
    try
    {
        std::string Sql = "SELECT COUNT(*) FROM groups WHERE name = ?;";
        SQLite::Statement group_exist(db,Sql);
        group_exist.bind(1,name);
        group_exist.executeStep();
        if(group_exist.getColumn(0).getInt() > 0)
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

std::optional<int> GroupRepository::getDatabaseID(const std::string& value)
{
    try
    {
        std::string Sql = "SELECT id FROM groups WHERE name = ?;";
        SQLite::Statement getID(db, Sql);
        getID.bind(1, value);
        if (getID.executeStep())
        {
            return getID.getColumn("id").getInt();
        }
        return std::nullopt;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "[GroupRepository::getDatabaseID] " << e.what() << '\n';
        return std::nullopt;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[GroupRepository::getDatabaseID] " << e.what() << '\n';
        return std::nullopt;
    }
}