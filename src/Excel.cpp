#include"Excel.hpp"
#include "crow/json.h"
#include "headers/XLDocument.hpp"
#include"model.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#include <variant>

Excel_export::Excel_export(std::string_view name,const crow::json::rvalue& json)
{
    // 确保 xlsx 目录存在
    std::filesystem::create_directories("xlsx");

    real_name = "xlsx/" + std::string(name) + ".xlsx";
    doc.create(real_name,OpenXLSX::XLForceOverwrite);

    auto wks = doc.workbook().worksheet("Sheet1");

    std::set<std::string> cell_range= {"A","B","C","D","E","F","G"};
    wks.cell("A1").value() = "学号";
    wks.cell("B1").value() = "姓名";
    wks.cell("C1").value() = "班级";
    wks.cell("D1").value() = "专业";
    wks.cell("E1").value() = "性别";
    wks.cell("F1").value() = "手机号";
    wks.cell("G1").value() = "备注";

    if(!json.has("result") || json["result"].t() != crow::json::type::List)
    {
        throw std::runtime_error("json error no result");
    }
    int col = 2;

    // 安全获取字符串字段的 lambda
    auto getString = [](const crow::json::rvalue& item, const char* key) -> std::string {
        if(!item.has(key)) return "";
        auto t = item[key].t();
        if(t == crow::json::type::String)
            return item[key].s();
        if(t == crow::json::type::Number)
            return std::to_string(item[key].i());
        return "";
    };

    // 安全获取整数字段的 lambda
    auto getInt = [](const crow::json::rvalue& item, const char* key) -> int {
        if(!item.has(key)) return 0;
        auto t = item[key].t();
        if(t == crow::json::type::Number)
            return item[key].i();
        if(t == crow::json::type::String)
        {
            try { return std::stoi(item[key].s()); }
            catch(...) { return 0; }
        }
        return 0;
    };

    std::vector<Student> stus;
    for(int i = 0;i < json["result"].size();i++)
    {
        const auto& item = json["result"][i];

        Student temp;
        temp.Name = getString(item, "name");
        temp.Student_Id = getString(item, "student_id");
        temp.Gender = getString(item, "gender");
        temp.Class = getInt(item, "class");
        temp.Subject = getString(item, "subject");
        temp.Phone = getString(item, "phone");
        temp.Remark = getString(item, "remark");

        stus.emplace_back(temp);
    }

    auto c_it = cell_range.begin();
    auto add_cell = [&](std::string content)
    {
        std::string cell_num = *c_it + std::to_string(col);

        wks.cell(cell_num).value() = content;

        c_it++;
        if(c_it == cell_range.end())
        {
            c_it = cell_range.begin();
        }
    };
    for(auto it = stus.begin();it != stus.end();it++)
    {
        add_cell(it->Student_Id);
        add_cell(it->Name);
        add_cell(std::to_string(it->Class));
        add_cell(it->Subject);
        add_cell(it->Gender);
        add_cell(it->Phone);
        add_cell(it->Remark);
        
        
        col++;
    }



    doc.save();
    doc.close();

}

std::string Excel_export::back()
{
    return real_name;
}

Excel_export::~Excel_export()
{
    std::filesystem::remove(real_name);
    std::remove(real_name.c_str());
}