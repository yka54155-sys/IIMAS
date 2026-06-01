#include"ToolAgent.hpp"
#include "DataBaseManager.hpp"
#include "crow/logging.h"
#include "llama_wrapper.hpp"
#include "model.hpp"
#include "model_pool.hpp"
#include "nlohmann/json_fwd.hpp"
#include "AI_params.hpp"
#include"model.hpp"
#include <algorithm>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

// 辅助函数：创建失败结果，避免全局变量的线程安全问题
static nlohmann::json make_failed_result(const std::string& msg)
{
    return nlohmann::json{{"status", "failed"}, {"message", msg}};
}

/**
 * @brief 构造函数
 *
 * 初始化 ToolAgent，绑定数据库管理器和 AI 模型实例，
 * 初始化执行函数映射表 Execute_Function
 *
 * @param db 数据库管理器引用
 * @param model AI 模型实例指针
 */
ToolAgent::ToolAgent(DataBaseManager& db,model_set model) : db(db) , model(model) 
{
    /*快速构造失败结果的辅助 lambda，避免重复书写相同的 JSON 结构*/
    auto failed_json = [](std::string msg)
    {
        return nlohmann::json({{"status","failed"},{"message",msg}});
    };
    /*
    {
        "mode":"query",
        "target":"students/groups/SAG",
        "conditions":
        [
            {"token":"class/name/student_id...","value":"(要查询的值)","match":"exact/fuzzy"},
            {"token":"class/name/student_id...","value":"(要查询的值)","match":"exact/fuzzy"},
            ...
        ],
        "logic":"AND/OR",
        "order_by":"CLASS/NAME...",
        "order_dir":"ASC/DESC"
    }
    */

    Query_Function["students"] = [&](const nlohmann::json& json)
    {
        nlohmann::json result;
        result["data"] = nlohmann::json::array();
        if(!json["conditions"].is_array())
        {
            return failed_json("conditions格式错误，应该是数组格式");
        }
        std::vector<std::vector<Student>> stus_lists;
        for(const auto& condition : json["conditions"])
        {
            if (!has_all_fields(condition, "token","value","match"))
            {
                return failed_json("condition数组中某组缺少必要字段");
            }
            auto it = StudentMap.find(condition["token"].get<std::string>());
            if(it == StudentMap.end())
            {
                return failed_json("conditions中有非法的token值");
            }
            StudentToken token = it->second;
            std::string value = condition["value"].get<std::string>();
            std::vector<Student> temp;
            const std::set<std::string> MatchMap = {"exact","fuzzy"};
            auto match_it = MatchMap.find(condition["match"]);
            if(match_it == MatchMap.end())
            {
                return failed_json("非法的match值");
            }
            std::string match = *match_it;
            const std::map<std::string,std::function<std::vector<Student>()>> search = 
            {
                {
                    "exact",[&]()
                    {
                        std::vector<Student> temp = db.exactSearchStudent(value, token);
                        return temp;
                    }
                },
                {
                    "fuzzy",[&]()
                    {
                        std::vector<Student> temp = db.fuzzySearchStudent(value, token);
                        return temp;
                    }
                }
            };
            auto search_it = search.find(match);
            if(search_it == search.end())
            {
                return failed_json("错误的match值，检查ToolAgent.cpp中的76-93行");
            }
            temp = search_it->second();

            stus_lists.emplace_back(temp);
        }

        if(stus_lists.empty())
        {
            return failed_json("conditions数组为空，至少需要一个查询条件");
        }
        const std::set<std::string> LogicMap = {"OR","AND"};
        auto logic_it = LogicMap.find(json["logic"]);
        std::vector<Student> stus_lists_final;
        if(logic_it == LogicMap.end())
        {
            return failed_json("非法的logic值");
        }
        if(*logic_it == "OR")
        {
            std::set<Student> merged;
            for(auto& list : stus_lists)
            {
                merged.insert(list.begin(),list.end());
            }
            stus_lists_final.assign(merged.begin(),merged.end());

        }
        else if(*logic_it == "AND")
        {
            std::set<Student> merged(stus_lists[0].begin(),stus_lists[0].end());
            for(size_t i = 1;i < stus_lists.size();i++)
            {
                std::set<Student> b(stus_lists[i].begin(),stus_lists[i].end());
                std::set<Student> temp;
                std::set_intersection(merged.begin(),merged.end(),b.begin(),b.end(),std::inserter(temp, temp.begin()));

                merged = std::move(temp);
            }
            stus_lists_final.assign(merged.begin(),merged.end());

        }
        else
        {
            return failed_json("未定义函数的logic值");
        }
        if(stus_lists_final.empty())
        {
            return failed_json("没有符合条件的查询结果");
        }

        int back_num = 0;
        //先排序
        std::string order_dir = json["order_dir"];
        auto dir_it = OrderDirectionMap.find(order_dir);
        if(dir_it == OrderDirectionMap.end())
        {
            return failed_json("非法的order_dir值，注意小写");
        }
        std::string order_by = json["order_by"];
        auto by_it = StudentMap.find(order_by);
        if(by_it == StudentMap.end())
        {
            return failed_json("非法的order_by值，注意小写");
        }
        stus_lists_final = db.orderStudent(stus_lists_final,by_it->second,dir_it->second);
        for(auto& stus : stus_lists_final)
        {
            
            
            nlohmann::json temp;
            temp["class"] = stus.Class;
            temp["name"] = stus.Name;
            temp["student_id"] = stus.Student_Id;
            temp["subject"] = stus.Subject;
            temp["remark"] = stus.Remark;
            temp["phone"] = stus.Phone;
            temp["gender"] = stus.Gender;
            if(back_num < AI_params::max_json_back)
            {
                result["data"].push_back(temp);
            }
            else
            {
                break;
            }
            back_num++;
        }

        


        return result;
    };
    Query_Function["groups"] = [&](const nlohmann::json& json)
    {
        nlohmann::json result;
        result["data"] = nlohmann::json::array();
        if(!json["conditions"].is_array())
        {
            return failed_json("conditions格式错误，应该是数组格式");
        }
        if(json["conditions"].empty())
        {
            return failed_json("conditions不能为空");
        }
        // 遍历条件数组，对每个条件执行分组查询
        std::vector<std::vector<Group>> grps_lists;
        for(const auto& condition : json["conditions"])
        {
            if(!has_all_fields(condition, "token","value","match"))
            {
                return failed_json("condition数组中某组缺少必要字段");
            }
            // 验证 token 是否在 GroupMap 中
            auto it = GroupMap.find(condition["token"].get<std::string>());
            if(it == GroupMap.end())
            {
                return failed_json("conditions中有非法的token值");
            }
            GroupToken token = it->second;
            std::string value = condition["value"].get<std::string>();
            const std::set<std::string> MatchMap = {"exact","fuzzy"};
            auto match_it = MatchMap.find(condition["match"]);
            if(match_it == MatchMap.end())
            {
                return failed_json("非法的match值");
            }
            std::string match = *match_it;
            // 根据 match 类型分发到精确搜索或模糊搜索
            const std::map<std::string,std::function<std::vector<Group>()>> search =
            {
                {
                    "exact",[&]()
                    {
                        return db.exactSearchGroup(value, token);
                    }
                },
                {
                    "fuzzy",[&]()
                    {
                        return db.fuzzySearchGroup(value, token);
                    }
                }
            };
            auto search_it = search.find(match);
            if(search_it == search.end())
            {
                return failed_json("错误的match值");
            }
            grps_lists.emplace_back(search_it->second());
        }

        // 根据 logic 字段合并多个条件的查询结果
        const std::set<std::string> LogicMap = {"OR","AND"};
        auto logic_it = LogicMap.find(json["logic"]);
        if(logic_it == LogicMap.end())
        {
            return failed_json("非法的logic值");
        }
        std::vector<Group> grps_final;
        if(*logic_it == "OR")
        {
            // OR：用 set 去重后合并所有条件的结果
            std::set<Group> merged;
            for(auto& list : grps_lists)
            {
                merged.insert(list.begin(), list.end());
            }
            grps_final.assign(merged.begin(), merged.end());
        }
        else if(*logic_it == "AND")
        {
            // AND：对所有条件的结果取交集
            std::set<Group> merged(grps_lists[0].begin(), grps_lists[0].end());
            for(size_t i = 1; i < grps_lists.size(); i++)
            {
                std::set<Group> b(grps_lists[i].begin(), grps_lists[i].end());
                std::set<Group> temp;
                std::set_intersection(merged.begin(), merged.end(),
                                      b.begin(), b.end(),
                                      std::inserter(temp, temp.begin()));
                merged = std::move(temp);
            }
            grps_final.assign(merged.begin(), merged.end());
        }
        else
        {
            return failed_json("未定义处理的logic值");
        }
        if(grps_final.empty())
        {
            return failed_json("没有符合条件的查询结果");
        }

        // 验证排序参数并排序
        std::string order_dir = json["order_dir"];
        auto dir_it = OrderDirectionMap.find(order_dir);
        if(dir_it == OrderDirectionMap.end())
        {
            return failed_json("非法的order_dir值，注意小写");
        }
        std::string order_by = json["order_by"];
        auto by_it = GroupMap.find(order_by);
        if(by_it == GroupMap.end())
        {
            return failed_json("非法的order_by值，注意小写");
        }
        grps_final = db.orderGroup(grps_final, by_it->second, dir_it->second);

        // 序列化结果，限制返回数量不超过 max_json_back
        int back_num = 0;
        for(auto& grp : grps_final)
        {
            nlohmann::json temp;
            temp["group_name"] = grp.Group_Name;
            temp["description"] = grp.Description;
            if(back_num < AI_params::max_json_back)
            {
                result["data"].push_back(temp);
            }
            else
            {
                break;
            }
            back_num++;
        }

        return result;
    };
    Query_Function["SAG"] = [&](const nlohmann::json& json)
    {
        nlohmann::json result;
        result["data"] = nlohmann::json::array();
        if(!json["conditions"].is_array())
        {
            return failed_json("conditions格式错误，应该是数组格式");
        }
        if(json["conditions"].empty())
        {
            return failed_json("conditions不能为空");
        }
        // SAG 搜索只需 token 和 value，不需要 match（searchSAG 内部按 token 类型决定匹配方式）
        std::vector<std::vector<SAG>> sag_lists;
        for(const auto& condition : json["conditions"])
        {
            if(!has_all_fields(condition, "token","value"))
            {
                return failed_json("condition数组中某组缺少必要字段");
            }
            // 验证 token 是否在 SAGMap 中
            auto it = SAGMap.find(condition["token"].get<std::string>());
            if(it == SAGMap.end())
            {
                return failed_json("conditions中有非法的token值");
            }
            SAGToken token = it->second;
            std::string value = condition["value"].get<std::string>();
            sag_lists.emplace_back(db.searchSAG(value, token));
        }

        // 根据 logic 字段合并多个条件的查询结果
        const std::set<std::string> LogicMap = {"OR","AND"};
        auto logic_it = LogicMap.find(json["logic"]);
        if(logic_it == LogicMap.end())
        {
            return failed_json("非法的logic值");
        }
        std::vector<SAG> sag_final;
        if(*logic_it == "OR")
        {
            // OR：用 set 去重后合并所有条件的结果
            std::set<SAG> merged;
            for(auto& list : sag_lists)
            {
                merged.insert(list.begin(), list.end());
            }
            sag_final.assign(merged.begin(), merged.end());
        }
        else if(*logic_it == "AND")
        {
            // AND：对所有条件的结果取交集
            std::set<SAG> merged(sag_lists[0].begin(), sag_lists[0].end());
            for(size_t i = 1; i < sag_lists.size(); i++)
            {
                std::set<SAG> b(sag_lists[i].begin(), sag_lists[i].end());
                std::set<SAG> temp;
                std::set_intersection(merged.begin(), merged.end(),
                                      b.begin(), b.end(),
                                      std::inserter(temp, temp.begin()));
                merged = std::move(temp);
            }
            sag_final.assign(merged.begin(), merged.end());
        }
        else
        {
            return failed_json("未定义处理的logic值");
        }

        // 联查学生信息和分组名：SAG 表只存 ID，需要反查得到完整信息
        std::vector<Student> students_for_sort;
        // 以学生 db id 为 key，存储该条关联记录的 group_name 和 SAG 自身的 remark
        std::map<int, std::pair<std::string,std::string>> sag_extra; // student db id -> (group_name, sag_remark)
        for(auto& sag : sag_final)
        {
            // 通过学生 db id 反查学生信息
            auto stus = db.exactSearchStudent(std::to_string(sag.Student_Id), StudentToken::ID);
            // 通过分组 db id 反查分组名
            auto grps = db.exactSearchGroup(std::to_string(sag.Group_Id), GroupToken::ID);
            // 跳过数据孤岛（学生或分组已被删除但关联记录残留）
            if(stus.empty() || grps.empty()) continue;
            students_for_sort.emplace_back(stus[0]);
            sag_extra[stus[0].Id] = {grps[0].Group_Name, sag.Remark};
        }
        if(students_for_sort.empty())
        {
            return failed_json("没有符合条件的查询结果");
        }

        // 验证排序参数（order_by 使用 StudentMap，因为返回的是学生信息）
        std::string order_dir = json["order_dir"];
        auto dir_it = OrderDirectionMap.find(order_dir);
        if(dir_it == OrderDirectionMap.end())
        {
            return failed_json("非法的order_dir值，注意小写");
        }
        std::string order_by = json["order_by"];
        auto by_it = StudentMap.find(order_by);
        if(by_it == StudentMap.end())
        {
            return failed_json("非法的order_by值，注意小写");
        }
        students_for_sort = db.orderStudent(students_for_sort, by_it->second, dir_it->second);

        // 序列化结果，限制返回数量不超过 max_json_back
        int back_num = 0;
        for(auto& stu : students_for_sort)
        {
            nlohmann::json temp;
            // 学生基本信息
            temp["class"] = stu.Class;
            temp["name"] = stu.Name;
            temp["student_id"] = stu.Student_Id;
            temp["subject"] = stu.Subject;
            temp["phone"] = stu.Phone;
            temp["gender"] = stu.Gender;
            // 补充 SAG 特有字段：所在组名和 SAG 自身的 remark（非学生的 remark）
            auto extra_it = sag_extra.find(stu.Id);
            if(extra_it != sag_extra.end())
            {
                temp["group_name"] = extra_it->second.first;
                temp["remark"] = extra_it->second.second;
            }
            if(back_num < AI_params::max_json_back)
            {
                result["data"].push_back(temp);
            }
            else
            {
                break;
            }
            back_num++;
        }

        return result;
    };

    /*初始化状态到处理函数的映射表*/
    Execute_Function[Parse_Status::CHAT] = [&](const nlohmann::json& json)
    {
        nlohmann::json result;
        /*将 AI 的 answer 字段透传，并补充 status 字段供前端判断*/
        result["status"]  = "success";
        result["answer"]  = json.value("answer", "");

        return result;
    };
    Execute_Function[Parse_Status::QUERY] = [&](const nlohmann::json& json)
    {
        nlohmann::json result;
        auto it = Query_Function.find(json["target"].get<std::string>());
        if(it == Query_Function.end())
        {
            result["status"] = "failed";
            result["success"] = "未知的target";
        }
        else
        {
            
            result = std::move(it->second(json));
            result["status"] = "success";
        }
        
        return result;
    };
    Execute_Function[Parse_Status::PARSE_ERROR] = [&](const nlohmann::json& json)
    {
        nlohmann::json result;
        result = json;
        result["status"] = "failed";
        return result;
    };

}

/**
 * @brief 检查 JSON 是否包含所有指定字段
 *
 * 遍历字段列表，检查 JSON 对象中是否存在所有必需字段
 *
 * @param j 要检查的 JSON 对象
 * @param list 必需字段名称列表
 * @return true 所有字段都存在
 * @return false 至少有一个字段缺失
 */
bool check_fields_vector(const nlohmann::json& j, const std::vector<std::string>& list)
{
    /*遍历字段列表检查存在性*/
    for(const auto& c : list)
    {
        if(!j.contains(c)) return false;
    }

    return true;
}


/**
 * @brief 解析 AI 输出的消息
 *
 * 验证 AI 返回的 JSON 格式，检查 mode 字段和必需参数，
 * 根据 mode 值返回对应的 Parse_Status
 *
 * @param message AI 生成的原始 JSON 字符串
 * @return Parse_Result 包含解析状态和解析后的 JSON 数据
 */
Parse_Result ToolAgent::parse(std::string_view message)
{
    /*各mode所需字段验证表 - 使用vector替代initializer_list避免生命周期问题*/
    const std::map<std::string,std::vector<std::string>> ModeValidate = 
    {
        {"chat",{"answer"}},
        {"query",{"target","conditions","logic","order_by","order_dir"}} 
    };
    /*mode字符串到枚举的映射表*/
    const std::map<std::string,Parse_Status> MapMode = 
    {
        {"chat",Parse_Status::CHAT},
        {"query",Parse_Status::QUERY}
    };
    
    /**
     * @brief 构造错误结果
     * @param message 错误信息
     * @return 状态为 PARSE_ERROR 的 Parse_Result
     */
    auto failed_result = [&](std::string_view message)
    {
        Parse_Result r = 
        {
            .status = Parse_Status::PARSE_ERROR,
            .parse_json = {{"status","failed"},{"message",message}}
            
        };
        
        return r;
    };

    Parse_Result p_result;
    /*AI接收问题后的第一次判断json
    {
        "mode":"chat",
        "answer":"(与操作无关的回答内容)"
    }
    {
        "mode":"query",
        "target":"students/groups/SAG",
        "conditions":
        [
            {"token":"class/name/student_id...","value":"(要查询的值)","match":"exact/fuzzy"},
            {"token":"class/name/student_id...","value":"(要查询的值)","match":"exact/fuzzy"},
            ...
        ],
        "logic":"AND/OR",
        "order_by":"CLASS/NAME...",
        "order_dir":"ASC/DESC"
    }
    {
        "mode":"（无效内容）",
        "answer":"模型异常请稍后重试（然后尝试重新初始化）"
    }
    */
    
    try
    {
        std::string mode;
        p_result.parse_json = nlohmann::json::parse(message);
        /*检查是否存在mode字段*/
        if(has_all_fields(p_result.parse_json, "mode"))
        {
            /*验证mode值是否有效*/
            auto it = ModeValidate.find(p_result.parse_json["mode"]);
            if(it == ModeValidate.end())
            {
                /*mode值不在验证表中*/
                return failed_result("无效的mode值");
            }

            /*验证该mode下的必需字段*/
            if(!check_fields_vector(p_result.parse_json,it->second))
            {
                /*缺少该mode必需的字段*/
                return failed_result("该mode缺少必要字段");
            }

            /*解析成功，添加状态信息*/
            p_result.parse_json["status"] = "success";
            p_result.parse_json["message"] = "解析成功";
            /*根据mode值设置解析状态*/
            p_result.status = [&]()
            {
                auto it = MapMode.find(p_result.parse_json["mode"]);
                if(it == MapMode.end())
                {
                    return Parse_Status::PARSE_ERROR;
                }
                else
                {
                    return it->second;
                }
            }();

            return p_result;
        }
        else
        {
            /*缺少mode字段*/
            return failed_result("缺少必要字段mode");
        }
        
        /*理论上不会执行到这里，兜底返回*/
        return failed_result("未知的JSON解析异常问题");

         
    }
    
    catch (const nlohmann::json::parse_error& e)
    {
        /*JSON语法解析失败*/
        CROW_LOG_ERROR << "模型json输出格式异常：" << e.what();
        return failed_result("模型json输出格式异常：");
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        /*JSON字段访问越界*/
        CROW_LOG_ERROR << "JSON字段访问越界：" << e.what();
        return failed_result("JSON字段访问越界");
    }
    catch (const std::exception& e)
    {
        /*其他标准异常捕获*/
        CROW_LOG_ERROR << "JSON解析异常，std部分："  << e.what();
        return failed_result("JSON解析异常，std部分");   
    }
    /*错误处理结束*/
}

nlohmann::json ToolAgent::execute(const Parse_Result&  p_result)
{
    
    auto it = Execute_Function.find(p_result.status);
    if(it == Execute_Function.end())
    {
        return {{"status","failed"},{"message",{"未知的mode值"}}};
    }

    return it->second(p_result.parse_json);
}


std::string ToolAgent::intent(const std::string& msg)
{
    if(!model.exist())
    {
        return make_failed_result("模型未初始化").dump();
    }
    model.intent_model->clear_message();
    model.intent_model->add_message("system", AI_params::IntentParams::prompt);
    model.intent_model->add_message("user", msg);

    std::string ai_response = model.intent_model->talk();

    CROW_LOG_INFO << "IntentAI raw: " << ai_response;

    // 后처理：优先从 AI 输出中提取 chat/query
    // 同时检查用户原始消息关键词，防止 AI 输出非预期内容时误判为 chat
    const std::vector<std::string> query_keywords = {"query","查询","查","团员","学生","分组","班级","都有谁","有哪些","谁在"};
    auto contains = [](const std::string& text, const std::string& kw)
    {
        return text.find(kw) != std::string::npos;
    };
    for(const auto& kw : query_keywords)
    {
        if(contains(ai_response, kw) || contains(msg, kw))
        {
            return "query";
        }
    }
    return "chat";
}

std::string ToolAgent::query(const std::string& msg)
{
    if(!model.exist())
    {
        return make_failed_result("模型未初始化").dump();
    }

    /*从原始输出中提取第一个完整的 JSON 对象（大括号匹配），丢弃多余内容*/
    auto extract_json = [](const std::string& raw) -> std::string
      {
          auto start = raw.find('{');
          if(start == std::string::npos) return raw;
          int depth = 0;
          bool in_string = false;
          bool escape = false;
          for(size_t i = start; i < raw.size(); i++)
          {
              char c = raw[i];
              if(escape)          { escape = false; continue; }
              if(c == '\\' && in_string) { escape = true; continue; }
              if(c == '"')        { in_string = !in_string; continue; }
              if(in_string)       continue;
              if(c == '{')        depth++;
              else if(c == '}')   { depth--; if(depth == 0) return raw.substr(start, i - start + 1); }
          }
          return raw; // 没找到完整闭合，返回原始内容让 parse() 报错
      };

    model.query_model->clear_message();
    model.query_model->add_message("system", AI_params::QueryParams::prompt);
    model.query_model->add_message("user", msg);
    Parse_Result parse_result;
    std::string ai_json = model.query_model->talk();
    CROW_LOG_INFO << "QueryAI raw: " << ai_json;
    std::string extracted = extract_json(ai_json);
    CROW_LOG_INFO << "QueryAI extracted: " << extracted;
    parse_result = parse(extracted);
    int query_time = 0;
    while (parse_result.status != Parse_Status::QUERY)
    {
        model.query_model->clear_message();
        model.query_model->add_message("system", AI_params::QueryParams::prompt);
        model.query_model->add_message("user", "json字段格式错误，重新构造以下问题的json格式:\n" + msg);
        ai_json = model.query_model->talk();
        CROW_LOG_INFO << "QueryAI retry raw: " << ai_json;
        parse_result = parse(extract_json(ai_json));
        if(parse_result.status == Parse_Status::QUERY || parse_result.status == Parse_Status::CHAT)
        {
            break;
        }
        query_time++;
        if (query_time >= AI_params::QueryParams::max_query_time)
        {
            break;
        }
    }

    if(parse_result.status == Parse_Status::PARSE_ERROR)
    {
        return make_failed_result("查询模型输出异常").dump();
    }
    else
    {
        std::string ai_response = parse_result.parse_json.dump();
        CROW_LOG_INFO << "QueryAI: " << ai_response;
        return ai_response;
    }
}

std::string ToolAgent::summary(const std::string& msg)
{
    if(!model.exist())
    {
        return make_failed_result("模型未初始化").dump();
    }
    model.summary_model->add_message("user", msg);
    std::string ai_response = model.summary_model->talk();

    CROW_LOG_INFO << "SummaryAI: " << ai_response;

    return ai_response;
}

std::string ToolAgent::run(std::string_view message)
{
    try
    {
        if(!model.exist())
        {
            return make_failed_result("模型未初始化").dump();
        }




        size_t current_loop = 0;
        std::string ai_response;
        std::string msg(message);


        model.summary_model->clear_message();
        model.summary_model->add_message("system", AI_params::SummaryParams::prompt);

        const std::map<std::string,std::function<std::string(const std::string&)>> chatmode = 
        {
        {"chat",[&](const std::string& msg)
        {
            return summary("直接回答这个问题:" + msg);
        }},
        {"query",[&](const std::string& msg)
        {
            std::string json = query(msg);
            nlohmann::json temp = nlohmann::json::parse(json);
            if(temp["status"] == "failed")
            {
                return json;
            }

            nlohmann::json query_json = Execute_Function[Parse_Status::QUERY](temp);

            std::string ai_response = summary(query_json.dump() + "按以下问题，总结以上内容：" + msg);

            return ai_response;
        }}
        };
        std::string intent_result = intent(msg);
        CROW_LOG_INFO << "IntentAI result: [" << intent_result << "]";
        auto it = chatmode.find(intent_result);
        if(it == chatmode.end())
        {
            CROW_LOG_ERROR << "IntentAI invalid: [" << intent_result << "]";
            return make_failed_result("意图模型输出非法的chatmode").dump();
        }else
        {
            ai_response = it->second(msg);
            nlohmann::json feedback;
            feedback["status"] = "success";
            feedback["message"] = "模型响应成功";
            feedback["answer"] = ai_response;  

            return feedback.dump();
        }    
    }
    catch (const nlohmann::json::parse_error& e) 
    {
    // AI 输出的不是有效 JSON
    // 走兜底逻辑：把原始文本当作回答
        std::cerr << "AI 输出的不是有效 JSON：" << e.what() << "\n";
        CROW_LOG_ERROR << "AI 输出的不是有效 JSON：" << e.what();
        
        return make_failed_result("AI 输出的不是有效 JSON：" + std::string(e.what())).dump();
    
    } 
    catch (const nlohmann::json::out_of_range& e) 
    {
        // JSON 格式正确但缺少必要字段
        // AI 返回了不符合约定的 JSON
        std::cerr << "JSON 格式正确但缺少必要字段" << e.what() << "\n";
        CROW_LOG_ERROR << "JSON 格式正确但缺少必要字段" << e.what();
        
        return make_failed_result("JSON 格式正确但缺少必要字段" + std::string(e.what())).dump();
        
    } 
    catch (const nlohmann::json::type_error& e) 
    {
        // 字段类型不匹配，比如期望字符串但实际是数字
        std::cerr << "字段类型不匹配，比如期望字符串但实际是数字" << e.what() << "\n";
        CROW_LOG_ERROR << "字段类型不匹配，比如期望字符串但实际是数字" << e.what();
        
        return make_failed_result("字段类型不匹配，比如期望字符串但实际是数字" + std::string(e.what())).dump();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ToolAgent 调用失败 (std)：" << e.what() << "\n";
        CROW_LOG_ERROR << "ToolAgent 调用失败 (std)：" << e.what();
        
        return make_failed_result("ToolAgent 调用失败 (std)：" + std::string(e.what())).dump();
        
    }
    
}