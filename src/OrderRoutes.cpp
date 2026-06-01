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
 * @brief 设置排序相关路由
 *
 * 注册学生排序API端点，支持按不同字段和方向排序
 */
void IIMAS_API::setupOrderRoutes()
{
    CROW_ROUTE(app, "/api/orderStudent/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;
        try
        {
            // 解析请求体中的JSON数据
            auto order_json = crow::json::load(req.body);
            // 验证JSON格式是否正确
            if(order_json.t() == crow::json::type::Null)
            {
                result["status"] = "failed";
                result["message"] = "排序失败，json格式错误";

                return crow::response(400,result);
            }
            // 验证必需字段是否存在
            if(!order_json.has("students") || !order_json.has("token") || !order_json.has("direction"))
            {
                result["status"] = "failed";
                result["message"] = "排序失败，缺少students,token,direction中某个因素";

                return crow::response(400,result);
            }
            // 获取排序参数
            auto token = order_json["token"];
            auto direction = order_json["direction"];
            if(order_json["students"].t() != crow::json::type::List)
            {
                result["status"] = "failed";
                result["message"] = "排序失败，students的格式不对，应当为列表";

                return crow::response(400,result);
            }
            auto it = StudentMap.find(token.s());
            if(it == StudentMap.end())
            {
                result["status"] = "failed";
                result["message"] = "排序失败，非法token值";

                return crow::response(400,result);
            }
            // 验证排序方向是否合法
            if(!(direction.s() == "asc" || direction.s() == "desc"))
            {
                result["status"] = "failed";
                result["message"] = "排序失败，非法direction值";

                return crow::response(400,result);
            }
            auto dir_it = OrderDirectionMap.find(direction.s());
            

            // 初始化数据结构
            std::vector<Student> stus;
            std::vector<Student> order_stus;
            crow::json::wvalue json_stus = crow::json::wvalue::list();
            // 解析学生列表数据
            for(const auto& json_stu : order_json["students"])
            {
                Student temp;
                // 从JSON中提取学生信息，必填字段
                temp.Student_Id = json_stu["student_id"].s();
                temp.Name = json_stu["name"].s();
                // 可选字段，提供默认值
                temp.Class = json_stu.has("class") ? json_stu["class"].i() : 0;
                temp.Subject = json_stu.has("subject") ? json_stu["subject"].s() : std::string("");
                temp.Remark = json_stu.has("remark") ? json_stu["remark"].s() : std::string("");
                temp.Phone = json_stu.has("phone") ? json_stu["phone"].s() : std::string("");
                temp.Gender = json_stu.has("gender") ? json_stu["gender"].s() : std::string("");
                temp.Create_Time = json_stu.has("create_time") ? json_stu["create_time"].s() : std::string("");

                // 添加到待排序列表
                order_stus.emplace_back(temp);
            }
            // 调用数据库排序函数（复用前面 find() 得到的迭代器，避免重复查找）
            stus = db.orderStudent(order_stus, it->second, dir_it->second);
            // 构建排序后的JSON响应
            int index = 0;
            for(const auto& stu : stus)
            {
                crow::json::wvalue temp;
                // 填充学生数据到JSON对象
                temp["name"] = stu.Name;
                temp["student_id"] = stu.Student_Id ;
                temp["gender"] = stu.Gender ;
                temp["class"] = stu.Class ;
                temp["subject"] = stu.Subject ;
                temp["remark"] = stu.Remark ;
                temp["phone"] = stu.Phone ;
                temp["create_time"] = stu.Create_Time ;

                // 添加到结果数组
                json_stus[index++] = std::move(temp);
            }

            // 设置成功响应
            result["status"] = "success";
            result["message"] = "排序成功";
            result["students"] = std::move(json_stus);

            return crow::response(200,result);
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database orderStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database orderStudent: " << e.what();
            std::string error = e.what();
            std::string message = "排序失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std orderStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std orderStudent: " << e.what();
            std::string error = e.what();
            std::string message = "排序失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });

    // 分组排序API：/api/orderGroup/
    CROW_ROUTE(app, "/api/orderGroup/").methods("POST"_method)([&](const crow::request& req)
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
                result["message"] = "排序失败，json格式错误";
                return crow::response(400,result);
            }
            // 检查必需字段是否存在
            if(!json.has("token") || !json.has("direction") || !json.has("groups"))
            {
                result["status"] = "failed";
                result["message"] = "排序失败，json缺少token、direction、groups某个元素";

                return crow::response(400,result);
            }
            // 检查groups是否为数组格式
            if(json["groups"].t() != crow::json::type::List)
            {
                result["status"] = "failed";
                result["message"] = "排序失败，json中groups格式错误，应当为数组格式";
                return crow::response(400,result);
            }
            // 验证排序字段是否合法
            auto token_it = GroupMap.find(json["token"].s());
            if(token_it == GroupMap.end())
            {
                result["status"] = "failed";
                result["message"] = "排序失败，非法的token值";
                return crow::response(400,result);
            }
            // 验证排序方向是否合法
            auto dir_it = OrderDirectionMap.find(json["direction"].s());
            if(dir_it == OrderDirectionMap.end())
            {
                result["status"] = "failed";
                result["message"] = "排序失败，非法direction值";
                return crow::response(400,result);
            }

            // 解析分组列表数据
            std::vector<Group> order;
            for(const auto& grp : json["groups"])
            {
                Group temp;
                // 从JSON中提取分组信息，必填字段
                temp.Group_Name = grp["name"].s();
                // 可选字段，提供默认值
                temp.Description = grp.has("description") ? grp["description"].s() : std::string("");
                temp.Create_Time = grp.has("create_time") ? grp["create_time"].s() : std::string("");

                order.emplace_back(temp);
            }

            // 调用数据库排序函数
            std::vector<Group> grps = db.orderGroup(order, token_it->second, dir_it->second);
            // 构建排序后的JSON响应
            crow::json::wvalue json_grps = crow::json::wvalue::list();
            int index = 0;
            for(const auto& grp : grps)
            {
                crow::json::wvalue temp;
                // 填充分组数据到JSON对象
                temp["name"] = grp.Group_Name;
                temp["description"] = grp.Description;
                temp["create_time"] = grp.Create_Time;

                // 添加到结果数组
                json_grps[index++] = std::move(temp);
            }

            // 设置成功响应
            result["groups"] = std::move(json_grps);
            result["status"] = "success";
            result["message"] = "排序成功";
            return crow::response(200,result);

        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database orderGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database orderGroup: " << e.what();
            std::string error = e.what();
            std::string message = "排序失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std orderGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std orderGroup: " << e.what();
            std::string error = e.what();
            std::string message = "排序失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });
}
