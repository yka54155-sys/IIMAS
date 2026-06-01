#include"StudentRepository.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include"SQLiteCpp/Transaction.h"
#include "model.hpp"
#include <exception>
#include <optional>  
#include <stdexcept>
#include <vector>
StudentRepository::StudentRepository(SQLite::Database& db) : db(db)
{
    try
    {
        const std::string SQL = "CREATE TABLE  IF NOT EXISTS students(id INTEGER PRIMARY KEY AUTOINCREMENT,student_id TEXT UNIQUE NOT NULL,name TEXT NOT NULL,gender TEXT, class INTEGER NOT NULL,subject TEXT NOT NULL,phone TEXT,remark TEXT ,create_time DATETIME DEFAULT CURRENT_TIMESTAMP);";
        db.exec(SQL);
    }
    catch (SQLite::Exception& e)
    {
        std::string err_msg = "DateBase students table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
    catch (const std::exception& e)
    {
        std::string err_msg = "DateBase students table Initialization failed: " + std::string(e.what());
        throw std::runtime_error(err_msg);
    }
}
bool StudentRepository::add(const Student& entity)
{
    try
    {
       

        //初始化添加sql语句
        const std::string Sql = "INSERT OR IGNORE INTO students (student_id,name,gender,class,subject,phone,remark) VALUES (?,?,?,?,?,?,?);";
        
        //绑定数据库与sql语句
        SQLite::Statement addStudent(db,Sql);

     
       
        addStudent.bind(1,entity.Student_Id);
        addStudent.bind(2,entity.Name);
        addStudent.bind(3,entity.Gender);
        addStudent.bind(4,entity.Class);
        addStudent.bind(5,entity.Subject);
        addStudent.bind(6,entity.Phone);
        addStudent.bind(7,entity.Remark);
        addStudent.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add Student Error in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add Student Error in std: " << e.what() << std::endl;
        
        return false;
    }
}

bool StudentRepository::remove(const std::string& id)
{
    try
    {
        std::string Sql = "DELETE FROM students WHERE student_id = ?;";
        
        SQLite::Statement deleteStudent(db,Sql);
        
        

        deleteStudent.bind(1,id);
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
bool StudentRepository::update(const std::string& token,const std::string& value,const std::string& id)
{
    try

    {

        std::string Sql;

        auto it = StudentMap.find(token);
        if(it == StudentMap.end())
        {
            return false;
        }
        StudentToken Token = it->second;

        switch(Token)

        {

            case StudentToken::STUDENT_ID:

                Sql = "UPDATE students SET student_id = ? WHERE student_id = ?;";

                break;

            case StudentToken::NAME:

                Sql = "UPDATE students SET name = ? WHERE student_id = ?;";

                break;

            case StudentToken::CLASS:

                Sql = "UPDATE students SET class = ? WHERE student_id = ?;";

                break;

            case StudentToken::SUBJECT:

                Sql = "UPDATE students SET subject = ? WHERE student_id = ?;";

                break;

            case StudentToken::PHONE:

                Sql = "UPDATE students SET phone = ? WHERE student_id = ?;";

                break;

            case StudentToken::REMARK:

                Sql = "UPDATE students SET remark = ? WHERE student_id = ?;";

                break;

            case StudentToken::GENDER:

                Sql = "UPDATE students SET gender = ? WHERE student_id = ?;";

                break;

            default:

                throw(SQLite::Exception("Update Student failed , undefined Token"));



        }



        SQLite::Statement update(db,Sql);

        if(Token == StudentToken::CLASS)

        {

            update.bind(1,std::stoi(value));    

        }

        else

        {

            update.bind(1,value);    

        }

        
        
        update.bind(2,id);

        update.exec();



        return true;

    }

    catch(const SQLite::Exception& e)

    {

        std::cerr << "Update Student failed in Database: " << e.what() << std::endl;

        

        return false;

    }

    catch (const std::exception& e)

    {

        std::cerr << "Update Student failed in std" << e.what() << std::endl;

        

        return false;

    }
}
std::vector<Student> StudentRepository::search(const std::string& token,const std::string& value)
{
     // 返回值列表
    std::vector<Student> Return;
    
    try
    {
        // SQL 语句
        std::string Sql;
        
        // 条件为整数时的复制载体
        int Value_Copy = 0;

        auto it = StudentMap.find(token);
        if(it == StudentMap.end())
        {
            return Return;
        }
        StudentToken Token = it->second;

        // 根据搜索条件初始化 SQL 语句，精确条件搜索
        switch(Token)
        {
            case StudentToken::STUDENT_ID:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE student_id = ?";
                break;
            case StudentToken::CLASS:
                Value_Copy = std::stoi(value);
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE class = ?";
                break;
            case StudentToken::CREATE_TIME:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE create_time = ?";
                break;
            case StudentToken::ID:
                Value_Copy = std::stoi(value);
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE id = ?";
                break;
            case StudentToken::NAME:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE name = ?";
                break;
            case StudentToken::PHONE:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE phone = ?";
                break;
            case StudentToken::REMARK:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE remark = ?";
                break;
            case StudentToken::SUBJECT:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE subject = ?";
                break;
            case StudentToken::GENDER:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE gender = ?";
                break;
            default:
                throw std::runtime_error("Undefined Token in exactSearchStudent()");
        }

        // 将 SQL 语句与具体数据库绑定
        SQLite::Statement findStudent(db,Sql);

        // SQL 语句的参数绑定
        if(Token == StudentToken::ID || Token == StudentToken::CLASS)
        {
            // 绑定整数参数
            findStudent.bind(1,Value_Copy);
        }
        else
        {
            // 绑定字符串参数
            findStudent.bind(1,value);
        }

        // 执行查询操作，可能有多条结果，所以用 while
        while(findStudent.executeStep())
        {
            // 行内容暂存变量
            Student temp;

            // 读取行内容，按列读取
            temp.Id = findStudent.getColumn("id").getInt();
            temp.Student_Id = findStudent.getColumn("student_id").getText();
            temp.Name = findStudent.getColumn("name").getText();
            temp.Class = findStudent.getColumn("class").getInt();
            temp.Subject = findStudent.getColumn("subject").getText();
            temp.Phone = findStudent.getColumn("phone").getText();
            temp.Remark = findStudent.getColumn("remark").getText();
            temp.Create_Time = findStudent.getColumn("create_time").getText();
            temp.Gender = findStudent.getColumn("gender").getText();

            // 将行内容放入最终返回变量中
            Return.emplace_back(temp);
        }

        return Return;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in exactSearchStudent(): " << e.what() << std::endl;
        
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in exactSearchStudent() and is 'std' Error: " << e.what() << std::endl;
        
        return Return;
    }
}
bool StudentRepository::exist(const std::string& id)
{
    try
    {
        std::string Sql = "SELECT COUNT(*) FROM students WHERE student_id = ?;";
        SQLite::Statement student_exist(db,Sql);
        student_exist.bind(1,id);
        student_exist.executeStep();
        if(student_exist.getColumn(0).getInt() > 0)
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

bool StudentRepository::addBatch(const std::vector<Student>& entities)
{
    try
    {
        //添加数据库事务
        SQLite::Transaction addTransaction(db);

        //初始化添加sql语句
        const std::string Sql = "INSERT OR IGNORE INTO students (student_id,name,gender,class,subject,phone,remark) VALUES (?,?,?,?,?,?,?);";
        
        //绑定数据库与sql语句
        SQLite::Statement addStudent(db,Sql);

     
       for(const auto& entity : entities)
       {
            addStudent.bind(1,entity.Student_Id);
            addStudent.bind(2,entity.Name);
            addStudent.bind(3,entity.Gender);
            addStudent.bind(4,entity.Class);
            addStudent.bind(5,entity.Subject);
            addStudent.bind(6,entity.Phone);
            addStudent.bind(7,entity.Remark);
            addStudent.exec();

            addStudent.reset();
       } 

        //提交事务，执行以上操作，有问题自动报错后回滚
        addTransaction.commit();
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add Student Error in Database: " << e.what() << std::endl;
        
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add Student Error in std: " << e.what() << std::endl;
        
        return false;
    }
}


std::vector<Student> StudentRepository::fuzzySearch(const std::string& token,const std::string& value)
{
    std::vector<Student> Return;//返回变量
    std::set<Student> Unique_Result;//用于中缀匹配时去重
    try
    {
        std::string Sql;//sql语句
        std::string Value_Copy = value;//复制条件值，便于模糊搜索
        auto it = StudentMap.find(token);
        if(it == StudentMap.end())
        {
            return Return;
        }
        StudentToken Token = it->second;
        
        //根据搜索条件初始化sql语句，模糊搜索
        switch(Token)
        {
            case StudentToken::STUDENT_ID:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE student_id LIKE ?";
                break;
            case StudentToken::CLASS:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE class LIKE ?";
                break;
            case StudentToken::CREATE_TIME:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE create_time LIKE ?";
                break;
            case StudentToken::ID:
                
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE id LIKE ?";
                break;
            case StudentToken::NAME:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE name LIKE ?";
                break;
            case StudentToken::PHONE:
                
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE phone LIKE ?";
                break;
            case StudentToken::REMARK:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE remark LIKE ?";
            break;
            case StudentToken::SUBJECT:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE subject LIKE ?";
                break;
            case StudentToken::GENDER:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE gender LIKE ?";
                break;
            default:
                throw std::runtime_error("Undefined Token in fuzzySearchStudent()");
        }

       
        //将数据库和sql语句绑定
        SQLite::Statement fuzzySearchStudent(db,Sql);

        //前缀匹配搜索。条件值参数化并与sql语句绑定参数
        Value_Copy = value + "%";
        fuzzySearchStudent.bind(1,Value_Copy);

        
        //执行查询操作
        while(fuzzySearchStudent.executeStep())
        {

            Student temp;//行内容暂存变量

            //读取每行内容，按列读取
            temp.Id = fuzzySearchStudent.getColumn("id").getInt();
            temp.Student_Id = fuzzySearchStudent.getColumn("student_id").getText();
            temp.Name = fuzzySearchStudent.getColumn("name").getText();
            temp.Class = fuzzySearchStudent.getColumn("class").getInt();
            temp.Subject = fuzzySearchStudent.getColumn("subject").getText();
            temp.Phone = fuzzySearchStudent.getColumn("phone").getText();
            temp.Remark = fuzzySearchStudent.getColumn("remark").getText();
            temp.Create_Time = fuzzySearchStudent.getColumn("create_time").getText();
            temp.Gender = fuzzySearchStudent.getColumn("gender").getText();

            //存入已读取内容
            Unique_Result.insert(temp);
        }

        //重置sql语句，便于进行其他搜索操作
        fuzzySearchStudent.reset();

        //中缀匹配搜索。条件值参数化并与sql语句绑定参数
        Value_Copy = "%" + value + "%";
        fuzzySearchStudent.bind(1,Value_Copy);
        while(fuzzySearchStudent.executeStep())
        {
            Student temp;
            temp.Id = fuzzySearchStudent.getColumn("id").getInt();
            temp.Student_Id = fuzzySearchStudent.getColumn("student_id").getText();
            temp.Name = fuzzySearchStudent.getColumn("name").getText();
            temp.Class = fuzzySearchStudent.getColumn("class").getInt();
            temp.Subject = fuzzySearchStudent.getColumn("subject").getText();
            temp.Phone = fuzzySearchStudent.getColumn("phone").getText();
            temp.Remark = fuzzySearchStudent.getColumn("remark").getText();
            temp.Create_Time = fuzzySearchStudent.getColumn("create_time").getText();
            temp.Gender = fuzzySearchStudent.getColumn("gender").getText();

            Unique_Result.insert(temp);
        }

        //将去重结果放入最终返回容器Return
        Return.assign(Unique_Result.begin(),Unique_Result.end());

        
        return Return;

    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in fuzzySearchStudent(): " << e.what() << std::endl;
       
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in fuzzySearchStudent() and is 'std' Error: " << e.what() << std::endl;
       
        return Return;
    }
}

std::optional<int> StudentRepository::getDatabaseID(const std::string& value)
{
    
    try
    {   
        std::string Sql = "SELECT id FROM students WHERE student_id = ?;";
        
        SQLite::Statement getID(db,Sql);
        getID.bind(1,value);
        if(getID.executeStep())
        {
            
            return getID.getColumn("id").getInt();
        }
        else
        {
            return std::nullopt;
        }
        
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in getDatabaseID(): " << e.what() << std::endl;
       
        return std::nullopt;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in getDatabaseID() and is 'std' Error: " << e.what() << std::endl;
       
        return std::nullopt;
    }
}