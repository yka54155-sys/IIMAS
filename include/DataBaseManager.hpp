
#pragma once



#include "SQLiteCpp/Database.h"

#include <cstddef>

#include <memory>

#include <vector>

#include"model.hpp"

#include"std.hpp"



// 数据库管理类，用于数据库的各项操作



/**

 * @brief 存在状态常量定义

 */

#define EXIST_YES 1      // 数据存在

#define EXIST_ERROR 0    // 查询出错

#define EXIST_NO 2       // 数据不存在

#define ID_OK 3          // ID 获取成功

#define ID_ERROR 4       // ID 获取失败



/**

 * @brief ID 返回结构体

 * 

 * 用于返回 ID 查询结果和状态

 */

typedef struct ID_RETURN

{

    int status;  // 查询状态码

    int id;      // 查询到的 ID 值

    

}ID_RETURN;



/**

 * @brief 数据库管理类

 * 

 * 负责所有数据库操作，包括学生、分组和学生-分组关联的增删改查

 */

class DataBaseManager

{

    private:

    std::unique_ptr<SQLite::Database> Db;  // SQLite 数据库智能指针



    public:

    /**

     * @brief 初始化数据库

     * 

     * 构造函数，自动打开或创建数据库文件，初始化表结构

     */

    DataBaseManager();
    DataBaseManager(const DataBaseManager&) = delete;
    DataBaseManager& operator=(const DataBaseManager&) = delete;

    DataBaseManager(DataBaseManager&&) = delete;
    DataBaseManager& operator=(DataBaseManager&&) = delete;

    

    

    // ========== 增加操作函数 ==========

    

    /**

     * @brief 在 students 表添加学生数据

     * @param stus 学生数据列表

     * @return 操作是否成功

     */

    bool addStudent(const std::vector<Student>& stus);

    

    /**

     * @brief 将学生关联某个分组

     * @param stus 学生数据列表

     * @param id 分组 ID

     * @return 操作是否成功

     */

    bool addStudent(const std::vector<Student>& stus,const int& id);

    

    /**

     * @brief 在 groups 表添加分组信息

     * @param grps 分组数据列表

     * @return 操作是否成功

     */

    bool addGroup(const std::vector<Group>& grps);



    // ========== 删除操作函数 ==========

    

    /**

     * @brief 在 students 表删除学生数据，会删除关联内容

     * @param stus 学生数据列表

     * @return 操作是否成功

     */

    bool deleteStudent(const std::vector<Student>& stus);

    

    /**

     * @brief 将学生从关联分组中删除，不会影响 students 表中内容

     * @param stus 学生数据列表

     * @param id 分组 ID

     * @return 操作是否成功

     */

    bool deleteStudent(const std::vector<Student>& stus,const int& id);

    

    /**

     * @brief 在 groups 表删除分组信息

     * @param grps 分组数据列表

     * @return 操作是否成功

     */

    bool deleteGroup(const std::vector<Group>& grps);

    

    // ========== 修改操作函数 ==========

    

    /**

     * @brief 单条修改学生信息

     * @param Value 修改后的值

     * @param Id 学生 ID

     * @param Token 修改的字段类型

     * @return 操作是否成功

     */

    bool updateStudent(const std::string& Value,const int& Id,StudentToken Token);

    

    /**

     * @brief 批量修改学生信息

     * @param stus 学生 ID 列表

     * @param Value 修改后的值

     * @param Token 修改的字段类型

     * @return 操作是否成功

     */

    bool updateStudent(const std::vector<int>& stus,const std::string& Value,StudentToken Token);

    

    /**

     * @brief 修改分组信息

     * @param value 修改后的值

     * @param id 分组 ID

     * @param Token 修改的字段类型

     * @return 操作是否成功

     */

    bool updateGroup(const std::string& value,const int& id,GroupToken Token);

    

    /**

     * @brief 修改学生-分组关联信息

     * @param value 修改后的值

     * @param id 关联 ID

     * @param Token 修改的字段类型

     * @return 操作是否成功

     */

    bool updateSAG(const std::string& value,const int& id,SAGToken Token);

    

    // ========== 查询操作函数 ==========

    

    /**

     * @brief 精确搜索学生

     * @param Value 搜索值

     * @param Token 搜索字段类型

     * @return 符合条件的学生列表

     */

    std::vector<Student> exactSearchStudent(const std::string& Value,const StudentToken Token);

    

    /**

     * @brief 模糊搜索学生

     * @param Value 搜索值

     * @param Token 搜索字段类型

     * @return 符合条件的学生列表

     */

    std::vector<Student> fuzzySearchStudent(const std::string& Value,const StudentToken Token);

    

    /**

     * @brief 精确搜索分组

     * @param Value 搜索值

     * @param Token 搜索字段类型

     * @return 符合条件的分组列表

     */

    std::vector<Group> exactSearchGroup(const std::string& Value,GroupToken Token);

    

    /**

     * @brief 模糊搜索分组

     * @param Value 搜索值

     * @param Token 搜索字段类型

     * @return 符合条件的分组列表

     */

    std::vector<Group> fuzzySearchGroup(const std::string& Value,GroupToken Token);

    

    /**

     * @brief 搜索学生-分组关联

     * @param Id 搜索 ID（学生 ID 或分组 ID）

     * @param Token 搜索类型（按学生搜或按分组搜）

     * @return 符合条件的关联列表

     */

    std::vector<SAG> searchSAG(const std::string& Id,SAGToken Token);

    

    // ========== 按条件排序 ==========

    

    /**

     * @brief 按指定字段对学生排序

     * @param stus 学生列表

     * @param OrderBy 排序字段

     * @param Order 排序方向

     * @return 排序后的学生列表

     */

    std::vector<Student> orderStudent(const std::vector<Student>& stus,StudentToken OrderBy,OrderDirection Order);

    

    /**

     * @brief 按指定字段对分组排序

     * @param grps 分组列表

     * @param OrderBy 排序字段

     * @param Order 排序方向

     * @return 排序后的分组列表

     */

    std::vector<Group> orderGroup(const std::vector<Group>& grps,GroupToken OrderBy,OrderDirection Order);

    

    // ========== 其他辅助函数 ==========

    

    /**

     * @brief 检查学生是否存在

     * @param id 学生学号

     * @return 存在的记录数（0 表示不存在）

     */

    size_t student_exist(const std::string& id);

    

    /**

     * @brief 检查分组是否存在

     * @param name 分组名称

     * @return 存在的记录数（0 表示不存在）

     */

    size_t group_exist(const std::string& name);

    

    /**

     * @brief 检查学生是否在指定分组中

     * @param id 学生学号

     * @param name 分组名称

     * @return 存在的记录数（0 表示不存在）

     */

    size_t student_exist(const std::string& id,const std::string& name);

    

    /**

     * @brief 根据学号获取学生表 ID

     * @param student_id 学生学号

     * @return ID_RETURN 结构体，包含状态和 ID

     */

    ID_RETURN get_stu_table_id(const std::string& student_id);

    

    /**

     * @brief 根据分组名获取分组表 ID

     * @param group_name 分组名称

     * @return ID_RETURN 结构体，包含状态和 ID

     */

    ID_RETURN get_grp_table_id(const std::string& group_name);

    

    /**

     * @brief 根据学号和分组名获取关联表 ID

     * @param student_id 学生学号

     * @param group_name 分组名称

     * @return ID_RETURN 结构体，包含状态和 ID

     */

    ID_RETURN get_SAG_table_id(const std::string& student_id,const std::string& group_name);

    bool addUser(const std::vector<User>&);
    bool deleteUser(const std::vector<User>&);
    bool updateUser(const std::string& ,int,UserToken);
    std::vector<User> searchUser(const std::string& , UserToken);
    bool verifyUser(const std::string& username,const std::string& password);
    

};

