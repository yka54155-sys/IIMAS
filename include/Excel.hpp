#pragma once
#include "OpenXLSX.hpp"
#include "crow.h"
#include "crow/json.h"
#include "std.hpp"
#include <string>

class Excel_export
{
    private:
    OpenXLSX::XLDocument doc;
    std::string real_name;
    public:
    Excel_export(std::string_view name, const crow::json::rvalue& json);
    std::string back();

    ~Excel_export();

    Excel_export(const Excel_export&) = delete;
    Excel_export& operator=(const Excel_export&) = delete;
    Excel_export(Excel_export&&) = delete;
    Excel_export& operator=(Excel_export&&) = delete;
};
