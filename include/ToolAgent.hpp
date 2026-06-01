#pragma once

#include"std.hpp"
#include "crow/json.h"
#include"nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include"crow.h"
#include <cstddef>
#include <string_view>

#include"DataBaseManager.hpp"
#include"llama_wrapper.hpp"
#include"model_pool.hpp"

/**
 * @brief 解析状态枚举
 *
 * 表示 AI 输出内容的解析结果类型
 */
enum class Parse_Status
{
    CHAT,        // 普通对话，无需查询数据库
    QUERY,       // 需要查询数据库
    PARSE_ERROR  // 解析失败
};

/**
 * @brief 解析结果结构体
 *
 * 封装 AI 输出的解析结果，包含状态和解析后的 JSON 数据
 */
struct Parse_Result
{
    Parse_Status status;  // 解析状态
    nlohmann::json parse_json;  // 解析后的 JSON 数据

};

/**
 * @brief 可变参数模板版本 - 检查多个字段是否存在
 *
 * 编译期展开，适合直接传入多个字段名进行检查
 * 示例：has_all_fields(j, "mode", "answer", "target")
 *
 * @tparam Args 字段名类型（自动推导为 string/string_view 等）
 * @param j 要检查的 JSON 对象
 * @param args 可变参数，字段名列表
 * @return true 所有字段都存在
 * @return false 至少有一个字段缺失
 */
template <typename ...Args>
bool has_all_fields(const nlohmann::json& j,Args&&... args)
{
    return (j.contains(args) && ...);
}

/**
 * @brief vector 版本 - 从配置中动态检查字段
 *
 * 运行时遍历，适合从 map 等配置中取出字段列表后进行检查
 *
 * @param j 要检查的 JSON 对象
 * @param list 字段名字符串列表（vector）
 * @return true 所有字段都存在
 * @return false 至少有一个字段缺失
 */
bool check_fields_vector(const nlohmann::json &j, const std::vector<std::string>& list);


/**
 * @brief AI 工具调用代理类
 *
 * 实现 ReAct（Reasoning + Acting）模式，让 AI 能够调用数据库查询工具。
 * 工作流程：接收用户输入 -> AI 生成 JSON 指令 -> 解析指令 -> 执行数据库查询 ->
 * 将结果喂回 AI -> AI 生成最终回答
 */
class ToolAgent
{
    private:
    DataBaseManager& db;  // 数据库管理器引用，用于执行查询
    model_set model; // AI 模型实例指针
    static constexpr int MAX_LOOP = 3;  // 最大循环次数，防止无限循环
    std::map<Parse_Status,std::function<nlohmann::json(const nlohmann::json&)>> Execute_Function;  // 执行函数映射表
    std::map<std::string,std::function<nlohmann::json(const nlohmann::json&)>> Query_Function;
    /**
     * @brief 解析 AI 输出的消息
     * @param message AI 生成的原始消息
     * @return 解析结果，包含状态和 JSON 数据
     */
    Parse_Result parse(std::string_view);

    /**
     * @brief 执行数据库查询
     * @param result 解析结果，包含查询指令
     * @return 查询结果的 JSON 格式
     */
    nlohmann::json execute(const Parse_Result& result);


    std::string intent(const std::string& msg);
    std::string query(const std::string& msg);
    std::string summary(const std::string& msg);

    public:
    /**
     * @brief 构造函数
     * @param db 数据库管理器引用
     * @param model AI 模型实例指针
     */
    ToolAgent(DataBaseManager& db,model_set model);

    // 禁止拷贝和移动，确保引用成员安全
    ToolAgent(const ToolAgent&) = delete;
    ToolAgent& operator=(const ToolAgent&) = delete;
    ToolAgent(ToolAgent&&) = delete;
    ToolAgent& operator=(ToolAgent&&) = delete;


    /**
     * @brief 运行工具代理
     * @param message 用户输入的消息
     * @return AI 的最终回答
     *
     * 主入口函数，协调解析-执行-反馈的完整流程
     */
    std::string run(std::string_view);

};
