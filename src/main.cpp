#include "Crow_API.hpp"
#include "GroupRepository.hpp"
#include "GroupService.hpp"
#include "SAGRepository.hpp"
#include "SAGService.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include "StudentService.hpp"
#include "UserRepository.hpp"
#include "crow.h"
#include"DataBaseManager.hpp"
#include "crow/http_response.h"
#include "crow/logging.h"
#include "headers/XLDocument.hpp"
#include "model.hpp"
#include"std.hpp"
#include <iostream>
#include <string>
#include"llama_wrapper.hpp"
#include"AI_params.hpp"
#include"model_pool.hpp"
#include"Excel.hpp"
#include"TokenStore.hpp"
#include"UserService.hpp"
#include"StudentRepository.hpp"
#ifdef _WIN32
//Test
#include <wincon.h>
#include<windows.h>

#endif



/**
 * @brief 主函数，IIMAS服务器入口
 *
 * 初始化数据库、创建Crow应用、注册API路由并启动服务器
 */
int main(int argc , char** argv)
{
   #ifdef _WIN32
   SetConsoleCP(65001);
   SetConsoleOutputCP(65001);
   #endif
   #ifdef __AVX2__
      std::cerr << "AVX2 supported" << std::endl;
   #endif
   #ifdef __AVX512F__
      std::cerr << "AVX512F supported" << std::endl;
   #endif
   try
   {


      TokenStore::instance();
      


      SQLite::Database db_("IIMAS.db",SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
      //初始化Repository
      StudentRepository stuRepo(db_);
      GroupRepository grpRepo(db_);
      SAGRepository SAGRepo(db_);
      UserRepository userRepo(db_);

      //初始化Service
      UserService user_ser(userRepo);
      StudentService stu_ser(stuRepo);
      GroupService grp_ser(grpRepo);
      SAGService SAG_ser(SAGRepo,stuRepo,grpRepo);

      // 初始化数据库管理器
      DataBaseManager db;

      // 初始化模型池
      model_pool AI_models;

      // 创建Crow应用对象并启用CORS中间件
      crow::App<CORS,AuthMiddleware> IIMAS;

      // 创建API管理器并注册所有路由
      IIMAS_API api(IIMAS,db,AI_models,user_ser,stu_ser,grp_ser,SAG_ser);
      api.registerRoutes();

      IIMAS.loglevel(crow::LogLevel::INFO);
      if(argc != 1 && std::string(argv[1]) == "debug")
      {
         IIMAS.loglevel(crow::LogLevel::DEBUG);
      }

      // 启动服务器，监听3002端口，使用多线程
      IIMAS.port(3002).multithreaded().run();
      TokenStore::instance().clear();
   }
   catch(const std::exception& e)
   {
      std::cerr << "[IIMAS] 启动失败: " << e.what() << std::endl;
      TokenStore::instance().clear();
   }
   catch(...)
   {
      std::cerr << "[IIMAS] 启动失败: 未知错误" << std::endl;
      TokenStore::instance().clear();
   }

   #ifdef _WIN32
   system("pause");
   #endif

   return 0;
}