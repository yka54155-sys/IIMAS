#include"SAGRepository.hpp"
#include"SQLiteCpp/SQLiteCpp.h"
#include"SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include"SQLiteCpp/Transaction.h"
#include"SQLiteCpp/Database.h"
#include "model.hpp"
#include <string>

SAGRepository::SAGRepository(SQLite::Database& db) : db(db)
{
    try
    {
        const std::string SQL = "CREATE TABLE IF NOT EXISTS student_group(id INTEGER PRIMARY KEY AUTOINCREMENT,student_id INTEGER NOT NULL,group_id INTEGER NOT NULL,join_time DATETIME DEFAULT CURRENT_TIMESTAMP, remark TEXT,FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,UNIQUE(student_id,group_id));";
        db.exec(SQL);
    }
    catch (SQLite::Exception& e)
    {
        std::string err_msg = "DateBase SAG table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
    catch (const std::exception& e)
    {
        std::string err_msg = "DateBase SAG table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
}
bool SAGRepository::add(const SAG& entity) 
{
    try
    {      
        //初始化，添加学生--分组关联的sql语句
        const std::string SAG_Sql = "INSERT OR IGNORE INTO student_group (student_id,group_id) VALUES (?,?);";
        
        
        //添加关联的sql语句与数据库绑定
        SQLite::Statement addSAG(db,SAG_Sql);
        
        //参数绑定和执行语句
        
            

        //绑定添加SAG的sql语句参数
        addSAG.bind(1,entity.Student_Id);
        addSAG.bind(2,entity.Group_Id);

        //执行添加SAG语句
        addSAG.exec();


        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add SAG Error in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add SAG Error in std: " << e.what() << std::endl;
        
        return false;
    }
}
bool SAGRepository::remove(const std::string& id) 
{
    try
    {
        std::string Sql = "DELETE FROM student_group WHERE id = ? ;";
        SQLite::Statement deleteStudent(db,Sql);

        int ID = std::stoi(id);
        deleteStudent.bind(1,ID);

        deleteStudent.exec();        

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete Student failed in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete Student failed in std: " << e.what() << std::endl;
        
        return false;
    }
}
bool SAGRepository::update(const std::string& token,const std::string& value,const std::string& id) 
{
    try
    {
        std::string Sql;
        int value_int;
        
        auto it = SAGMap.find(token);
        if(it == SAGMap.end())
        {
            return false;
        }
        SAGToken Token = it->second;

        if(Token != SAGToken::REMARK)
        {
            value_int = std::stoi(value);
        }   
        switch(Token)
        {
            case SAGToken::STUDENT_ID:
                Sql  = "UPDATE student_group SET student_id = ? WHERE id = ?;";
                break;
            case SAGToken::GROUP_ID:
                Sql  = "UPDATE student_group SET group_id = ? WHERE id = ?;";
                break;
            case SAGToken::JOIN_TIME:
                Sql  = "UPDATE student_group SET join_time = ? WHERE id = ?;";
                break;
            case SAGToken::REMARK:
                Sql  = "UPDATE student_group SET remark = ? WHERE id = ?;";
                break;
            default:
            throw SQLite::Exception("Update SAG failed , undefined Token ");
        }

        SQLite::Statement updateSAG(db,Sql);
        if(Token == SAGToken::REMARK)
        {
            updateSAG.bind(1,value);
        }
        else
        {
            updateSAG.bind(1,value_int);
        }

        updateSAG.bind(2,id);

        updateSAG.exec();

        return true;

    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Update SAG failed in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Update SAG failed in std" << e.what() << std::endl;
        
        return false;
    }
}
std::vector<SAG> SAGRepository::search(const std::string& token,const std::string& value) 
{
     //返回值
    std::vector<SAG> Return;

    try
    { 
        std::string Sql;//sql语句
        std::string Value = "%" + value + "%";

        auto it = SAGMap.find(token);
        if(it == SAGMap.end())
        {
            return Return;
        }
        SAGToken Token = it->second;

        //根据搜索条件初始化sql语句，模糊条件搜索
        switch(Token)
        {   
            case SAGToken::GROUP_ID:
                Sql = "SELECT id,student_id,group_id,join_time,remark FROM student_group WHERE group_id = ?;";
                break;
            case SAGToken::STUDENT_ID:
                Sql = "SELECT id,student_id,group_id,join_time,remark FROM student_group WHERE student_id = ?;";
                break;
            case SAGToken::ID:
                Sql = "SELECT id,student_id,group_id,join_time,remark FROM student_group WHERE id = ?;";
                break;
            case SAGToken::JOIN_TIME:
                Sql = "SELECT id,student_id,group_id,join_time,remark FROM student_group WHERE join_time = ?;";
                break;
            case SAGToken::REMARK:
                Sql = "SELECT id,student_id,group_id,join_time,remark FROM student_group WHERE remark LIKE ?;";
                break;
            default:
                throw std::runtime_error("Undefined Token in searchSAG()");
        }

        //将sql语句与具体数据库绑定
        SQLite::Statement searchSAG(db,Sql);
        //sql语句的参数绑定
        if(Token == SAGToken::REMARK)
        {
            searchSAG.bind(1,Value);    
        }
        else
        {
            searchSAG.bind(1,value);    
        }
        

        //执行查询操作，可能有多条结果，所以用while
        while(searchSAG.executeStep())
        {
            SAG temp;//行内容暂存变量

            //读取每行内容，按列读取
            temp.Id = searchSAG.getColumn("id").getInt();
            temp.Student_Id = searchSAG.getColumn("student_id").getInt();
            temp.Group_Id = searchSAG.getColumn("group_id").getInt();
            temp.Join_Time = searchSAG.getColumn("join_time").getText();
            temp.Remark = searchSAG.getColumn("remark").getText();
            
            //存入已读取内容
            Return.emplace_back(temp);
        }

        
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in searchSAG(): " << e.what() << std::endl;
       
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in searchSAG() and is 'std' Error: " << e.what() << std::endl;
       
    }
    




    return Return;
}
bool SAGRepository::exist(const std::string& stu_id , const std::string& grp_id) 
{
    try
    {
        std::string sql = "SELECT COUNT(*) FROM student_group WHERE student_id = ? AND group_id = ?;";
        SQLite::Statement SAG_exist(db,sql);
        int ID1 = std::stoi(stu_id);
        int ID2 = std::stoi(grp_id);
        SAG_exist.bind(1,ID1);
        SAG_exist.bind(2,ID2);
        SAG_exist.executeStep();
        if(SAG_exist.getColumn(0).getInt() > 0)
        {
            return true;
        }

        return false;

    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Exist(double param version) function failed in Database: " << e.what() << std::endl;

        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exist(double param version) function failed in std" << e.what() << std::endl;
        
        return false;
    }
}
bool SAGRepository::exist(const std::string& id) 
{
    try
    {
        std::string sql = "SELECT COUNT(*) FROM student_group WHERE id = ?;";
        SQLite::Statement SAG_exist(db,sql);
        int ID = std::stoi(id);
        SAG_exist.bind(1,ID);
        
        SAG_exist.executeStep();
        if(SAG_exist.getColumn(0).getInt() > 0)
        {
            return true;
        }

        return false;

    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Exist(one param version) function failed in Database: " << e.what() << std::endl;

        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exist(one param version) function failed in std" << e.what() << std::endl;
        
        return false;
    }
}