#include "crow/json.h"
#include "crow/logging.h"
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
 * @brief 设置搜索相关路由
 * 
 * 注册学生精确搜索、学生模糊搜索、分组搜索等API端点
 */

void IIMAS_API::setupSearchRoutes()
{
    // 学生精确搜索API：/api/exactSearchStudent/
    CROW_ROUTE(app,"/api/exactSearchStudent/").methods("GET"_method)([&](const crow::request& req)
    {
        crow::json::wvalue::list result;
        try
        {
            // 获取URL参数
            auto token = req.url_params.get("token");
            auto value = req.url_params.get("value");
            
            // 参数验证
            if(!value || !token)
            {
                return ApiResponse::badRequest("搜索失败，value/token不可为空");
            }

            StudentQuery query{.token = token,.value = value,.mode = StudentQuery::SearchMode::EXACT};
            auto searchResult = stu_ser.searchStudent(query);
            using Status = StudentResult::Status;
            switch(searchResult.status)
            {
                case Status::SUCCESS:
                    for(const auto& stu : searchResult.data)
                    {
                        crow::json::wvalue temp;
                        temp["name"] = stu.Name;
                        temp["class"] = stu.Class;
                        temp["gender"] = stu.Gender;
                        temp["ID"] = stu.Id;
                        temp["phone"] = stu.Phone;
                        temp["remark"] = stu.Remark;
                        temp["student_id"] = stu.Student_Id;
                        temp["subject"] = stu.Subject;
                        temp["create_time"] = stu.Create_Time;

                        result.push_back(temp);
                    }
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::ok("搜索成功",result);
                    
                case Status::FAILURE:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::ok("搜索结果为空");
                    
                case Status::INVALID_INPUT:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::badRequest("搜索失败，无效输入: " + searchResult.message);
                    
                default:
                    CROW_LOG_ERROR << searchResult.message;
                    return ApiResponse::badRequest("搜索失败，异常: " + searchResult.message);
                    
            }
            
           
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database exactSearchStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database exactSearchStudent: " << e.what();
       
            return ApiResponse::badRequest("搜索失败" );
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std exactSearchStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std exactSearchStudent: " << e.what();
            
            return ApiResponse::badRequest("搜索失败" );
        }

    });

    // 学生模糊搜索API：/api/fuzzySearchStudent/
    CROW_ROUTE(app, "/api/fuzzySearchStudent/").methods("GET"_method)([&](const crow::request& req)
    {
        crow::json::wvalue::list result;
        try
        {
            // 获取URL参数
            auto token = req.url_params.get("token");
            auto value = req.url_params.get("value");
            
            // 参数验证
            if(!value || !token)
            {
                return ApiResponse::badRequest("搜索失败，value/token不可为空");
            }

            StudentQuery query{.token = token,.value = value,.mode = StudentQuery::SearchMode::FUZZY};
            auto searchResult = stu_ser.searchStudent(query);
            using Status = StudentResult::Status;
            switch(searchResult.status)
            {
                case Status::SUCCESS:
                    for(const auto& stu : searchResult.data)
                    {
                        crow::json::wvalue temp;
                        temp["name"] = stu.Name;
                        temp["class"] = stu.Class;
                        temp["gender"] = stu.Gender;
                        temp["ID"] = stu.Id;
                        temp["phone"] = stu.Phone;
                        temp["remark"] = stu.Remark;
                        temp["student_id"] = stu.Student_Id;
                        temp["subject"] = stu.Subject;
                        temp["create_time"] = stu.Create_Time;

                        result.push_back(temp);
                    }
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::ok("搜索成功",result);
                    
                case Status::FAILURE:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::ok("搜索结果为空");
                    
                case Status::INVALID_INPUT:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::badRequest("搜索失败，无效输入: " + searchResult.message);
                    
                default:
                    CROW_LOG_ERROR << searchResult.message;
                    return ApiResponse::badRequest("搜索失败，异常: " + searchResult.message);
                    
            }
            
           
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database fuzzySearchStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database fuzzySearchStudent: " << e.what();
       
            return ApiResponse::badRequest("搜索失败" );
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std fuzzySearchStudent: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std fuzzySearchStudent: " << e.what();
            
            return ApiResponse::badRequest("搜索失败" );
        }
    });

    // 分组搜索API：/api/searchGroup/（先精确搜索，无结果则模糊搜索）
    CROW_ROUTE(app, "/api/searchGroup/").methods("GET"_method)([&](const crow::request& req)
    {
        crow::json::wvalue::list result;
        try
        {
            // 获取URL参数
            auto token = req.url_params.get("token");
            auto value = req.url_params.get("value");

            // 参数验证
            if(!token || !value)
            {
                return ApiResponse::badRequest("搜索失败，value/token不可为空");
            }

            auto searchFunc = [&](GroupQuery::SearchMode mode)
            {
                GroupQuery query{.token = token,.value = value,.mode = mode};
                return grp_ser.searchGroup(query);
            };
            GroupResult searchResult = searchFunc(GroupQuery::SearchMode::EXACT);
            
            using Status = GroupResult::Status;
            if(searchResult.status == Status::FAILURE)
            {
                searchResult = searchFunc(GroupQuery::SearchMode::FUZZY);
            }

            switch (searchResult.status)
            {
                case Status::SUCCESS:
                    for(const auto& grp : searchResult.data)
                    {
                        crow::json::wvalue temp;
                        temp["name"] = grp.Group_Name;
                        temp["create_time"] = grp.Create_Time;
                        temp["description"] = grp.Description;
                        temp["ID"] = grp.Id;
                        result.push_back(temp);
                    }
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::ok("搜索成功",result);
                case Status::FAILURE:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::ok("搜索结果为空");
                case Status::INVALID_INPUT:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::badRequest("搜索失败，无效输入： " + searchResult.message);
                
                default:
                    CROW_LOG_INFO << searchResult.message;
                    return ApiResponse::badRequest("搜索失败，异常： " + searchResult.message);
                    
            }
            
            
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database searchGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database searchGroup: " << e.what();
           
            return ApiResponse::badRequest("搜索失败");
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std searchGroup: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std searchGroup: " << e.what();
           
            return ApiResponse::badRequest("搜索失败");
        }
    });

    // 学生-分组关联搜索API：/api/searchSAG/
    // 功能：按分组名搜索组内学生，或按学号搜索学生所在分组
    // 参数：token ("group"|"student"), value (分组名|学号)
    
    CROW_ROUTE(app, "/api/searchSAG/").methods("GET"_method)([&](const crow::request& req)
    {
        // 创建JSON响应对象
        crow::json::wvalue::list result;
        try
        {
            // 获取URL参数：token和value
            auto token = req.url_params.get("token");
            auto value = req.url_params.get("value");
            // 参数验证：检查参数是否为空
            if(!token || !value)
            {
                // result["status"] = "failed";
                // result["message"] = "搜索失败，缺少必要参数: value 和 token";

                // return crow::response(400,result);
                return ApiResponse::badRequest("搜索失败，value/token不可为空");
            }
           
            SAGResult searchReturn = SAG_ser.searchSAG({token,value});
            switch (searchReturn.status) 
            {
                case SAGResult::Status::SUCCESS:
                    for(const auto& d : searchReturn.data)
                    {
                        crow::json::wvalue temp;
                        temp["name"] = d.Name;
                        temp["class"] = d.Class;
                        temp["create_time"] = d.Create_time;
                        temp["description"] = d.Description;
                        temp["gender"] = d.Gender;
                        temp["id"] = d.ID;
                        temp["phone"] = d.Phone;
                        temp["remark"] = d.Remark;
                        temp["student"] = d.Student_Id;
                        temp["subject"] = d.Subject;

                        result.push_back(temp);
                    }
                    return ApiResponse::ok("搜索成功",result);

                case SAGResult::Status::FAILURE:
                    return ApiResponse::badRequest("搜索失败，没有符合条件的结果： " + searchReturn.message);
                case SAGResult::Status::INVALID_INPUT:
                    return ApiResponse::badRequest("搜索失败，非法的token值： " + searchReturn.message);
                default:
                    return ApiResponse::badRequest("搜索失败，异常： " + searchReturn.message);
            }

          

        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database searchSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database searchSAG: " << e.what();
           
            return ApiResponse::badRequest("搜索异常");
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std searchSAG: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std searchSAG: " << e.what();
       
            return ApiResponse::badRequest("搜索异常");
        }
    });
}