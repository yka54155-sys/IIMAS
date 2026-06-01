#include"std.hpp"
#include"crow.h"
#include"Crow_API.hpp"
#include"model.hpp"
#include"ApiResponse.hpp"
#include"TokenStore.hpp"
#include"StudentService.hpp"
#include"GroupService.hpp"
#include"SAGService.hpp"
#include"Excel.hpp"

void IIMAS_API::setupExcelRoutes()
{
    CROW_ROUTE(app, "/api/exportExcel/").methods("POST"_method)([&](const crow::request& req)
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
            if(!json.has("result"))
            {
                result["status"] = "failed";
                result["message"] = "json缺少result字段";
                return crow::response(400,result);
            }
          
            // 检查action字段是否为空
            if(json["result"].t() != crow::json::type::List)
            {
                result["status"] = "failed";
                result["message"] = "result字段必须是列表格式";
                return crow::response(400,result);
            }

            auto name = req.url_params.get("name");
            if(!name)
            {
                result["status"] = "failed";
                result["message"] = "缺少必要参数name（分组名）";
                return crow::response(400,result);
            }

            
            Excel_export excel(name,json);
            std::string path = excel.back();
            std::ifstream file(path,std::ios::binary);
            if(!file.is_open())
            {
                result["status"] = "failed";
                result["message"] = "Excel文件生成失败";
                return crow::response(500,result);
            }
            std::string content;
            content.assign(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());
            crow::response resp(200,content);
            resp.set_header("Content-Type", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
            resp.set_header("Content-Disposition", "attachment; filename=\"" + std::string(name) + ".xlsx\"");
            return resp;
            

            
            
        }
        catch (const SQLite::Exception& e)
        {
            // 数据库异常处理
            std::cerr << "exportExcel error in Datebase:" << e.what() << std::endl;
            CROW_LOG_ERROR << "exportExcel error in Datebase:" << e.what();
            result["status"] = "failed";
            result["message"] = "数据库错误";
            return crow::response(400,result);
        }
        catch (const OpenXLSX::XLInputError& e) 
        {
            CROW_LOG_ERROR << "OpenXLSX error: " << e.what();
             result["status"] = "failed";
            result["message"] = "Excel库错误";
            return crow::response(400,result);
        }
        catch (const OpenXLSX::XLException& e) 
        {
            CROW_LOG_ERROR << "Input error: " << e.what();
             result["status"] = "failed";
            result["message"] = "Excel库错误";
            return crow::response(400,result);
        } 
        catch (const std::exception& e)
        {
            // 标准库异常处理
            std::cerr << "exportExcel error in std:" << e.what() << std::endl;
            CROW_LOG_ERROR << "exportExcel error in std:" << e.what();
            result["status"] = "failed";
            result["message"] = "标准库错误";
            return crow::response(400,result);
        }
    });
}
