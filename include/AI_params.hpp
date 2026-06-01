#pragma once
#include<string>
#include"llama_wrapper.hpp"
#include"llama.h"
namespace AI_params
{

    namespace IntentParams
    {
        // 意图分类提示词：模型只需输出 "chat" 或 "query"
        inline const std::string prompt = R"(判断用户消息是否需要查询数据库中的学生或分组数据。
只能输出 chat 或 query，不要输出任何其他内容。

query：查学生信息、查分组、查班级、查团员、查谁在哪个组
chat：问候、知识问答、算法、编程、数学、天气

只输出一个单词 chat 或 query。)";

        inline const std::string path = "model/Qwen2.5-1.5B-Instruct-Q3_K_M.gguf";
        // 意图分类只需一个单词输出
        inline const int max_new_tokens = 10;

        inline init_model_params model_params
        {
            .n_gpu_layers = 0,
            .use_mmap = true,
            .use_mlock = false
        };

        inline init_ctx_params ctx_params
        {
            .n_ctx = 4096,
            .n_threads = 4,
            .n_threads_batch = 8,
            .embeddings = false
        };

        inline penalties_params penal_params
        {
            // 意图分类输出极短，不需要惩罚窗口
            .penalty_last_n = 16,
            .penalty_repeat = 1.0,
            .penalty_freq = 0.0,
            .penalty_present = 0.0,
        };

        inline struct sampler_init_params sam_params
        {
            // top_k = 1：意图分类只需选择 chat/query，使用贪心解码
            .top_k = 1,
            .top_p_min = 1,
            .top_p_p = 1.0,
            // 温度最低：意图分类需要完全确定性的输出
            .temp = 0.01,
            .dist = 12345
        };

    }
    namespace QueryParams
    {
        // 查询JSON构造提示词：模型只需生成查询JSON
        inline const std::string prompt = R"(你是JSON生成器。将用户的查询请求转换为标准JSON格式。

【严格规则】
1. 只输出一行JSON，不要换行
2. 禁止输出Markdown代码块标记(```)
3. 禁止输出任何解释、说明、问候语
4. students和groups的每个condition必须包含token、value、match三个字段
5. SAG的每个condition只需token和value两个字段，不要加match
6. order_dir必须小写(asc或desc)

【输出格式】
students/groups: {"mode":"query","target":"students","conditions":[{"token":"字段","value":"值","match":"exact/fuzzy"}],"logic":"AND/OR","order_by":"字段","order_dir":"asc/desc"}
SAG: {"mode":"query","target":"SAG","conditions":[{"token":"字段","value":"值"}],"logic":"AND/OR","order_by":"name","order_dir":"asc"}

【字段说明】
- students表token: name, student_id, gender, class, subject, phone, remark, create_time
- groups表token: name, description, create_time
- SAG表token: student, group, remark, join_time
- match: exact(精确匹配)或fuzzy(模糊匹配，包含即可)

【示例 - 严格模仿】

输入:查2班学生
输出:{"mode":"query","target":"students","conditions":[{"token":"class","value":"2","match":"exact"}],"logic":"OR","order_by":"name","order_dir":"asc"}

输入:找姓张的
输出:{"mode":"query","target":"students","conditions":[{"token":"name","value":"张","match":"fuzzy"}],"logic":"OR","order_by":"name","order_dir":"asc"}

输入:查张三的信息
输出:{"mode":"query","target":"students","conditions":[{"token":"name","value":"张三","match":"exact"}],"logic":"OR","order_by":"name","order_dir":"asc"}

输入:张三在哪个组
输出:{"mode":"query","target":"SAG","conditions":[{"token":"student","value":"张三"}],"logic":"OR","order_by":"name","order_dir":"asc"}

输入:团员有哪些
输出:{"mode":"query","target":"students","conditions":[{"token":"remark","value":"团员","match":"fuzzy"}],"logic":"OR","order_by":"name","order_dir":"asc"}

输入:查所有分组
输出:{"mode":"query","target":"groups","conditions":[{"token":"name","value":"","match":"fuzzy"}],"logic":"OR","order_by":"name","order_dir":"asc"})";

        inline const std::string path = "model/Qwen2.5-1.5B-Instruct-Q3_K_M.gguf";
        // 增加token数，确保JSON不会被截断
        inline const int max_new_tokens = 1024;
        inline const int max_query_time = 3;

        inline init_model_params model_params
        {
            .n_gpu_layers = 0,
            .use_mmap = true,
            .use_mlock = false
        };

        inline init_ctx_params ctx_params
        {
            .n_ctx = 4096,
            .n_threads = 4,
            .n_threads_batch = 8,
            .embeddings = false
        };

        inline penalties_params penal_params
        {
            // 窗口缩小到 32：JSON 输出极短，无需大窗口；避免过度惩罚 JSON 语法中的高频 token
            .penalty_last_n = 32,
            // 轻微惩罚防止死循环，但不能太高（JSON 中 { " : } 等符号高频出现是正常的）
            .penalty_repeat = 1.0,
            .penalty_freq = 0.0,
            .penalty_present = 0.0,
        };

        inline struct sampler_init_params sam_params
        {
            // JSON生成需要极高确定性，使用贪心解码
            .top_k = 1,
            .top_p_min = 1,
            .top_p_p = 1.0,
            // 温度接近0，确保格式严格遵循
            .temp = 0.01,
            .dist = 12345
        };

    }
    namespace SummaryParams
    {
        // 总结提示词：既能直接回答普通问题，也能总结数据库查询结果
        inline const std::string prompt = R"(你是班级管理助手，负责回答老师和学生的问题。

回答要求：
1. 如果消息以"直接回答这个问题:"开头，直接友好地回答该问题，不要提及数据库
2. 如果消息包含JSON数据，根据JSON中的data数组内容回答后面的问题
3. 如果JSON中data为空或status为failed，告诉用户没有找到相关数据
4. 语言简洁清晰，不要过于冗长，保持友好专业的语气)";

        inline const std::string path = "model/Qwen2.5-1.5B-Instruct-Q3_K_M.gguf";
        inline const int max_new_tokens = 512;

        inline init_model_params model_params
        {
            .n_gpu_layers = 0,
            .use_mmap = true,
            .use_mlock = false
        };

        inline init_ctx_params ctx_params
        {
            .n_ctx = 4096,
            .n_threads = 4,
            .n_threads_batch = 8,
            .embeddings = false
        };

        inline penalties_params penal_params
        {
            // 总结模式需要更大的窗口，允许一定程度的重复惩罚
            .penalty_last_n = 64,
            .penalty_repeat = 1.1,
            .penalty_freq = 0.0,
            .penalty_present = 0.0,
        };

        inline struct sampler_init_params sam_params
        {
            // top_k 适当提高：自然语言生成需要更多多样性
            .top_k = 40,
            .top_p_min = 1,
            .top_p_p = 0.9,
            // 温度提高：自然语言生成需要一定的随机性，输出更自然
            .temp = 0.7,
            .dist = 12345
        };

    }

    inline const int pool_size = 1;

    inline const int max_json_back = 10;

}
