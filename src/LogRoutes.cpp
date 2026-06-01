#include"std.hpp"
#include"crow.h"
#include"Crow_API.hpp"
#include"model.hpp"
#include"ApiResponse.hpp"
#include"TokenStore.hpp"
#include"UserService.hpp"
void IIMAS_API::setupLogRoutes()
{
    CROW_ROUTE(app, "/api/login/").methods("POST"_method)([&](const crow::request& req)
    {
        
        try
        {
            auto json = crow::json::load(req.body);
            if(json.t() == crow::json::type::Null||!json.has("username") || !json.has("password"))
            {
                
                    return ApiResponse::badRequest("json缺少必要字段",{});
            }
            if(json["username"].t() == crow::json::type::Null || json["password"].t() == crow::json::type::Null)
            {
                return ApiResponse::badRequest("用户名或密码为空",{});
            }

            std::string username = json["username"].s();
            std::string password = json["password"].s();
            
            if(user_ser.authenticateUser(username, password).status != UserResult::UserStatus::SUCCESS)
            {
                
                return ApiResponse::unauthorized("用户名或密码错误");
            }
            else
            {
                std::string token = TokenStore::instance().generate_token();
                TokenStore::instance().insert(token);
                crow::json::wvalue result;
                result["status"] = "success";
                result["message"] = "登录成功！";
                result["token"] = std::move(token);
                return crow::response(200,result);
            }
        }
        catch (const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "login error in Datebase:" << e.what() << std::endl;
            CROW_LOG_ERROR << "login error in Datebase:" << e.what();
            return ApiResponse::badRequest("登录异常，数据库错误",{});
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "login error in std:" << e.what() << std::endl;
            CROW_LOG_ERROR << "login error in std:" << e.what();
            return ApiResponse::badRequest("登录异常，标准库错误");
        }

    });

    CROW_ROUTE(app, "/api/logout/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;
        try
        {
            std::string auth = req.get_header_value("Authorization");
            std::string token;
            if (auth.size() > 7 && auth.substr(0, 7) == "Bearer ")
                token = auth.substr(7);

            if (!TokenStore::instance().empty() && TokenStore::instance().contains(token))
            {
                TokenStore::instance().erase(token);
                return ApiResponse::ok("退出成功");
            }
            else
            {
               return ApiResponse::badRequest("退出失败");
            }
        }
        catch (const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "logout error in Datebase:" << e.what() << std::endl;
            CROW_LOG_ERROR << "logout error in Datebase:" << e.what();
            return ApiResponse::badRequest("退出异常,数据库错误");
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "logout error in std:" << e.what() << std::endl;
            CROW_LOG_ERROR << "logout error in std:" << e.what();
            return ApiResponse::badRequest("退出异常，标准库错误");
        }
        

    });
}