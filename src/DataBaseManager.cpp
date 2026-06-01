#include "DataBaseManager.hpp"
#include"SQLiteCpp/Database.h"
#include"SQLiteCpp/Exception.h"
#include"SQLiteCpp/Statement.h"
#include"SQLiteCpp/Transaction.h"
#include"SQLiteCpp/Column.h"
#include"crow.h"
#include "std.hpp"
#include"model.hpp"
#include"HashUtility.hpp"
#include <algorithm>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


/**
 * @brief 三种表结构：
 students（表id，学号，姓名，性别，班级，专业，电话，备注,创建时间）
 groups （表id，组名，组简介，组创建时间）
 student_group （表id，students表id，groups表id，加入时间）
 * 
 */
const std::string Students_Table = " CREATE TABLE  IF NOT EXISTS students(id INTEGER PRIMARY KEY AUTOINCREMENT,student_id TEXT UNIQUE NOT NULL,name TEXT NOT NULL,gender TEXT, class INTEGER NOT NULL,subject TEXT NOT NULL,phone TEXT,remark TEXT ,create_time DATETIME DEFAULT CURRENT_TIMESTAMP);";
const std::string Groups_Table = "CREATE TABLE IF NOT EXISTS groups(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT NOT NULL UNIQUE,description TEXT,create_time DATETIME DEFAULT CURRENT_TIMESTAMP);";
const std::string Student_Group_Table = "CREATE TABLE IF NOT EXISTS student_group(id INTEGER PRIMARY KEY AUTOINCREMENT,student_id INTEGER NOT NULL,group_id INTEGER NOT NULL,join_time DATETIME DEFAULT CURRENT_TIMESTAMP, remark TEXT,FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,UNIQUE(student_id,group_id));";
const std::string User_Table = "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY AUTOINCREMENT,user_name TEXT UNIQUE NOT NULL,password TEXT NOT NULL);";


/**
 * @brief 数据库管理器构造函数
 * 
 * 打开或创建数据库文件，初始化表结构和索引
 */
DataBaseManager::DataBaseManager() : Db(nullptr)
{
    try
    {
        // 打开数据库 IIMAS.db，如果没有就创建
        Db = std::make_unique<SQLite::Database>("IIMAS.db",SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        
        // 启用外键约束，确保级联删除正常工作
        Db->exec("PRAGMA foreign_keys = ON;");

        // 检查表是否存在，如果都不存在则创建
        if(!Db->tableExists("students") && !Db->tableExists("groups") && !Db->tableExists("student_group"))
        {
            // 创建三种表：学生表、分组表和学生-分组关联表
            Db->exec(Students_Table);
            Db->exec(Groups_Table);
            Db->exec(Student_Group_Table);
            

            // 创建索引以提高查询性能
            Db->exec("CREATE INDEX idx_student_id ON students(student_id);");
            Db->exec("CREATE INDEX idx_name ON students(name);");
            Db->exec("CREATE INDEX idx_class ON students(class);");
            Db->exec("CREATE INDEX idx_subject ON students(subject);");
            // Db->exec("CREATE INDEX idx_phone ON students(phone);");
            // Db->exec("CREATE INDEX idx_remark ON students(remark);");
            // Db->exec("CREATE INDEX idx_create_time ON students(create_time);");
            Db->exec("CREATE INDEX idx_group_name ON groups(name);");
            Db->exec("CREATE INDEX idx_group_description ON groups(description);");
            // Db->exec("CREATE INDEX idx_group_create_time ON group(create_time);");
        }
        Db->exec(User_Table);

        SQLite::Statement query(*Db,"SELECT EXISTS(SELECT 1 FROM users);");
        query.executeStep();
        bool has_data = query.getColumn(0).getInt() == 1;

        if(!has_data)
        {
            Db->exec("INSERT OR IGNORE INTO users(user_name,password) VALUES('admin','" + hash_password("123456", "iimas") + "');");
        }
    }
    catch (const SQLite::Exception& e)
    {
        // 输出错误信息
        std::cerr << "DataBase Initialization failed: " <<  e.what() << std::endl;
        std::cerr << "DataBase Initialization failed: " <<  e.getErrorStr() << std::endl;
        // 记录日志
        CROW_LOG_ERROR <<"DataBase Initialization failed: " <<  e.what() ;
        // 抛出异常
        throw std::runtime_error("DataBase Initialization failed");
    }
}

/**
 * @brief 按精确条件搜索学生
 * @param Value 搜索值，班级、姓名、学号等，参考 Student 结构体内属性
 * @param Token 搜索条件，可以按照班级、姓名、学号等条件搜索
 * @return 符合搜索条件的学生数组
 * 
 * 精确搜索学生，使用 SQL = 操作符，不进行模糊匹配
 */
std::vector<Student> DataBaseManager::exactSearchStudent(const std::string& Value,const StudentToken Token)
{
    // 返回值列表
    std::vector<Student> Return;
    
    try
    {
        // SQL 语句
        std::string Sql;
        
        // 条件为整数时的复制载体
        int Value_Copy = 0;

        // 根据搜索条件初始化 SQL 语句，精确条件搜索
        switch(Token)
        {
            case StudentToken::STUDENT_ID:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE student_id = ?";
                break;
            case StudentToken::CLASS:
                Value_Copy = std::stoi(Value);
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE class = ?";
                break;
            case StudentToken::CREATE_TIME:
                Sql = "SELECT id , student_id,name,gender,class,subject,phone,remark,create_time FROM students WHERE create_time = ?";
                break;
            case StudentToken::ID:
                Value_Copy = std::stoi(Value);
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
        SQLite::Statement findStudent(*Db,Sql);

        // SQL 语句的参数绑定
        if(Token == StudentToken::ID || Token == StudentToken::CLASS)
        {
            // 绑定整数参数
            findStudent.bind(1,Value_Copy);
        }
        else
        {
            // 绑定字符串参数
            findStudent.bind(1,Value);
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
        CROW_LOG_ERROR << "DateBase Search failed in exactSearchStudent(): " << e.what();
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in exactSearchStudent() and is 'std' Error: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in exactSearchStudent() and is 'std' Error: " << e.what();
        return Return;
    }
}


/**
 * @brief 按模糊条件搜索学生,前缀和中缀匹配,包含异常处理
 * 
 * @param Value 搜索值，班级、姓名、学号、等，参考Student结构体内属性,int/string
 * @param Token 搜索条件，可以按照班级、姓名、学号等条件搜索，来自enum class StudentToken
 * @return std::vector<Student> 符合搜索条件的学生的数组
 */
std::vector<Student> DataBaseManager::fuzzySearchStudent(const std::string& Value,StudentToken Token)
{

    std::vector<Student> Return;//返回变量
    std::set<Student> Unique_Result;//用于中缀匹配时去重
    try
    {
        std::string Sql;//sql语句
        std::string Value_Copy = Value;//复制条件值，便于模糊搜索

        
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
        SQLite::Statement fuzzySearchStudent(*Db,Sql);

        //前缀匹配搜索。条件值参数化并与sql语句绑定参数
        Value_Copy = Value + "%";
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
        Value_Copy = "%" + Value + "%";
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
        CROW_LOG_ERROR << "DateBase Search failed in fuzzySearchStudent(): " << e.what();
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in fuzzySearchStudent() and is 'std' Error: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in fuzzySearchStudent() and is 'std' Error: " << e.what();
        return Return;
    }
    
}


/**
 * @brief 按精确条件搜索分组,不能模糊搜索,包含异常处理
 * 
 * @param Value 搜索值，组id，分组名，简介，创建时间，string格式
 * @param Token 搜索条件，可以按照组id，分组名，简介，创建时间，参考GroupToken
 * @return std::vector<Group> 符合搜索条件的分组的数组
 */
std::vector<Group> DataBaseManager::exactSearchGroup(const std::string& Value,GroupToken Token)
{
     //返回值
    std::vector<Group> Return;
    try
    {
        
        std::string Sql;//sql语句

        
        int Value_Copy = 0;//条件为整数时的复制载体

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
        SQLite::Statement findGroup(*Db,Sql);

        //sql语句的参数绑定
         findGroup.bind(1,Value);

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
        CROW_LOG_ERROR << "DateBase Search failed in exactSearchGroup(): " << e.what();
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in exactSearchGroup() and is 'std' Error: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in exactSearchGroup() and is 'std' Error: " << e.what();
        return Return;
    }
}


/**
 * @brief 按模糊条件搜索分组,包含前缀和中缀匹配，包含异常处理
 * 
 * @param Value 搜索值，组id，分组名，简介，创建时间，string格式
 * @param Token 搜索条件，可以按照组id，分组名，简介，创建时间，参考GroupToken
 * @return std::vector<Group> 符合搜索条件的分组的数组
 */
std::vector<Group> DataBaseManager::fuzzySearchGroup(const std::string& Value,GroupToken Token)
{
     //返回值
    std::vector<Group> Return;
    std::set<Group> Unique_Result;
    try
    {
        
        std::string Sql;//sql语句
        std::string Value_Copy = Value;//复制条件值，便于模糊搜索
        

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
        SQLite::Statement fuzzySearchGroup(*Db,Sql);

        //前缀匹配搜索。条件值参数化并与sql语句绑定参数
        Value_Copy = Value + "%";
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
        Value_Copy = "%" + Value + "%";
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
        CROW_LOG_ERROR << "DateBase Search failed in fuzzySearchGroup(): " << e.what();
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in fuzzySearchGroup() and is 'std' Error: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in fuzzySearchGroup() and is 'std' Error: " << e.what();
        return Return;
    }
    
    
}

/**
 * @brief 搜索学生--分组的联系表内容，包含异常处理
 * 
 * @param Id 查询时的参数，可以是学生id或分组id
 * @param Token 查询方式，是要查分组中的学生，还是学生在哪些分组，参考SAGToekn
 * @return std::vector<SAG> 返回值，包含了符合查询条件的学生--分组关系数组
 */
std::vector<SAG> DataBaseManager::searchSAG(const std::string& Id,SAGToken Token)
{
    //返回值
    std::vector<SAG> Return;

    try
    { 
        std::string Sql;//sql语句
        std::string Value = "%" + Id + "%";

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
        SQLite::Statement searchSAG(*Db,Sql);
        //sql语句的参数绑定
        if(Token == SAGToken::REMARK)
        {
            searchSAG.bind(1,Value);    
        }
        else
        {
            searchSAG.bind(1,Id);    
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
        CROW_LOG_ERROR << "DateBase Search failed in searchSAG(): " << e.what();
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in searchSAG() and is 'std' Error: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in searchSAG() and is 'std' Error: " << e.what();
    }
    




    return Return;
}


/**
 * @brief 在students表添加学生，表id和创建时间不需要，包含异常处理
 * 
 * @param stus 学生信息，用Student结构体封装
 * @return true 添加成功
 * @return false 添加失败
 */
bool DataBaseManager::addStudent(const std::vector<Student>& stus)
{
    try
    {
        //添加数据库事务
        SQLite::Transaction addTransaction(*Db);

        //初始化添加sql语句
        const std::string Sql = "INSERT OR IGNORE INTO students (student_id,name,gender,class,subject,phone,remark) VALUES (?,?,?,?,?,?,?);";
        
        //绑定数据库与sql语句
        SQLite::Statement addStudent(*Db,Sql);

        //绑定sql语句参数
        for(const auto& stu : stus)
        {
            addStudent.bind(1,stu.Student_Id);
            addStudent.bind(2,stu.Name);
            addStudent.bind(3,stu.Gender);
            addStudent.bind(4,stu.Class);
            addStudent.bind(5,stu.Subject);
            addStudent.bind(6,stu.Phone);
            addStudent.bind(7,stu.Remark);
            addStudent.exec();

            //重置参数绑定，方便读取下一个输入的学生信息
            addStudent.reset();
        }

        //提交事务，执行以上操作，有问题自动报错后回滚
        addTransaction.commit();
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add Student Error in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add Student Error in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add Student Error in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add Student Error in std: " << e.what();
        return false;
    }
}


/**
 * @brief 将学生添加到指定分组中，不需要表id和创建时间，包含异常处理
 * 
 * @param stus 学生信息，封装在Student结构体中
 * @param group_id 要将这些学生添加到的分组的表id
 * @return true 添加成功
 * @return false 添加失败
 */
bool DataBaseManager::addStudent(const std::vector<Student>& stus,const int& group_id)
{
    try
    {
        
        //创建事务
        SQLite::Transaction addTransaction(*Db);
        
        
        //初始化，添加学生--分组关联的sql语句
        const std::string SAG_Sql = "INSERT OR IGNORE INTO student_group (student_id,group_id) VALUES (?,?);";
        
        
        //添加关联的sql语句与数据库绑定
        SQLite::Statement addSAG(*Db,SAG_Sql);
        
        //参数绑定和执行语句
        for(const auto& stu : stus)
        {
            

            //绑定添加SAG的sql语句参数
            addSAG.bind(1,stu.Id);
            addSAG.bind(2,group_id);

            //执行添加SAG语句
            addSAG.exec();

            //重置两个语句的参数绑定，直到添加所有stus内容
            addSAG.reset();
        }
        
        //提交事务，执行以上操作，有问题自动报错后回滚
        addTransaction.commit();
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add Student Error in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add Student Error in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add Student Error in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add Student Error in std: " << e.what();
        return false;
    }
}

/**
 * @brief 添加分组
 * 
 * @param grps 分组信息
 * @return true 添加成功
 * @return false 添加失败
 */
bool DataBaseManager::addGroup(const std::vector<Group>& grps)
{
    try
    {
        std::string Sql = "INSERT INTO groups (name,description) VALUES (?,?);";
        SQLite::Transaction addTransaction(*Db);
        SQLite::Statement addGroup(*Db,Sql);
        for(const auto& grp : grps)
        {
            addGroup.bind(1,grp.Group_Name);
            addGroup.bind(2,grp.Description);

            addGroup.exec();
            addGroup.reset();
        }

        addTransaction.commit();

            
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add Group failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add Group failed in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add Group failed in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add Group failed in std: " << e.what();
        return false;
    }
}

/**
 * @brief 删除学生，从students表
 * 
 * @param stus 想要删除的学生信息，可批量删除
 * @return true 删除成功
 * @return false 
 */
bool DataBaseManager::deleteStudent(const std::vector<Student>& stus)
{
    try
    {
        std::string Sql = "DELETE FROM students WHERE id = ?;";
        SQLite::Transaction deleteTransaction(*Db);
        SQLite::Statement deleteStudent(*Db,Sql);
        for(const auto& stu : stus)
        {
            deleteStudent.bind(1,stu.Id);
            deleteStudent.exec();
            deleteStudent.reset();
        }

        deleteTransaction.commit();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete Student failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete Student failed in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete Student failed in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete Student failed in std: " << e.what();
        return false;
    }

}

/**
 * @brief 删除学生分组的关联信息，也就是在某个组删除学生与本组的关联
 * 
 * @param stus 要删除的学生信息
 * @param id 要在哪个组删除
 * @return true 删除成功
 * @return false 
 */
bool DataBaseManager::deleteStudent(const std::vector<Student>& stus,const int& id)
{
    try
    {
        std::string Sql = "DELETE FROM student_group WHERE student_id = ? AND group_id = ?;";
        SQLite::Transaction deleteTransaction(*Db);
        SQLite::Statement deleteStudent(*Db,Sql);

        for(const auto& stu : stus)
        {
            deleteStudent.bind(1,stu.Id);
            deleteStudent.bind(2,id);

            deleteStudent.exec();

            deleteStudent.reset();
        }

        deleteTransaction.commit();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete Student failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete Student failed in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete Student failed in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete Student failed in std: " << e.what();
        return false;
    }
}

/**
 * @brief 删除分组，在groups表中
 * 
 * @param grps 要删除的分组信息
 * @return true 删除成功
 * @return false 
 */
bool DataBaseManager::deleteGroup(const std::vector<Group>& grps)
{
    try
    {
        std::string Sql = "DELETE FROM groups WHERE id = ?;";
        SQLite::Transaction deleteTransaction(*Db);
        SQLite::Statement deleteGroup(*Db,Sql);
        for(const auto& grp : grps)
        {
            deleteGroup.bind(1,grp.Id);

            deleteGroup.exec();

            deleteGroup.reset();
        }

        deleteTransaction.commit();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete Group failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete Group failed in Database:  " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete Group failed in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete Group failed in std: " << e.what();
        return false;
    }
}

/**

 * @brief 更新单个学生的指定字段信息

 * 

 * @param Value 更新后的值，字符串格式

 * @param Id 学生表id，用于定位要更新的学生

 * @param Token 更新字段，参考StudentToken枚举（STUDENT_ID、NAME、CLASS、SUBJECT、PHONE、REMARK、GENDER）

 * @return true 更新成功

 * @return false 更新失败

 */

bool DataBaseManager::updateStudent(const std::string& Value,const int& Id,StudentToken Token)

{

    try

    {

        std::string Sql;

        switch(Token)

        {

            case StudentToken::STUDENT_ID:

                Sql = "UPDATE students SET student_id = ? WHERE id = ?;";

                break;

            case StudentToken::NAME:

                Sql = "UPDATE students SET name = ? WHERE id = ?;";

                break;

            case StudentToken::CLASS:

                Sql = "UPDATE students SET class = ? WHERE id = ?;";

                break;

            case StudentToken::SUBJECT:

                Sql = "UPDATE students SET subject = ? WHERE id = ?;";

                break;

            case StudentToken::PHONE:

                Sql = "UPDATE students SET phone = ? WHERE id = ?;";

                break;

            case StudentToken::REMARK:

                Sql = "UPDATE students SET remark = ? WHERE id = ?;";

                break;

            case StudentToken::GENDER:

                Sql = "UPDATE students SET gender = ? WHERE id = ?;";

                break;

            default:

                throw(SQLite::Exception("Update Student failed , undefined Token"));



        }



        SQLite::Statement update(*Db,Sql);

        if(Token == StudentToken::CLASS)

        {

            update.bind(1,std::stoi(Value));    

        }

        else

        {

            update.bind(1,Value);    

        }

        

        update.bind(2,Id);

        update.exec();



        return true;

    }

    catch(const SQLite::Exception& e)

    {

        std::cerr << "Update Student failed in Database: " << e.what() << std::endl;

        CROW_LOG_ERROR << "Update Student failed in Database:  " << e.what();

        return false;

    }

    catch (const std::exception& e)

    {

        std::cerr << "Update Student failed in std" << e.what() << std::endl;

        CROW_LOG_ERROR << "Update Student failed in std" << e.what();

        return false;

    }



};


/**
 * @brief 批量更新多个学生的指定字段信息
 * 
 * @param stus 学生表id列表，用于定位要更新的学生
 * @param Value 更新后的值，字符串格式
 * @param Token 更新字段，参考StudentToken枚举（STUDENT_ID、NAME、CLASS、SUBJECT、PHONE、REMARK、GENDER）
 * @return true 更新成功
 * @return false 更新失败
 */
bool DataBaseManager::updateStudent(const std::vector<int>& stus,const std::string& Value,StudentToken Token)
{
    try
    {
        SQLite::Transaction updateTransaction(*Db);
        std::string Sql;
        switch(Token)
        {
            case StudentToken::STUDENT_ID:
                Sql = "UPDATE students SET student_id = ? WHERE id = ?;";
                break;
            case StudentToken::NAME:
                Sql = "UPDATE students SET name = ? WHERE id = ?;";
                break;
            case StudentToken::CLASS:
                Sql = "UPDATE students SET class = ? WHERE id = ?;";
                break;
            case StudentToken::SUBJECT:
                Sql = "UPDATE students SET subject = ? WHERE id = ?;";
                break;
            case StudentToken::PHONE:
                Sql = "UPDATE students SET phone = ? WHERE id = ?;";
                break;
            case StudentToken::REMARK:
                Sql = "UPDATE students SET remark = ? WHERE id = ?;";
                break;
            case StudentToken::GENDER:
                Sql = "UPDATE students SET gender = ? WHERE id = ?;";
                break;
            default:
                throw(SQLite::Exception("Update Student failed , undefined Token"));
        }

        SQLite::Statement update(*Db,Sql);
        for(const auto& stu : stus)
        {
            if(Token == StudentToken::CLASS)
            {
                update.bind(1,std::stoi(Value));    
            }
            else
            {
                update.bind(1,Value);    
            }

            update.bind(2,stu);
            update.exec();
            update.reset();
            
        }

        updateTransaction.commit();
        
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Update Student failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Update Student failed in Database:  " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Update Student failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Update Student failed in std" << e.what();
        return false;
    }
}

/**
 * @brief 更新分组信息，支持按不同字段更新
 * 
 * @param Value 更新后的值
 * @param id 分组表id
 * @param Token 更新字段，参考GroupToken枚举（ID、GROUP_NAME、DESCRIPTION、CREATE_TIME）
 * @return true 更新成功
 * @return false 更新失败
 */
bool DataBaseManager::updateGroup(const std::string& Value,const int& id,GroupToken Token)
{
    try
    {
        std::string Sql;
        switch(Token)
        {
            case GroupToken::GROUP_NAME:
                Sql = "UPDATE groups SET name = ? WHERE id = ?;";
                break;
            case GroupToken::DESCRIPTION:
            Sql = "UPDATE groups SET description = ? WHERE id = ?;";
                break;
            case GroupToken::CREATE_TIME:
             Sql = "UPDATE groups SET create_time = ? WHERE id = ?;";
                break;
            default:
                throw(SQLite::Exception("Update Group failed , undefined Token"));
        }

        SQLite::Statement update(*Db,Sql);
         update.bind(1,Value);
        
        update.bind(2,id);
        update.exec();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Update Group failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Update Group failed in Database:  " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Update Group failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Update Group failed in std" << e.what();
        return false;
    }
}

/**
 * @brief 更新学生-分组关联信息，支持更新备注、学生id、分组id等
 * 
 * @param Value 更新后的值
 * @param Id student_group表的id
 * @param Token 更新字段，参考SAGToken枚举（STUDENT_ID、GROUP_ID、REMARK、JOIN_TIME）
 * @return true 更新成功
 * @return false 更新失败
 */
bool DataBaseManager::updateSAG(const std::string& Value,const int& Id,SAGToken Token)
{
    try
    {
        std::string Sql;
        int value;
        if(Token != SAGToken::REMARK)
        {
            value = std::stoi(Value);
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

        SQLite::Statement updateSAG(*Db,Sql);
        if(Token == SAGToken::REMARK)
        {
            updateSAG.bind(1,Value);
        }
        else
        {
            updateSAG.bind(1,value);
        }

        updateSAG.bind(2,Id);

        updateSAG.exec();

        return true;

    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Update SAG failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Update SAG failed in Database:  " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Update SAG failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Update SAG failed in std" << e.what();
        return false;
    }
}

/**
 * @brief 按指定条件对已查询的学生列表进行排序
 * 
 * @param stus 待排序的学生列表
 * @param OrderBy 排序字段，参考StudentToken枚举（ID、NAME、CLASS等）
 * @param Order 排序方向，OrderDirection::ASC为升序，OrderDirection::DESC为降序
 * @return std::vector<Student> 排序后的学生列表
 */
std::vector<Student> DataBaseManager::orderStudent(const std::vector<Student>& stus,StudentToken OrderBy,OrderDirection Order)
{
    std::vector<Student> Return = stus;
    try
    {
        if(Order == OrderDirection::ASC)
        {
            std::sort(Return.begin(),Return.end(),[OrderBy](const Student& a,const Student& b)
            {
                switch(OrderBy)
                {
                    case StudentToken::STUDENT_ID:
                        return a.Student_Id< b.Student_Id;
                        break;
                    case StudentToken::ID:
                        return a.Id< b.Id;
                        break;
                    case StudentToken::NAME:
                        return a.Name< b.Name;
                        break;
                    case StudentToken::CLASS:
                        return a.Class< b.Class;
                        break;
                    case StudentToken::SUBJECT:
                        return a.Subject< b.Subject;
                        break;
                    case StudentToken::CREATE_TIME:
                        return a.Create_Time < b.Create_Time;
                        break;
                    case StudentToken::PHONE:
                        return a.Phone< b.Phone;
                        break;
                    case StudentToken::REMARK:
                        return a.Remark< b.Remark;
                        break;
                    case StudentToken::GENDER:
                        return a.Gender< b.Gender;
                        break;

                    default:
                        throw std::runtime_error("Order Error , undefined StudentToken ");
                        break;
                }
            });
        }
        else 
        {
            std::sort(Return.begin(),Return.end(),[OrderBy](const Student& a,const Student& b)
            {
                switch(OrderBy)
                {
                    case StudentToken::STUDENT_ID:
                        return a.Student_Id> b.Student_Id;
                        break;
                    case StudentToken::ID:
                        return a.Id> b.Id;
                        break;
                    case StudentToken::NAME:
                        return a.Name> b.Name;
                        break;
                    case StudentToken::CLASS:
                        return a.Class> b.Class;
                        break;
                    case StudentToken::SUBJECT:
                        return a.Subject> b.Subject;
                        break;
                    case StudentToken::CREATE_TIME:
                        return a.Create_Time > b.Create_Time;
                        break;
                    case StudentToken::PHONE:
                        return a.Phone> b.Phone;
                        break;
                    case StudentToken::REMARK:
                        return a.Remark> b.Remark;
                        break;
                    case StudentToken::GENDER:
                        return a.Gender> b.Gender;
                        break;

                    default:
                        throw std::runtime_error("Order Error , undefined StudentToken ");
                        break;
                }
            });
        }
        return Return;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Order Student failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Order Student failed in std" << e.what();
        return Return;
    }

    
}

/**
 * @brief 按指定条件对已查询的分组列表进行排序
 * 
 * @param grps 待排序的分组列表
 * @param OrderBy 排序字段，参考GroupToken枚举（ID、GROUP_NAME、DESCRIPTION、CREATE_TIME）
 * @param Order 排序方向，OrderDirection::ASC为升序，OrderDirection::DESC为降序
 * @return std::vector<Group> 排序后的分组列表
 */
std::vector<Group> DataBaseManager::orderGroup(const std::vector<Group>& grps,GroupToken OrderBy,OrderDirection Order)
{   
    std::vector<Group> Return = grps;
    try
    {
        if(Order == OrderDirection::ASC)
        {
            switch(OrderBy)
            {
                case GroupToken::GROUP_NAME :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Group_Name < b.Group_Name;
                    });
                    break;
                case GroupToken::ID :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Id < b.Id;
                    });
                    break;
                case GroupToken::DESCRIPTION :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Description < b.Description;
                    });
                    break;
                case GroupToken::CREATE_TIME :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Create_Time < b.Create_Time;
                    });
                    break;
                default:
                    throw std::runtime_error("Undefined GroupToken in Order Group ");
            }
        }
        else
        {
            switch(OrderBy)
            {
                case GroupToken::GROUP_NAME :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Group_Name > b.Group_Name;
                    });
                    break;
                case GroupToken::ID :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Id > b.Id;
                    });
                    break;
                case GroupToken::DESCRIPTION :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Description > b.Description;
                    });
                    break;
                case GroupToken::CREATE_TIME :
                    std::sort(Return.begin(),Return.end(),[OrderBy](const Group& a,const Group& b)
                    {
                        return a.Create_Time > b.Create_Time;
                    });
                    break;
                default:
                    throw std::runtime_error("Undefined GroupToken in Order Group ");
            }
        }

        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Order Group failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Order Group failed in std" << e.what();
        return Return;
    }

}

/**
 * @brief 检查学生是否存在（根据学号）
 * 
 * @param id 学生学号
 * @return size_t 返回值：EXIST_YES（存在）、EXIST_NO（不存在）、EXIST_ERROR（查询出错）
 */
size_t DataBaseManager::student_exist(const std::string& id)
{
    try
    {
        std::string Sql = "SELECT COUNT(*) FROM students WHERE student_id = ?;";
        SQLite::Statement student_exist(*Db,Sql);
        student_exist.bind(1,id);
        student_exist.executeStep();
        if(student_exist.getColumn(0).getInt() > 0)
        {
            return EXIST_YES;
        }

        return EXIST_NO;
    }
    
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Exist function failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Exist function failed in Database:  " << e.what();
        return EXIST_ERROR;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exist function failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Exist function failed in std" << e.what();
        return EXIST_ERROR;
    }
}

/**
 * @brief 检查分组是否存在（根据分组名称）
 * 
 * @param name 分组名称
 * @return size_t 返回值：EXIST_YES（存在）、EXIST_NO（不存在）、EXIST_ERROR（查询出错）
 */
size_t DataBaseManager::group_exist(const std::string& name)
{
    try
    {
        std::string Sql = "SELECT COUNT(*) FROM groups WHERE name = ?;";
        SQLite::Statement group_exist(*Db,Sql);
        group_exist.bind(1,name);
        group_exist.executeStep();
        if(group_exist.getColumn(0).getInt() > 0)
        {
            return EXIST_YES;
        }

        return EXIST_NO;
    }
    
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Exist function failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Exist function failed in Database:  " << e.what();
        return EXIST_ERROR;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exist function failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Exist function failed in std" << e.what();
        return EXIST_ERROR;
    }
}

/**
 * @brief 检查指定学生是否存在于指定分组中
 * 
 * @param id 学生学号
 * @param name 分组名称
 * @return size_t 返回值：EXIST_YES（学生在该分组中）、EXIST_NO（学生不在该分组中）、EXIST_ERROR（查询出错）
 */
size_t DataBaseManager::student_exist(const std::string& id,const std::string& name)
{
    try
    {
        if(student_exist(id) == EXIST_YES && group_exist(name) == EXIST_YES)
        {
            auto group = exactSearchGroup(name, GroupToken::GROUP_NAME);
            if(group.empty())
            {
                return EXIST_ERROR;
            }
            auto it = std::find_if(group.begin(),group.end(),[&name](const Group& grp)
            {
                return grp.Group_Name == name;
            });
            if(it == group.end())
            {
                return EXIST_ERROR;
            }
            int group_id = it->Id;
            auto student = exactSearchStudent(id,StudentToken::STUDENT_ID);
            if(student.empty())
            {
                return EXIST_ERROR;
            }
            auto stu_it = std::find_if(student.begin(),student.end(),[&id](const Student& stu)
            {
                return stu.Student_Id == id;
            });
            if(stu_it == student.end())
            {
                return EXIST_ERROR;
            }
            int student_id = stu_it->Id;

            auto SAG = searchSAG(std::to_string(group_id), SAGToken::GROUP_ID);
            if(SAG.empty())
            {
                return EXIST_ERROR;
            }
            auto SAG_it = std::find_if(SAG.begin(),SAG.end(),[&](const Student_and_Group& sag)
            {
                return sag.Student_Id == student_id;
            });

            if(SAG_it == SAG.end())
            {
                return EXIST_NO;
            }
            else
            {
                return EXIST_YES;
            }
            
            


        }
        else
        {
            return EXIST_NO;
        }

    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Exist function failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Exist function failed in Database:  " << e.what();
        return EXIST_ERROR;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exist function failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "Exist function failed in std" << e.what();
        return EXIST_ERROR;
    }
}

/**
 * @brief 根据学号获取学生在数据库表中的ID
 *
 * @param student_id 学生学号
 * @return ID_RETURN 返回值：status（ID_OK表示成功，ID_ERROR表示失败）、id（学生表ID，失败时为0）
 */
ID_RETURN DataBaseManager::get_stu_table_id(const std::string& student_id)
{
    ID_RETURN result;
    try
    {
        // 直接查询学生信息
        std::vector<Student> stu = exactSearchStudent(student_id, StudentToken::STUDENT_ID);

        // 检查查询结果
        if(stu.empty())
        {
            // 学生不存在或查询失败
            result.status = ID_ERROR;
            result.id = 0;
        }
        else
        {
            // 成功获取学生ID
            result.status = ID_OK;
            result.id = stu[0].Id;
        }

        return result;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "get_id function failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "get_id function failed in Database:  " << e.what();
        result.id = 0;
        result.status = ID_ERROR;
        return result;
    }
    catch (const std::exception& e)
    {
        std::cerr << "get_id function failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "get_id function failed in std" << e.what();
        result.id = 0;
        result.status = ID_ERROR;
        return result;
    }
}

/**
 * @brief 根据分组名称获取分组在数据库表中的ID
 *
 * @param group_name 分组名称
 * @return ID_RETURN 返回值：status（ID_OK表示成功，ID_ERROR表示失败）、id（分组表ID，失败时为0）
 */
ID_RETURN DataBaseManager::get_grp_table_id(const std::string& group_name)
{
    ID_RETURN result;
    try
    {
        // 直接查询分组信息
        std::vector<Group> grp = exactSearchGroup(group_name, GroupToken::GROUP_NAME);

        // 检查查询结果
        if(grp.empty())
        {
            // 分组不存在或查询失败
            result.status = ID_ERROR;
            result.id = 0;
        }
        else
        {
            // 成功获取分组ID
            result.status = ID_OK;
            result.id = grp[0].Id;
        }

        return result;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "get_id function failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "get_id function failed in Database:  " << e.what();
        result.id = 0;
        result.status = ID_ERROR;
        return result;
    }
    catch (const std::exception& e)
    {
        std::cerr << "get_id function failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "get_id function failed in std" << e.what();
        result.id = 0;
        result.status = ID_ERROR;
        return result;
    }
}

// 获取学生-分组关联记录的ID
// 参数：student_id（学号）、group_name（分组名称）
// 返回：ID_RETURN结构体，包含关联ID和状态
ID_RETURN DataBaseManager::get_SAG_table_id(const std::string& student_id,const std::string& group_name)
{
    ID_RETURN result;
    try
    {
        // SQL查询语句：获取指定学生在指定分组中的关联记录ID
        std::string Sql = "SELECT id FROM student_group WHERE student_id = ? AND group_id = ?;";

        // 查询分组信息，获取分组ID
        std::vector<Group> grp = exactSearchGroup(group_name, GroupToken::GROUP_NAME);

        // 查询学生信息，获取学生ID
        std::vector<Student> stu = exactSearchStudent(student_id, StudentToken::STUDENT_ID);

        // 声明分组ID和学生ID变量
        int grp_id;
        int stu_id;

        // 检查分组是否存在
        if(grp.empty())
        {
            // 分组不存在或查询失败
            result.status = ID_ERROR;
            result.id = 0;

            return result;
        }
        else
        {
            // 成功获取分组ID
            grp_id = grp[0].Id;
        }

        // 检查学生是否存在
        if(stu.empty())
        {
            // 学生不存在或查询失败
            result.status = ID_ERROR;
            result.id = 0;
            return result;
        }
        else
        {
            // 成功获取学生ID
            stu_id = stu[0].Id;
        }

        // 查询学生-分组关联记录
        SQLite::Statement return_id(*Db,Sql);
        return_id.bind(1,stu_id);  // 绑定学生ID
        return_id.bind(2,grp_id);  // 绑定分组ID

        // 检查是否找到关联记录
        if(return_id.executeStep())
        {
            // 找到关联记录，返回关联ID
            result.id = return_id.getColumn("id").getInt();
            result.status = ID_OK;
        }
        else
        {
            // 未找到关联记录（该学生不在该分组中）
            result.id = 0;
            result.status = ID_ERROR;
        }


        return result;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "get_id function failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "get_id function failed in Database:  " << e.what();
        result.id = 0;
        result.status = ID_ERROR;
        return result;
    }
    catch (const std::exception& e)
    {
        std::cerr << "get_id function failed in std" << e.what() << std::endl;
        CROW_LOG_ERROR << "get_id function failed in std" << e.what();
        result.id = 0;
        result.status = ID_ERROR;
        return result;
    }
}

bool DataBaseManager::addUser(const std::vector<User>& users)
{
    try
    {
        //添加数据库事务
        SQLite::Transaction addTransaction(*Db);

        //初始化添加sql语句
        const std::string Sql = "INSERT OR IGNORE INTO users (user_name,password) VALUES (?,?);";
        
        //绑定数据库与sql语句
        SQLite::Statement addUser(*Db,Sql);

        //绑定sql语句参数
        for(const auto& user : users)
        {
            addUser.bind(1,user.User_Name);
            addUser.bind(2,user.Password);
            addUser.exec();

            //重置参数绑定，方便读取下一个输入的学生信息
            addUser.reset();
        }

        //提交事务，执行以上操作，有问题自动报错后回滚
        addTransaction.commit();
        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Add User Error in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add User Error in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Add User Error in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Add User Error in std: " << e.what();
        return false;
    }
}

bool DataBaseManager::deleteUser(const std::vector<User>& users)
{
    try
    {
        std::string Sql = "DELETE FROM users WHERE id = ?;";
        SQLite::Transaction deleteTransaction(*Db);
        SQLite::Statement deleteStudent(*Db,Sql);
        for(const auto& user : users)
        {
            deleteStudent.bind(1,user.Id);
            deleteStudent.exec();
            deleteStudent.reset();
        }

        deleteTransaction.commit();

        return true;
    }
    catch(const SQLite::Exception& e)
    {
        std::cerr << "Delete User failed in Database: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete User failed in Database: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Delete User failed in std: " << e.what() << std::endl;
        CROW_LOG_ERROR << "Delete User failed in std: " << e.what();
        return false;
    }

}

bool DataBaseManager::updateUser(const std::string& Value,int Id,UserToken Token)

{

    try

    {

        std::string Sql;

        switch(Token)

        {

            case UserToken::USER_NAME:

                Sql = "UPDATE users SET user_name = ? WHERE id = ?;";

                break;

            case UserToken::PASSWORD:

                Sql = "UPDATE users SET password = ? WHERE id = ?;";

                break;

            default:

                throw(SQLite::Exception("Update Users failed , undefined Token"));



        }



        SQLite::Statement update(*Db,Sql);

        

        update.bind(1,Value);    

        

        

        update.bind(2,Id);

        update.exec();



        return true;

    }

    catch(const SQLite::Exception& e)

    {

        std::cerr << "Update User failed in Database: " << e.what() << std::endl;

        CROW_LOG_ERROR << "Update User failed in Database:  " << e.what();

        return false;

    }

    catch (const std::exception& e)

    {

        std::cerr << "Update User failed in std" << e.what() << std::endl;

        CROW_LOG_ERROR << "Update User failed in std" << e.what();

        return false;

    }



};

std::vector<User> DataBaseManager::searchUser(const std::string& Value,UserToken Token)
{
     //返回值
    std::vector<User> Return;
    try
    {
        
        std::string Sql;//sql语句

        
        int Value_Copy = 0;//条件为整数时的复制载体

        //根据搜索条件初始化sql语句，精确条件搜索
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

        //将sql语句与具体数据库绑定
        SQLite::Statement findUser(*Db,Sql);

        //sql语句的参数绑定
         findUser.bind(1,Value);

        //执行查询操作，可能有多条结果，所以用while
        while(findUser.executeStep())
        {

            User temp;//行内容暂存变量

            temp.Id = findUser.getColumn("id").getInt();
            temp.User_Name = findUser.getColumn("user_name").getText();
            temp.Password = findUser.getColumn("password").getText();

            //将行内容放入最终返回变量中
            Return.emplace_back(temp);
        }

        return Return;

    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "DateBase Search failed in searchUser: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in searchUser: " << e.what();
        return Return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "DateBase Search failed in searchUser and is 'std' Error: " << e.what() << std::endl;
        CROW_LOG_ERROR << "DateBase Search failed in searchUser and is 'std' Error: " << e.what();
        return Return;
    }
}

bool DataBaseManager::verifyUser(const std::string& username,const std::string& password)
{
    try
    {
        SQLite::Statement verify(*Db,"SELECT EXISTS(SELECT 1 FROM users WHERE user_name = ? AND password = ?);");

        verify.bind(1,username);
        verify.bind(2,password);
        
        verify.executeStep();

        return verify.getColumn(0).getInt() == 1;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << " " << e.what() << std::endl;
        CROW_LOG_ERROR << " " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "  " << e.what() << std::endl;
        CROW_LOG_ERROR << "  " << e.what();
        return false;
    }
}