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
 * @brief 设置添加相关路由
 *
 * 注册学生添加、分组添加等API端点
 * - /api/addStudent/ - 学生添加API（POST方法）
 * - /api/addGroup/ - 分组添加API（POST方法）
 */
void IIMAS_API::setupAddRoutes()
{
    // 学生添加API：/api/addStudent/
    CROW_ROUTE(app, "/api/addStudent/").methods("POST"_method)([&](const crow::request& req)
    {

        try
        {
            auto json_stus = crow::json::load(req.body);
            if(json_stus.t() == crow::json::type::Null)
            {
                
                return ApiResponse::badRequest("添加失败,json 不可为空");
            }


            if(json_stus["students"].t() != crow::json::type::List)
            {
                return ApiResponse::badRequest("添加失败,json 缺少students列表元素");
            }
            // 初始化学生列表
            std::vector<Student> stus;
            // 遍历学生数据，验证并构建Student对象
            for(const auto& stu: json_stus["students"])
            {
                Student temp;
                // 验证必填字段是否存在
                if(!stu.has("student_id") || !stu.has("name"))
                {
                    
                    return ApiResponse::badRequest("添加失败,json 缺少必要因素：name,student_id");
                }
                // 提取学生信息，可选字段使用默认值
                temp.Student_Id = stu["student_id"].s();
                temp.Name = stu["name"].s();
                temp.Class = stu.has("class") ? stu["class"].i() : 0;
                temp.Gender = stu.has("gender") ? stu["gender"].s() : std::string("");
                temp.Phone = stu.has("phone") ? stu["phone"].s() : std::string("");
                temp.Subject = stu.has("subject") ? stu["subject"].s() : std::string("");
                temp.Remark = stu.has("remark") ? stu["remark"].s() : std::string("");

                // 添加到学生列表
                stus.emplace_back(temp);
            }
            auto searchReturn = stu_ser.addStudent(stus);
            switch(searchReturn.status)
            {
                case StudentResult::Status::SUCCESS:
                    return ApiResponse::ok("添加成功");
                case StudentResult::Status::FAILURE:
                    return ApiResponse::badRequest("添加失败： " + searchReturn.message);
                case StudentResult::Status::INVALID_INPUT:
                    return ApiResponse::badRequest("添加失败，请检查字段格式： " + searchReturn.message);
                default:
                    return ApiResponse::badRequest("添加失败,异常: " + searchReturn.message);
            }
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database addStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database addStudent: " << e.what();
            std::string error = e.what();
            return ApiResponse::badRequest("添加失败，数据库部分出错: " + error);
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std addStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std addStudent: " << e.what();
            std::string error = e.what();
            return ApiResponse::badRequest("添加失败，标准库部分出错" + error);
        }
    });

    // 分组添加API：/api/addGroup/
    CROW_ROUTE(app, "/api/addGroup/").methods("POST"_method)([&](const crow::request& req)
    {
        try
        {
            auto json = crow::json::load(req.body);

            if(json.t() == crow::json::type::Null)
            {
                return ApiResponse::badRequest("添加失败，json不可为空");
            }

            if(!json.has("groups"))
            {
                return ApiResponse::badRequest("添加失败，json缺少groups字段");
            }

            if(json["groups"].t() != crow::json::type::List)
            {
                return ApiResponse::badRequest("添加失败，groups格式错误，应当为数组格式");
            }

            std::vector<Group> grps;

            for(const auto& grp : json["groups"])
            {
                Group temp;

                if(!grp.has("name"))
                {
                    return ApiResponse::badRequest("添加失败，groups数组中缺少必要的name字段");
                }

                temp.Group_Name = grp["name"].s();
                temp.Description = grp.has("description") ? grp["description"].s() : std::string{};

                grps.emplace_back(temp);
            }

            for(const auto& g : grps)
            {
                auto ret = grp_ser.addGroup(g);
                if(ret.status != GroupResult::Status::SUCCESS)
                {
                    return ApiResponse::badRequest("添加失败：" + ret.message);
                }
            }

            return ApiResponse::ok("添加成功");
        }
        catch(const SQLite::Exception& e)
        {
            std::cerr << "API Error in Database addGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database addGroup: " << e.what();
            return ApiResponse::badRequest("添加失败，数据库部分出错: " + std::string(e.what()));
        }
        catch (const std::exception& e)
        {
            std::cerr << "API Error in std addGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std addGroup: " << e.what();
            return ApiResponse::badRequest("添加失败，标准库部分出错" + std::string(e.what()));
        }
    });

    // 从学生总表添加学生到分组API：/api/addSAG/fromAll/
    CROW_ROUTE(app, "/api/addSAG/fromAll/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;
        try
        {
            // 解析请求体中的JSON数据
            auto json = crow::json::load(req.body);

            // 验证JSON格式是否正确
            if(json.t() == crow::json::type::Null)
            {
                result["status"] = "failed";
                result["message"] = "添加失败,json格式错误，不能为空";

                return crow::response(400,result);
            }

            // 验证必需字段是否存在
            if(!json.has("students") || !json.has("group_name"))
            {
                result["status"] = "failed";
                result["message"] = "添加失败,json内容错误，缺少students或group_name某个元素";

                return crow::response(400,result);
            }

            // 验证分组是否存在
            size_t Exist = db.group_exist(json["group_name"].s());
            switch (Exist)
            {
                case EXIST_YES:
                    break;
                case EXIST_NO:
                    result["status"] = "failed";
                    result["message"] = "添加失败,json内容错误，group_name不存在";

                    return crow::response(400,result);
                    break;
                case EXIST_ERROR:
                    result["status"] = "failed";
                    result["message"] = "添加失败,检查group_name时报错";

                    return crow::response(400,result);
                    break;
                default:
                    result["status"] = "failed";
                    result["message"] = "添加失败,group_exist函数返回未知值？";

                    return crow::response(400,result);
            }

            // 验证students字段是否为数组格式
            if(json["students"].t() != crow::json::type::List)
            {
                result["status"] = "failed";
                result["message"] = "添加失败,json格式错误，students内容应为数组格式";

                return crow::response(400,result);
            }

            // 初始化学生列表
            std::vector<Student> stus;

            // 遍历学生数据，验证并构建Student对象
            for(const auto& stu : json["students"])
            {
                Student temp;

                // 验证必填字段是否存在
                if(!stu.has("name") || !stu.has("student_id"))
                {
                    result["status"] = "failed";
                    result["message"] = "添加失败,json格式错误，某个student缺少必要因素student_id或name";

                    return crow::response(400,result);
                }

                // 提取学生基本信息
                temp.Name = stu["name"].s();
                temp.Student_Id = stu["student_id"].s();

                // 验证必填字段是否为空
                if(temp.Name.empty() || temp.Student_Id.empty())
                {
                    result["status"] = "failed";
                    result["message"] = "添加失败,json格式错误，students中name 和 student_id不可为空";

                    return crow::response(400,result);
                }

                // 获取学生在数据库表中的ID
                ID_RETURN id;
                id = db.get_stu_table_id(temp.Student_Id);
                temp.Id = id.id;

                // 验证学生是否存在
                if(temp.Id == 0)
                {
                    result["status"] = "failed";
                    result["message"] = "添加失败,json格式错误，不存在的学号（student_id）,请尝试new方法添加";

                    return crow::response(400,result);
                }

                // 提取可选字段，使用默认值
                temp.Class = stu.has("class") ? stu["class"].i() : 0;
                temp.Gender = stu.has("gender") ? stu["gender"].s() : std::string("");
                temp.Subject = stu.has("subject") ? stu["subject"].s() : std::string("");
                temp.Create_Time = stu.has("create_time") ? stu["create_time"].s() : std::string("");
                temp.Phone = stu.has("phone") ? stu["phone"].s() : std::string("");
                temp.Remark = stu.has("remark") ? stu["remark"].s() : std::string("");

                // 添加到学生列表
                stus.emplace_back(temp);
            }

            // 获取目标分组ID
            int grp_id = 0;
            std::string group_name = json["group_name"].s();
            ID_RETURN g_id = db.get_grp_table_id(group_name);
            grp_id = g_id.id;

            // 验证分组ID是否有效
            if(grp_id == 0)
            {
                result["status"] = "failed";
                result["message"] = "添加失败,json格式错误，不存在的分组，请检查分组名称";

                return crow::response(400,result);
            }

            // 调用数据库添加函数，将学生关联到分组
            if(!db.addStudent(stus,grp_id))
            {
                result["status"] = "failed";
                result["message"] = "添加失败,关联函数报错";

                return crow::response(400,result);
            }

            // 设置成功响应
            result["status"] = "success";
            result["message"] = "添加成功，学生已关联分组";

            return crow::response(200,result);

        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database addSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database addSAG: " << e.what();
            std::string error = e.what();
            std::string message = "添加失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std addSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std addSAG: " << e.what();
            std::string error = e.what();
            std::string message = "添加失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }

    });
    // 新添加学生并关联到指定分组（先创建学生，再添加到分组）
    CROW_ROUTE(app, "/api/addSAG/new/").methods("POST"_method)([&](const crow::request& req)
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
                result["message"] = "添加失败,json格式错误，不能为空";

                return crow::response(400,result);
            }
            // 检查必需字段是否存在
            if(!json.has("students") || !json.has("group_name"))
            {
                result["status"] = "failed";
                result["message"] = "添加失败,json内容错误，缺少students或group_name某个元素";

                return crow::response(400,result);
            }
            // 检查分组是否存在
            size_t Exist = db.group_exist(json["group_name"].s());
            switch (Exist)
            {
                case EXIST_YES:break;
                case EXIST_NO:
                    result["status"] = "failed";
                    result["message"] = "添加失败,json内容错误，group_name不存在";

                    return crow::response(400,result);
                    break;
                case EXIST_ERROR:
                    result["status"] = "failed";
                    result["message"] = "添加失败,检查group_name时报错";

                    return crow::response(400,result);
                    break;
                default:
                    result["status"] = "failed";
                    result["message"] = "添加失败,group_exist函数返回未知值？";

                    return crow::response(400,result);

            }
            // 检查students是否为数组格式
            if(json["students"].t() != crow::json::type::List)
            {
                result["status"] = "failed";
                result["message"] = "添加失败,json格式错误，students内容应为数组格式";

                return crow::response(400,result);
            }
            // 初始化学生列表
            std::vector<Student> stus;
            // 遍历students数组，验证并构建Student对象
            for(const auto& json_stu : json["students"])
            {
                Student temp;
                // 检查必填字段是否存在
                if(!json_stu.has("student_id") || !json_stu.has("name"))
                {
                    result["status"] = "failed";
                    result["message"] = "添加失败,json格式错误，students内缺少必要字段name 或 student_id";

                    return crow::response(400,result);
                }
                // 提取姓名和学号
                temp.Name = json_stu["name"].s();
                temp.Student_Id = json_stu["student_id"].s();
                // 检查姓名和学号是否为空
                if(temp.Name.empty() || temp.Student_Id.empty())
                {
                    result["status"] = "failed";
                    result["message"] = "添加失败,json格式错误，name 和 student_id不可为空";

                    return crow::response(400,result);
                }
                // 检查学号是否已存在
                if(db.student_exist(temp.Student_Id) == EXIST_YES)
                {

                    result["status"] = "failed";
                    result["message"] = "添加失败,json格式错误，student_id已存在，请尝试fromAll方法";

                    return crow::response(400,result);
                }
                // 提取可选字段，不存在则使用默认值
                temp.Class = json_stu.has("class") ? json_stu["class"].i() : 0;
                temp.Subject = json_stu.has("subject") ? json_stu["subject"].s() : std::string("");
                temp.Phone = json_stu.has("phone") ? json_stu["phone"].s() : std::string("");
                temp.Remark = json_stu.has("remark") ? json_stu["remark"].s() : std::string("");
                temp.Gender = json_stu.has("gender") ? json_stu["gender"].s() : std::string("");
                temp.Create_Time = json_stu.has("create_time") ? json_stu["create_time"].s() : std::string("");

                // 添加到学生列表
                stus.emplace_back(temp);

            }
            // 向students表插入新学生
            if(!db.addStudent(stus))
            {
                result["status"] = "failed";
                result["message"] = "插入失败，addStudent()出错";
                return crow::response(400,result);
            }
            // 获取刚插入的学生的数据库ID
            for(auto& stu : stus)
            {
                ID_RETURN id = db.get_stu_table_id(stu.Student_Id);
                if(id.status == ID_OK)
                {
                    stu.Id = id.id;
                }
            }
            // 获取分组名称
            std::string group_name= json["group_name"].s();
            // 获取分组的数据库ID
            ID_RETURN id = db.get_grp_table_id(group_name);
            if(id.status == ID_ERROR)
                {

                    result["status"] = "failed";
                    result["message"] = "添加失败,获取分组id时失败";

                    return crow::response(400,result);
            }

            // 向student_group表插入关联记录
            db.addStudent(stus,id.id);

            // 返回成功结果
            result["status"] = "success";
            result["message"] = "添加成功";

            return crow::response(200,result);


        }
        // 捕获数据库异常
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database addSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database addSAG: " << e.what();
            std::string error = e.what();
            std::string message = "添加失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        // 捕获标准库异常
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std addSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std addSAG: " << e.what();
            std::string error = e.what();
            std::string message = "添加失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });
}
