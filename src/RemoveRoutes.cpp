#include"std.hpp"
#include"crow.h"
#include"Crow_API.hpp"
#include"model.hpp"
#include"ApiResponse.hpp"
#include"TokenStore.hpp"
#include"StudentService.hpp"
#include"GroupService.hpp"
#include"SAGService.hpp"
/**
 * @brief 设置删除相关路由
 *
 * 注册学生删除、分组删除等API端点
 */
void IIMAS_API::setupRemoveRoutes()
{
    // 删除学生（从数据库中完全删除，包括关联）
    CROW_ROUTE(app, "/api/deleteStudent/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;
        try
        {
            // 解析请求体JSON
            auto json = crow::json::load(req.body);
            // 检查JSON是否为空
            if(json.t() == crow::json::type::Null)
            {
                result["status"] = "failed";
                result["message"] = "删除失败，json内容不能为空";

                return crow::response(400,result);
            }
            // 检查students字段是否存在
            if(!json.has("students"))
            {
                result["status"] = "failed";
                result["message"] = "删除失败，json缺少必要字段students";

                return crow::response(400,result);
            }
            // 检查students是否为数组格式
            if(json["students"].t() != crow::json::type::List)
                {

                result["status"] = "failed";
                result["message"] = "删除失败，students格式必须为列表格式";

                return crow::response(400,result);
            }

            // 初始化学生列表
            std::vector<Student> stus;
            // 遍历students数组，验证并构建Student对象
            for(const auto& json_stu : json["students"])
            {
                Student temp;
                // 检查student_id字段是否存在
                if(!json_stu.has("student_id"))
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，students缺少student_id";

                    return crow::response(400,result);
                }
                // 提取student_id
                temp.Student_Id = json_stu["student_id"].s();
                // 检查student_id是否为空
                if(temp.Student_Id.empty())
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，student_id不可为空";

                    return crow::response(400,result);
                }
                // 获取学生的数据库ID
                ID_RETURN id = db.get_stu_table_id(temp.Student_Id);
                if(id.status == ID_ERROR)
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，student_id不存在";

                    return crow::response(400,result);
                }
                // 设置学生ID
                temp.Id = id.id;

                // 添加到学生列表
                stus.emplace_back(temp);
            }

            // 调用删除函数
            if(!db.deleteStudent(stus))
            {

                result["status"] = "failed";
                result["message"] = "删除失败，调用delete函数时出错";

                return crow::response(400,result);
            }

            // 返回成功结果
            result["status"] = "success";
            result["message"] = "删除成功";

            return crow::response(200,result);
        }
        // 捕获数据库异常
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database deleteStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database deleteStudent: " << e.what();
            std::string error = e.what();
            std::string message = "删除失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std deleteStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std deleteStudent: " << e.what();
            std::string error = e.what();
            std::string message = "删除失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });
    // 删除分组（从数据库中完全删除，包括关联）
    CROW_ROUTE(app, "/api/deleteGroup/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;
        try
        {
            // 解析请求体JSON
            auto json = crow::json::load(req.body);
            // 检查JSON是否为空
            if(json.t() == crow::json::type::Null)
            {
                result["status"] = "failed";
                result["message"] = "删除失败，json内容不能为空";

                return crow::response(400,result);
            }
            // 检查groups字段是否存在
            if(!json.has("groups"))
            {
                result["status"] = "failed";
                result["message"] = "删除失败，json缺少必要字段groups";

                return crow::response(400,result);
            }
            // 检查groups是否为数组格式
            if(json["groups"].t() != crow::json::type::List)
                {

                result["status"] = "failed";
                result["message"] = "删除失败，groups格式必须为列表格式";

                return crow::response(400,result);
            }

            // 初始化分组列表
            std::vector<Group> grps;
            // 遍历groups数组，验证并构建Group对象
            for(const auto& json_grp : json["groups"])
            {
                Group temp;
                // 检查name字段是否存在
                if(!json_grp.has("name"))
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，groups缺少name";

                    return crow::response(400,result);
                }
                // 提取分组名称
                temp.Group_Name = json_grp["name"].s();
                // 检查name是否为空
                if(temp.Group_Name.empty())
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，name不可为空";

                    return crow::response(400,result);
                }
                // 获取分组的数据库ID
                ID_RETURN id = db.get_grp_table_id(temp.Group_Name);
                if(id.status == ID_ERROR)
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，name不存在";

                    return crow::response(400,result);
                }
                // 设置分组ID
                temp.Id = id.id;

                // 添加到分组列表
                grps.emplace_back(temp);
            }

            // 调用删除分组函数
            if(!db.deleteGroup(grps))
            {

                result["status"] = "failed";
                result["message"] = "删除失败，调用delete函数时出错";

                return crow::response(400,result);
            }

            // 返回成功结果
            result["status"] = "success";
            result["message"] = "删除成功";

            return crow::response(200,result);
        }
        // 捕获数据库异常
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database deleteGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database deleteGroup: " << e.what();
            std::string error = e.what();
            std::string message = "删除失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std deleteGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std deleteGroup: " << e.what();
            std::string error = e.what();
            std::string message = "删除失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });
    // 从分组中移除学生（删除关联关系，不影响students表）
    CROW_ROUTE(app, "/api/deleteSAG/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;
        try
        {
            // 解析请求体JSON
            auto json = crow::json::load(req.body);
            // 检查JSON是否为空
            if(json.t() == crow::json::type::Null)
            {
                result["status"] = "failed";
                result["message"] = "删除失败，json内容不能为空";

                return crow::response(400,result);
            }
            // 检查students字段是否存在
            if(!json.has("students") || !json.has("group_name"))
            {
                result["status"] = "failed";
                result["message"] = "删除失败，json缺少必要字段students 或 group_name";

                return crow::response(400,result);
            }
            // 检查students是否为数组格式
            if(json["students"].t() != crow::json::type::List)
                {

                result["status"] = "failed";
                result["message"] = "删除失败，students格式必须为列表格式";

                return crow::response(400,result);
            }

            // 初始化学生列表
            std::vector<Student> stus;
            // 遍历students数组，验证并构建Student对象
            for(const auto& json_stu : json["students"])
            {
                Student temp;
                // 检查student_id字段是否存在
                if(!json_stu.has("student_id"))
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，students缺少student_id";

                    return crow::response(400,result);
                }
                // 提取student_id
                temp.Student_Id = json_stu["student_id"].s();
                // 检查student_id是否为空
                if(temp.Student_Id.empty())
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，student_id不可为空";

                    return crow::response(400,result);
                }
                // 获取学生的数据库ID
                ID_RETURN id = db.get_stu_table_id(temp.Student_Id);
                if(id.status == ID_ERROR)
                {

                    result["status"] = "failed";
                    result["message"] = "删除失败，student_id不存在";

                    return crow::response(400,result);
                }
                // 设置学生ID
                temp.Id = id.id;

                // 添加到学生列表
                stus.emplace_back(temp);
            }

            // 获取分组的数据库ID
            ID_RETURN grp_id = db.get_grp_table_id(json["group_name"].s());
            if(grp_id.status == ID_ERROR)
            {
                
                result["status"] = "failed";
                result["message"] = "删除失败，group_name不存在";

                return crow::response(400,result);
            }

            // 调用删除关联关系函数（从指定分组移除学生）
            if(!db.deleteStudent(stus,grp_id.id))
            {

                result["status"] = "failed";
                result["message"] = "删除失败，调用delete函数时出错";

                return crow::response(400,result);
            }

            // 返回成功结果
            result["status"] = "success";
            result["message"] = "删除成功";

            return crow::response(200,result);
        }
        // 捕获数据库异常
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database deleteSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database deleteSAG: " << e.what();
            std::string error = e.what();
            std::string message = "删除失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std deleteSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std deleteSAG: " << e.what();
            std::string error = e.what();
            std::string message = "删除失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });


}