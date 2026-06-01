// SQLite异常处理
#include "AI_params.hpp"
#include "DataBaseManager.hpp"
#include "SQLiteCpp/Exception.h"
// Crow框架核心组件
#include "UserService.hpp"
#include "crow/app.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include "crow/json.h"
#include "crow/logging.h"
// 数据模型和类型定义
#include "model.hpp"
// 标准库和通用工具
#include "model_pool.hpp"
#include"std.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include<functional>

// API类定义
#include"Crow_API.hpp"
#include"ToolAgent.hpp"
#include"ApiResponse.hpp"
//Excel
#include"Excel.hpp"

//Token
#include"TokenStore.hpp"
/**
 * @brief 构造函数，使用成员初始化列表初始化app和db引用
 * @param app Crow应用对象引用
 * @param db 数据库管理器引用
 */
IIMAS_API::IIMAS_API
(
    crow::App<CORS,AuthMiddleware>& app,
    DataBaseManager& db,
    model_pool& AI_models,
    UserService& user_ser,
    StudentService& stu_ser,
    GroupService& grp_ser,
    SAGService& SAG_ser
): app(app) , db (db) , AI_models(AI_models) , user_ser(user_ser) ,stu_ser(stu_ser),grp_ser(grp_ser),SAG_ser(SAG_ser)
{
    
    use_model = {nullptr,nullptr,nullptr};
    chatMode["start"] = std::function<void(crow::json::wvalue&,const std::string&)>([&](crow::json::wvalue& result,const std::string& temp_content)
    {
        std::lock_guard<std::mutex> lock(model_mutex);
        if(use_model.exist())
        {
            result["status"] = "failed";
            result["message"] = "初始化失败，模型已初始化";
            return;
        }
        use_model =  AI_models.acquire();
        if(!use_model.exist())
        {
            result["status"] = "failed";
            result["message"] = "初始化失败，模型池已满，请稍后重试";
            return;
        }
        else
        {
            // use_model.intent_model->add_message("system", AI_params::IntentParams::prompt);
            // use_model.query_model->add_message("system", AI_params::QueryParams::prompt);
            // use_model.summary_model->add_message("system", AI_params::SummaryParams::prompt);
            result["status"] = "success";
            result["message"] = "模型初始化成功！";
            return;
        }

    });
    chatMode["end"] = std::function<void(crow::json::wvalue&,const std::string&)>([&](crow::json::wvalue& result,const std::string& temp_content)
    {
        std::lock_guard<std::mutex> lock(model_mutex);
        if(!use_model.exist())
        {
            result["status"] = "failed";
            result["message"] = "释放模型失败，模型未初始化";
            return;
        }
        if(AI_models.release(use_model))
        {
            use_model.intent_model = nullptr;
            use_model.query_model = nullptr;
            use_model.summary_model = nullptr;
            result["status"] = "success";
            result["message"] = "释放模型成功！";
            return;
        }
        else
        {
            result["status"] = "failed";
            result["message"] = "模型释放失败，未知原因！";
            return;
        }
        
        
    });
    chatMode["chat"] = std::function<void(crow::json::wvalue&,const std::string&)>([&](crow::json::wvalue& result ,const std::string& temp_content)
    {
        
        model_set temp_model_set;
        {
            std::lock_guard<std::mutex> lock(model_mutex);
            if(!use_model.exist())
            {
                result["status"] = "failed";
                result["message"] = "模型未初始化，请先初始化后使用";
                return;
            }
            temp_model_set = use_model;

        }
        ToolAgent agent(db,temp_model_set);
        std::string answer = agent.run(temp_content);

        auto temp_json = crow::json::load(answer);
        if(temp_json.t() == crow::json::type::Null)
        {
            result["status"] = "failed";
            result["message"] = "ToolAgent.run()返回值错误";
            return ;
        }
        result = std::move(temp_json);
        
    });
};

/**
 * @brief 注册所有API路由
 * 
 * 按顺序调用各个路由设置方法，确保路由正确注册到Crow应用中
 */
void IIMAS_API::registerRoutes()
{
    // 注册添加相关路由
    setupAddRoutes();
    // 注册删除相关路由
    setupRemoveRoutes();
    // 注册修改相关路由
    setupUpdateRoutes();
    // 注册搜索相关路由
    setupSearchRoutes();
    // 注册排序相关路由
    setupOrderRoutes();
    //服务器操作路由
    setupServerOperationRoutes();
    //AI模块路由
    setupChatAIRoutes();

    setupExcelRoutes();

    setupLogRoutes();

}









/**
 * @brief 设置服务器操作相关路由
 *
 * 注册服务器管理API端点，如关闭服务器等
 */
void IIMAS_API::setupServerOperationRoutes()
{
    CROW_ROUTE(app, "/api/shutdownServer/").methods("POST"_method)([&](const crow::request& req)
    {
        crow::json::wvalue result;

        try
        {
            // 从URL参数获取密码
            auto password_param = req.url_params.get("password");
            std::string password = password_param ? password_param : "";

            // 验证密码
            const std::string correct_password = "iimas";
            if (password != correct_password)
            {
                CROW_LOG_WARNING << "关闭服务器失败：密码错误";
                result["status"] = "failed";
                result["message"] = "密码错误，拒绝关闭服务器";
                return crow::response(401, result);
            }

            // 密码正确，返回成功响应
            result["status"] = "success";
            result["message"] = "密码验证通过，服务器将在3秒后关闭";
            crow::response response(200, result);

            // 延迟关闭服务器，确保响应发送成功
            std::thread([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                CROW_LOG_WARNING << "正在关闭服务器！";
                app.stop();
                CROW_LOG_WARNING << "服务器关闭成功。";
            }).detach();

            return response;
        }
        catch(const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "API Error in Database shutdownServer: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in Database shutdownServer: " << e.what();
            std::string error = e.what();
            std::string message = "服务器关闭失败，数据库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;

            return crow::response(400,result);
        }
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "API Error in std shutdownServer: " << e.what() << std::endl;
            CROW_LOG_ERROR << "API Error in std shutdownServer: " << e.what();
            std::string error = e.what();
            std::string message = "服务器关闭失败，标准库部分出错" + error;
            result["status"] = "failed";
            result["message"] = message;
            return crow::response(400,result);
        }
    });
}




