#pragma once
#include "DataBaseManager.hpp"
#include "GroupService.hpp"
#include "SAGService.hpp"
#include "StudentService.hpp"
#include "crow/app.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include "crow/json.h"
#include "llama.h"
#include "llama_wrapper.hpp"
#include "model_pool.hpp"
#include"TokenStore.hpp"
#include <functional>
#include <unordered_set>
#include"UserService.hpp"

// 定义CORS中间件
struct CORS {
   struct context {};
   void before_handle(crow::request& req, crow::response& res, context& ctx) {}
   void after_handle(crow::request& req, crow::response& res, context& ctx) {
       res.add_header("Access-Control-Allow-Origin", "*");
       res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
       res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
   }
};

struct AuthMiddleware
{
    struct context{};
    void before_handle(crow::request& req,crow::response& res, context& ctx)
    {
        if(req.url == "/api/login/") return;
        std::string auth = req.get_header_value("Authorization");
        if (auth.size() > 7 && auth.substr(0, 7) == "Bearer ")
        {
            std::string token = auth.substr(7);
            if(TokenStore::instance().contains(token)) return;
        }
        res.code = 401;
        res.body = R"({"status":"failed","message":"未登录"})";
        res.end();
    }
   void after_handle(crow::request& req, crow::response& res, context& ctx) {};
};

/**
 * @brief API管理类，负责所有RESTful API路由的注册和管理
 *
 * 该类使用模块化方式组织路由，分为搜索、添加、删除、修改四个功能模块
 * 通过成员方法注册路由，持有app和db的引用进行数据访问
 */
class IIMAS_API
{
    private:
        crow::App<CORS,AuthMiddleware>& app;       // Crow应用对象引用（带CORS中间件）
        DataBaseManager& db;        // 数据库管理器引用
        UserService& user_ser;
        StudentService& stu_ser;
        GroupService& grp_ser;
        SAGService& SAG_ser;
        model_pool& AI_models;      // AI模型池引用
        model_set use_model;   // 当前使用的AI模型实例指针
        std::mutex model_mutex;     // 模型访问互斥锁，保证单并发会话
        // std::string temp_content;   // 临时存储用户输入内容
        void setupSearchRoutes();   // 设置搜索相关路由
        void setupAddRoutes();      // 设置添加相关路由
        void setupRemoveRoutes();   // 设置删除相关路由
        void setupUpdateRoutes();   // 设置修改相关路由
        void setupOrderRoutes();    // 设置排序相关路由
        void setupServerOperationRoutes();  // 设置服务器操作相关路由
        void setupChatAIRoutes();   // 设置AI对话相关路由
        void setupExcelRoutes();
        void setupLogRoutes();
        std::map<std::string,std::function<void(crow::json::wvalue&,const std::string&)>> chatMode;  // AI对话模式处理函数映射


    public:
        /**
         * @brief 构造函数，初始化IIMAS_API对象
         * @param app Crow应用对象引用
         * @param db 数据库管理器引用
         */
        IIMAS_API
        (
            crow::App<CORS,AuthMiddleware>& app,
            DataBaseManager& db,
            model_pool& AI_models,
            UserService& user_ser,
            StudentService& stu_ser,
            GroupService& grp_ser,
            SAGService& SAG_ser
        );

        /**
         * @brief 注册所有API路由
         *
         * 按顺序调用各个路由设置方法：setupAddRoutes → setupDeleteRoutes → setupModifyRoutes → setupSearchRoutes
         */
        void registerRoutes();

};