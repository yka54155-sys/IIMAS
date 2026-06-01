#pragma once
#include "crow/http_response.h"
#include "crow/json.h"
#include"std.hpp"
#include"crow.h"
#include <utility>
class ApiResponse
{   
    public:
    static crow::json::wvalue success(std::string message = "操作成功", crow::json::wvalue data = crow::json::wvalue::list({}))
    {
        crow::json::wvalue result;
        result["status"] = "success";
        result["message"]  = std::move(message);
        result["data"] = std::move(data);

        return result;
    }
    static crow::json::wvalue failure(std::string message = "操作失败", crow::json::wvalue data = crow::json::wvalue::list({}))
    {
        crow::json::wvalue result;
        result["status"] = "failure";
        result["message"] = std::move(message);
        result["data"] = std::move(data);

        return result;
    }
    

    static crow::response ok(std::string message = "操作成功", crow::json::wvalue data = crow::json::wvalue::list({}))
    {
        crow::response res(200,success(message,data));
        return res;
    }

    static crow::response badRequest(std::string message = "操作失败", crow::json::wvalue data = crow::json::wvalue::list({}))
    {
        crow::response res(400,failure(message,data));
        return res;
    }
    static crow::response unauthorized(std::string message = "登录失败", crow::json::wvalue data = crow::json::wvalue::list({}))
    {
        crow::response res(401,failure(message,data));
        return res;
    }
};