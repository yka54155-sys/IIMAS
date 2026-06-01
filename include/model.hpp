#pragma  once
#include"std.hpp"

/**
 * @brief 学生结构体，存储学生的基本信息
 *
 * 对应数据库中的students表
 */
typedef struct Student
{
    int Id;                      // 数据库ID（主键）
    std::string Student_Id;      // 学号
    std::string Name;            // 姓名
    std::string Gender;          // 性别
    std::string Phone;           // 电话号码
    int Class;                   // 班级
    std::string Subject;         // 专业
    std::string Remark;          // 备注
    std::string Create_Time;     // 创建时间

    // 重载 < 运算符，支持在std::set中自动排序
    bool operator < (const Student& other) const
    {
        return Id < other.Id;
    }

}Student;

/**
 * @brief 学生字段枚举，用于标识搜索/排序的字段
 */
enum class StudentToken
{
    ID,         // 数据库ID
    NAME,       // 姓名
    PHONE,      // 电话
    CLASS,      // 班级
    SUBJECT,    // 专业
    REMARK,     // 备注
    STUDENT_ID, // 学号
    CREATE_TIME,// 创建时间
    GENDER      // 性别


};


// 字符串到StudentToken的映射表，用于API参数解析
inline std::map<std::string,StudentToken> StudentMap =
{
    {"id",StudentToken::ID},
    {"name",StudentToken::NAME},
    {"phone",StudentToken::PHONE},
    {"class",StudentToken::CLASS},
    {"subject",StudentToken::SUBJECT},
    {"remark",StudentToken::REMARK},
    {"student_id",StudentToken::STUDENT_ID},
    {"create_time",StudentToken::CREATE_TIME},
    {"gender",StudentToken::GENDER}
};

/**
 * @brief 分组结构体，存储分组的基本信息
 *
 * 对应数据库中的groups表
 */
typedef struct Group
{
    int Id;                      // 数据库ID（主键）
    std::string Group_Name;      // 分组名称
    std::string Description;     // 分组描述
    std::string Create_Time;     // 创建时间

    // 重载 < 运算符，支持在std::set中自动排序
    bool operator < (const Group& other) const
    {
        return Id < other.Id;
    }
}Group;

/**
 * @brief 分组字段枚举，用于标识搜索/排序的字段
 */
enum class GroupToken
{
    ID,             // 数据库ID
    GROUP_NAME,     // 分组名称
    DESCRIPTION,    // 分组描述
    CREATE_TIME     // 创建时间

};

// 字符串到GroupToken的映射表，用于API参数解析
inline std::map<std::string,GroupToken> GroupMap =
{
    {"id",GroupToken::ID},
    {"name",GroupToken::GROUP_NAME},
    {"create_time",GroupToken::CREATE_TIME},
    {"description",GroupToken::DESCRIPTION}
};

/**
 * @brief 学生-分组关联结构体，存储学生与分组的关联关系
 *
 * 对应数据库中的student_group表
 */
typedef struct Student_and_Group
{
    int Id;                      // 关联ID（主键）
    int Student_Id;              // 学生ID（外键）
    int Group_Id;                // 分组ID（外键）
    std::string Remark;          // 备注（该学生在该分组中的独立备注）
    std::string Join_Time;       // 加入时间

    // 重载 < 运算符，支持在std::set中自动排序
    bool operator < (const Student_and_Group& other) const
    {
        return Id < other.Id;
    }

}Student_and_Group;

// 别名，简化类型名称
using SAG = Student_and_Group;


/**
 * @brief 学生-分组关联字段枚举，用于标识搜索/更新的字段
 */
enum class StudentAndGroupToken
{
    ID,         // 关联ID
    STUDENT_ID, // 学生ID
    GROUP_ID,   // 分组ID
    JOIN_TIME,  // 加入时间
    REMARK      // 备注
};
// 别名，简化类型名称
using SAGToken = StudentAndGroupToken;

// 字符串到SAGToken的映射表，用于API参数解析
inline std::map<std::string,SAGToken> SAGMap =
{
    {"id",SAGToken::ID},
    {"student",SAGToken::STUDENT_ID},
    {"join_time",SAGToken::JOIN_TIME},
    {"remark",SAGToken::REMARK},
    {"group",SAGToken::GROUP_ID}
};



/**
 * @brief 排序方向枚举
 */
enum class OrderDirection
{
    ASC,  // 升序（从小到大）
    DESC  // 降序（从大到小）
};

inline std::map<std::string,OrderDirection>OrderDirectionMap = 
{
    {"asc",OrderDirection::ASC},
    {"desc",OrderDirection::DESC}
};

struct User
{
    int Id;
    std::string User_Name;
    std::string Password;
    bool operator < (const User& other) const 
    {
        return Id < other.Id;
    }
};

enum class UserToken
{
    ID,
    USER_NAME,
    PASSWORD
};

inline std::map<std::string,UserToken> UserMap= 
{
    {"id",UserToken::ID},
    {"user_name",UserToken::USER_NAME},
    {"password",UserToken::PASSWORD}
};
