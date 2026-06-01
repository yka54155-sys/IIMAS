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
 * @brief 设置修改相关路由
 *
 * 注册学生修改、分组修改等API端点
 */
void IIMAS_API::setupUpdateRoutes()
{
    // 修改学生信息（支持单个修改和批量修改）
    // single模式：修改单个学生
    // batch模式：批量修改多个学生
    CROW_ROUTE(app, "/api/updateStudent/<string>").methods("POST"_method)([&](const crow::request& req,const std::string& mode)
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
                result["message"] = "修改失败，json不可为空";

                return crow::response(400,result);
            }
            // 单个修改模式
            if(mode == "single")
            {
                // 检查必填字段
                if(!json.has("value") || !json.has("token") || !json.has("student_id"))
                {
                    result["status"] = "failed";
                    result["message"] = "修改失败，json 缺少 value 或 token 、 student_id";

                    return crow::response(400,result);
                }

                // 检查必填字段是否为空
                if(json["value"].s() == "" || json["token"].s() == "" || json["student_id"].s() == "")
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败， value 或 token 、 student_id不可为空";

                    return crow::response(400,result);
                }
                // 验证token是否合法
                auto it = StudentMap.find(json["token"].s());
                if(it == StudentMap.end())
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败，非法token值";

                    return crow::response(400,result);
                }

                // 获取学生数据库ID
                ID_RETURN id = db.get_stu_table_id(json["student_id"].s());
                if(id.status == ID_ERROR)
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败，不存在的student_id";

                    return crow::response(400,result);
                }

                // 调用修改函数
                if(!db.updateStudent(json["value"].s(),id.id,it->second))
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败，调用update函数时出错";

                    return crow::response(400,result);
                }



            }
            // 批量修改模式
            else if(mode == "batch")
            {
                // 检查必填字段
                if(!json.has("students") || !json.has("value") || !json.has("token"))
                {
                    result["status"] = "failed";
                    result["message"] = "修改失败，json中缺少students 、value 或 token";

                    return crow::response(400,result);
                }
                // 检查students是否为数组
                if(json["students"].t() != crow::json::type::List)
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败，students应当为列表类型";

                    return crow::response(400,result);
                }

                // 检查必填字段是否为空
                if(json["value"].s() == "" || json["token"].s() == "")
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败，value 和 token不可为空";

                    return crow::response(400,result);
                }

                // 验证token是否合法
                auto it = StudentMap.find(json["token"].s());
                if(it == StudentMap.end())
                {

                    result["status"] = "failed";
                    result["message"] = "修改失败，非法token值";

                    return crow::response(400,result);
                }


                // 初始化学生ID列表
                std::vector<int> stus;

                // 遍历students数组，获取每个学生的ID
                for(const auto& json_stu : json["students"])
                {
                    Student temp;
                    // 检查student_id字段
                    if(!json_stu.has("student_id"))
                    {

                        result["status"] = "failed";
                        result["message"] = "修改失败，缺少student_id";

                        return crow::response(400,result);
                    }

                    // 提取student_id
                    temp.Student_Id = json_stu["student_id"].s();
                    // 检查student_id是否为空
                    if(temp.Student_Id.empty())
                    {

                        result["status"] = "failed";
                        result["message"] = "修改失败，student_id不可为空";

                        return crow::response(400,result);
                    }

                    // 获取学生数据库ID
                    ID_RETURN id  = db.get_stu_table_id(temp.Student_Id);
                    if(id.status == ID_ERROR)
                    {

                        result["status"] = "failed";
                        result["message"] = "修改失败，不存在的student_id";

                        return crow::response(400,result);
                    }
                    
                    // 添加到ID列表
                    stus.emplace_back(id.id);

                }

                // 调用批量修改函数
                db.updateStudent(stus,json["value"].s(),it->second);


            }
            // 无效的mode参数
            else
            {
                result["status"] = "failed";
                result["message"] = "URL错误,请检查/api/updateStudent/?  <---此处参数  ，只能为single 或 batch ";

                return crow::response(400,result);
            }

            // 返回成功结果
            result["status"] = "success";
            result["message"] = "修改成功";

            return crow::response(200,result);
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database updateStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database updateStudent: " << e.what();
            std::string error = e.what();
            std::string message = "修改失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std updateStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std updateStudent: " << e.what();
            std::string error = e.what();
            std::string message = "修改失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
        
        

    });
    // 修改分组信息
    CROW_ROUTE(app, "/api/updateGroup/").methods("POST"_method)([&](const crow::request& req)
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
                result["message"] = "修改失败，json不可为空";

                return crow::response(400,result);
            }
            // 检查必填字段
            if(!json.has("value") || !json.has("token") || !json.has("group_name"))
            {
                result["status"] = "failed";
                result["message"] = "修改失败，json 缺少 value 或 token 、 group_name";

                return crow::response(400,result);
            }

            // 检查必填字段是否为空
            if(json["value"].s() == "" || json["token"].s() == "" || json["group_name"].s() == "")
            {

                result["status"] = "failed";
                result["message"] = "修改失败， value 或 token 、 group_name不可为空";

                return crow::response(400,result);
            }
            // 验证token是否合法
            auto it = GroupMap.find(json["token"].s());
            if(it == GroupMap.end())
            {

                result["status"] = "failed";
                result["message"] = "修改失败，非法token值";

                return crow::response(400,result);
            }

            // 获取分组数据库ID
            ID_RETURN id = db.get_grp_table_id(json["group_name"].s());
            if(id.status == ID_ERROR)
            {

                result["status"] = "failed";
                result["message"] = "修改失败，不存在的group_name";

                return crow::response(400,result);
            }

            // 调用修改函数
            if(!db.updateGroup(json["value"].s(),id.id,it->second))
            {

                result["status"] = "failed";
                result["message"] = "修改失败，调用update函数时出错";

                return crow::response(400,result);
            }


            // 返回成功结果
            result["status"] = "success";
            result["message"] = "修改成功";

            return crow::response(200,result);
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database updateGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database updateGroup: " << e.what();
            std::string error = e.what();
            std::string message = "修改失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std updateGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std updateGroup: " << e.what();
            std::string error = e.what();
            std::string message = "修改失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
        
        

    });
    // 修改学生-分组关联信息（如备注）
    CROW_ROUTE(app, "/api/updateSAG/").methods("POST"_method)([&](const crow::request& req)
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
                result["message"] = "修改失败，json不可为空";

                return crow::response(400,result);
            }
            // 检查必填字段
            if(!json.has("value") || !json.has("token") || !json.has("group_name") || !json.has("student_id"))
            {
                result["status"] = "failed";
                result["message"] = "修改失败，json 缺少 value 或 token 、 group_name 、student_id";

                return crow::response(400,result);
            }

            // 检查必填字段是否为空
            if(json["value"].s() == "" || json["token"].s() == "" || json["group_name"].s() == "" || json["student_id"].s() == "") 
            {

                result["status"] = "failed";
                result["message"] = "修改失败， value 或 token 、 group_name 、 student_id不可为空";

                return crow::response(400,result);
            }
            // 验证token是否合法
            auto it = SAGMap.find(json["token"].s());
            if(it == SAGMap.end())
            {

                result["status"] = "failed";
                result["message"] = "修改失败，非法token值";

                return crow::response(400,result);
            }

            ID_RETURN id = db.get_SAG_table_id(json["student_id"].s(),json["group_name"].s());

            if(id.status == ID_ERROR)
            {
                result["status"] = "failed";
                result["message"]  = "不存在的关联信息";

                return crow::response(400,result);
            }
            // 调用修改函数
            if(!db.updateSAG(json["value"].s(),id.id,it->second))
            {

                result["status"] = "failed";
                result["message"] = "修改失败，调用update函数时出错";

                return crow::response(400,result);
            }


            // 返回成功结果
            result["status"] = "success";
            result["message"] = "修改成功";

            return crow::response(200,result);
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database updateSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database updateSAG: " << e.what();
            std::string error = e.what();
            std::string message = "修改失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std updateSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std updateSAG: " << e.what();
            std::string error = e.what();
            std::string message = "修改失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
        
        

    });
}