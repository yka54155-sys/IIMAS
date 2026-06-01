#include"llama_wrapper.hpp"
#include "AI_params.hpp"
#include"llama.h"
#include <atomic>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

// 引用计数，保证 llama_backend_init/free 在进程内只调用一次
static std::atomic<int> backend_ref_count{0};

/**
 * @brief 构造函数（默认参数）
 * @param path 模型文件路径
 * 
 * 使用默认参数加载模型和初始化上下文
 */
llama_wrapper::llama_wrapper(std::string path)
{
    // 设置静默日志回调
    llama_log_set([](ggml_log_level, const char*, void*) {}, nullptr);
    // 引用计数为 0 时才初始化后端，避免多实例重复初始化
    if(backend_ref_count.fetch_add(1) == 0)
        llama_backend_init();
    
    // 配置模型参数
    model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0;    // GPU 层数设为 0，纯 CPU 运行
    model_params.use_mmap = true;     // 使用 mmap 加速加载
    model_params.use_mlock = false;   // 不锁定内存
    
    // 从文件加载模型
    model = llama_model_load_from_file(path.c_str(),model_params);
    
    // 检查模型是否加载成功
    if(!model)
    {
        std::cerr << "模型加载错误，检查模型路径 " << std::endl;
        throw std::runtime_error("model load failed ,check the path");
    }

    save.max_new_tokens = 512;  // 默认值，实际使用时会被带参数的构造函数覆盖

    // 配置上下文参数
    struct llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 4096;         // 上下文大小
    ctx_params.n_threads = 8;        // 线程数
    ctx_params.n_threads_batch = 8;  // 批处理线程数
    ctx_params.embeddings = false;   // 不使用嵌入


    // 从模型初始化上下文
    context = llama_init_from_model(model,ctx_params);
    
    // 检查上下文是否创建成功
    if(!context)
    {
        std::cerr << "上下文创建失败" << std::endl;
        llama_model_free(model);
        throw std::runtime_error("context create failed");
        
    }

    // 初始化采样器链
    sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());

    // 添加惩罚机制
    llama_sampler_chain_add(sampler, llama_sampler_init_penalties(64,1.05,0.0,0.0));

    // 添加采样策略
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(40));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.9,1) );
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.7) );
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(12345) );

    // 获取词汇表
    vocab = llama_model_get_vocab(model);
}

/**
 * @brief 构造函数（自定义参数）
 * @param path 模型文件路径
 * @param m_params 模型参数
 * @param c_params 上下文参数
 * @param p_params 惩罚参数
 * @param s_params 采样参数
 * 
 * 使用自定义参数加载模型和初始化上下文
 */
llama_wrapper::llama_wrapper(std::string path,struct init_model_params m_params,struct init_ctx_params c_params,penalties_params p_params,struct sampler_init_params s_params,int max_new_tokens)
{
    // 设置静默日志回调
    llama_log_set([](ggml_log_level, const char*, void*) {}, nullptr);
    // 引用计数为 0 时才初始化后端，避免多实例重复初始化
    if(backend_ref_count.fetch_add(1) == 0)
        llama_backend_init();

    // 配置模型参数
    model_params = llama_model_default_params();
    model_params.n_gpu_layers = m_params.n_gpu_layers;
    model_params.use_mmap = m_params.use_mmap;
    model_params.use_mlock = m_params.use_mlock;
    
    // 从文件加载模型
    model = llama_model_load_from_file(path.c_str(),model_params);
    
    // 检查模型是否加载成功
    if(!model)
    {
        std::cerr << "模型加载错误，检查模型路径 " << std::endl;
        throw std::runtime_error("model load failed ,check the path");
    }

    // 配置上下文参数
    struct llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = c_params.n_ctx;
    ctx_params.embeddings = c_params.embeddings;

    // 运行时自动检测 CPU 线程数，适配 2 核到多核的各种硬件
    // hardware_concurrency() 返回逻辑线程数（含超线程），最少保证 1
    unsigned int hw_threads = std::max(1u, std::thread::hardware_concurrency());
    // 生成阶段（逐 token 自回归）：用逻辑线程数的一半，接近物理核数，减少超线程竞争
    ctx_params.n_threads = std::max(1u, std::min((unsigned int)c_params.n_threads, hw_threads / 2 > 0 ? hw_threads / 2 : 1u));
    // prefill 批处理阶段：I/O 密集，用全部逻辑线程，但不超过配置上限
    ctx_params.n_threads_batch = std::min((unsigned int)c_params.n_threads_batch, hw_threads);

    // 从模型初始化上下文
    context = llama_init_from_model(model,ctx_params);
    
    save.ctx_params.n_ctx = c_params.n_ctx;
    save.ctx_params.embeddings = c_params.embeddings;
    save.ctx_params.n_threads = c_params.n_threads;
    save.ctx_params.n_threads_batch = c_params.n_threads_batch;
    save.max_new_tokens = max_new_tokens;
    // 检查上下文是否创建成功
    if(!context)
    {
        std::cerr << "上下文创建失败" << std::endl;
        llama_model_free(model);
        throw std::runtime_error("context create failed");
        
    }

    // 初始化采样器链
    sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());

    // 添加惩罚机制
    llama_sampler_chain_add(sampler, llama_sampler_init_penalties
        (
            p_params.penalty_last_n,
            p_params.penalty_repeat,
            p_params.penalty_freq,
            p_params.penalty_present
        ));

    // 添加采样策略
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(s_params.top_k));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(s_params.top_p_p,s_params.top_p_min) );
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(s_params.temp) );
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(s_params.dist) );

    // 获取词汇表
    vocab = llama_model_get_vocab(model);
}

/**
 * @brief 析构函数，释放所有资源
 */
llama_wrapper::~llama_wrapper()
{
    // 释放采样器
    if(sampler)llama_sampler_free(sampler);
    // 释放上下文
    if(context)llama_free(context);
    // 释放模型
    if(model)llama_model_free(model);
    // 最后一个实例析构时才释放后端
    if(backend_ref_count.fetch_sub(1) == 1)
        llama_backend_free();
}

/**
 * @brief 添加对话消息
 * @param role 角色（如 "user", "assistant"）
 * @param content 消息内容
 * 
 * 将消息添加到对话历史中
 */
void llama_wrapper::add_message(const std::string& role,const std::string& content)
{
    // 添加角色到角色列表
    this->role.push_back(role);
    // 添加内容到内容列表
    this->content.push_back(content);
    // 构造聊天消息并添加到消息列表
    message.push_back({this->role.back().c_str(),this->content.back().c_str()});
}

/**
 * @brief 生成 AI 回复
 * @return AI 生成的回复文本
 * 
 * 基于对话历史生成 AI 回复，最多生成 512 个 token
 */
std::string llama_wrapper::talk()
{
    std::string output;
    
    try
    {
        // 检查消息是否为空
        if(message.empty())
        {
            throw std::runtime_error("message is empty");
        }

        // 应用聊天模板，获取 prompt 长度
        int32_t len = llama_chat_apply_template(nullptr, message.data(),message.size() , true, nullptr, 0);
        // 分配内存
        std::string prompt(len + 1,'\0');
        // 应用聊天模板
        llama_chat_apply_template(nullptr, message.data(), message.size(), true, &prompt[0], len + 1);
        // 调整字符串大小
        prompt.resize(len);

        // 创建 token 列表
        std::vector<llama_token> all_tokens;

        // tokenize prompt
        int32_t n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.size(), nullptr, 0, true, true);
        
        // 调整 token 数量
        n_tokens = (n_tokens < 0) ? -n_tokens : n_tokens;

        // 调整 token 列表大小
        all_tokens.resize(n_tokens);
        // 执行 tokenize
        llama_tokenize(vocab, prompt.c_str(), prompt.size(), all_tokens.data(), n_tokens, true, true);

        // Prefill：一次性批量解码所有新增 token，充分利用 KV cache
        if(processed_pos < n_tokens)
        {
            llama_batch batch = llama_batch_get_one(&all_tokens[processed_pos], n_tokens - processed_pos);
            llama_decode(context, batch);
            processed_pos = n_tokens;
        }

        // 生成回复，上限由 AI_params::max_new_tokens 控制（JSON 响应通常 50-150 token）
        for(int i = 0;i < save.max_new_tokens;i++)
        {
            // 采样 token
            llama_token token = llama_sampler_sample(sampler, context, -1);
            // 接受采样结果
            llama_sampler_accept(sampler,  token);

            // 检查是否到达生成结束
            if(llama_vocab_is_eog(vocab,  token))break;

            // 将 token 转换为文本
            char piece[1024];
            int32_t piece_len = llama_token_to_piece(vocab, token,piece, sizeof(piece), 0, true);
            if(piece_len > 0)
            {
                // 追加到输出
                output.append(piece,piece_len);
            }

            // 解码生成的 token，processed_pos 持续递增，下次 talk() 可增量 decode
            llama_batch next = llama_batch_get_one(&token, 1);
            llama_decode(context, next);
            processed_pos++;
        }

        // 重置采样器（清除惩罚历史，避免影响下一轮生成）
        llama_sampler_reset(sampler);

        // 注意：不重置 processed_pos，下次 talk() 将从当前位置增量 decode 新增 token
        
        return output;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("talk error: ") + e.what());
    }
}

/**
 * @brief 清空对话消息历史
 * 
 * 清空所有对话消息并重置上下文
 */
void llama_wrapper::clear_message()
{
    // 重置已处理位置
    processed_pos = 0;
    // 清空历史 token
    history_tokens.clear();
    // 清空消息列表
    message.clear();
    // 清空角色列表
    role.clear();
    // 清空内容列表
    content.clear();

    // 释放旧上下文
    llama_free(context);

    // 重新配置上下文参数
    struct llama_context_params ctx_params = llama_context_default_params();
    
    ctx_params.n_ctx = save.ctx_params.n_ctx;
    ctx_params.n_threads = save.ctx_params.n_threads;
    ctx_params.n_threads_batch = save.ctx_params.n_threads_batch;
    ctx_params.embeddings = save.ctx_params.embeddings;
    
    // 重新初始化上下文
    context = llama_init_from_model(model, ctx_params);
    
    // 检查上下文是否创建成功
    if(!context)
    {
        throw std::runtime_error("Failed to reinitialize context in clear_message");
    }

    // 重置采样器
    llama_sampler_reset(sampler);
}