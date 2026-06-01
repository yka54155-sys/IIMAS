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
 * @brief 设置AI对话相关路由
 *
 * 注册AI聊天API端点，支持对话初始化、聊天、结束对话等操作
 */
void IIMAS_API::setupChatAIRoutes()
{
    // AI对话API：/api/chatAI/
    // action字段控制操作模式：start（初始化）、chat（对话）、end（释放）
    CROW_ROUTE(app, "/api/chatAI/").methods("POST"_method)([&](const crow::request& req)
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
                result["message"] = "输入错误，json不可为空";
                return crow::response(400,result);
            }
            // 检查action字段是否存在
            if(!json.has("action"))
            {
                result["status"] = "failed";
                result["message"] = "json缺少action字段";
                return crow::response(400,result);
            }
          
            // 检查action字段是否为空
            if(json["action"].t() == crow::json::type::Null)
            {
                result["status"] = "failed";
                result["message"] = "action字段不可为空";
                return crow::response(400,result);
            }

            std::string action = json["action"].s();
            auto it = chatMode.find(action);
            // chat模式需要额外读取content字段并存入 temp_content

            std::string temp_content;
            if(action == "chat")
            {
                if(!json.has("content") || json["content"].t() == crow::json::type::Null)
                {
                    result["status"] = "failed";
                    result["message"] = "content字段不可为空";
                    return crow::response(400,result);
                }
                else
                {
                    // 将用户输入内容暂存，供 chatMode["chat"] 内的 lambda 使用
                    temp_content = json["content"].s();
                }
            }
            // 查找对应的处理函数并执行
            if(it != chatMode.end())
            {
                it->second(result,temp_content);
                
                
            }
            else
            {
                result["status"] = "failed";
                result["message"] = "未知的action值";

            }
            
            
            return crow::response(200,result);
        }
        catch (const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "chatAI error in Datebase:" << e.what() << std::endl;
            CROW_LOG_ERROR << "chatAI error in Datebase:" << e.what();
            result["status"] = "failed";
            result["message"] = "数据库错误";
            return crow::response(400,result);
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "chatAI error in std:" << e.what() << std::endl;
            CROW_LOG_ERROR << "chatAI error in std:" << e.what();
            result["status"] = "failed";
            result["message"] = "标准库错误";
            return crow::response(400,result);
        }


    });
}