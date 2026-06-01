#pragma once
#include"llama.h"
#include <cstddef>
#include<string>
#include<vector>
#include<iostream>

/**
 * @brief 采样器初始化参数结构体
 * 
 * 用于配置文本生成时的采样策略
 */
typedef struct sampler_init_params
{
    int top_k;           // Top-K 采样参数
    double top_p_min;    // Top-P 最小值
    double top_p_p;      // Top-P 核心值
    double temp;         // 温度参数
    int dist;            // 随机种子
}sampler_init_params;

/**
 * @brief 惩罚参数结构体
 * 
 * 用于配置文本生成时的惩罚机制
 */
typedef struct penalties_params
{
    int penalty_last_n;     // 惩罚最后 N 个 token
    double penalty_repeat;   // 重复惩罚系数
    double penalty_freq;     // 频率惩罚系数
    double penalty_present;  // 存在惩罚系数
}penalties_params;

struct init_model_params 
{                                   
      int n_gpu_layers = 0;                                          
      bool use_mmap = true;                                          
      bool use_mlock = false;                                        
};                                                                 
                                                                     
struct init_ctx_params 
{                                     
    int n_ctx = 4096;                                              
    int n_threads = 4;                                             
    int n_threads_batch = 8;                                       
    bool embeddings = false;                                       
};

struct save_params
{
    init_ctx_params ctx_params;
    int max_new_tokens;
};

/**
 * @brief llama.cpp 封装类
 * 
 * 封装 llama.cpp 的核心功能，提供简单的 API 进行对话消息管理和文本生成
 */
class llama_wrapper
{
    private:
    struct llama_model* model = nullptr;           // llama 模型指针
    const struct llama_vocab* vocab = nullptr;      // llama 词汇表指针
    struct llama_context* context = nullptr;       // llama 上下文指针
    struct llama_sampler* sampler = nullptr;       // llama 采样器指针
    struct llama_model_params model_params;         // 模型参数
    int processed_pos = 0;                         // 已处理的位置
    std::vector<llama_token> history_tokens;       // 历史 token 列表
    std::vector<llama_chat_message> message;       // 对话消息列表
    std::vector<std::string> role;                 // 角色列表
    std::vector<std::string>content;               // 内容列表
    save_params save;
    
    public:
    /**
     * @brief 构造函数（默认参数）
     * @param path 模型文件路径
     */
    llama_wrapper(std::string path);
    
    /**
     * @brief 构造函数（自定义参数）
     * @param path 模型文件路径
     * @param m_params 模型参数
     * @param c_params 上下文参数
     * @param p_params 惩罚参数
     * @param s_params 采样参数
     */
    llama_wrapper(std::string path,struct init_model_params,struct init_ctx_params,penalties_params,struct sampler_init_params,int max_new_tokens);
    
    /**
     * @brief 禁止拷贝构造
     */
    llama_wrapper(const llama_wrapper&) = delete;
    
    /**
     * @brief 禁止拷贝赋值
     */
    llama_wrapper& operator=(const llama_wrapper&) = delete;
    
    /**
     * @brief 析构函数，释放资源
     */
    ~llama_wrapper();
    
    /**
     * @brief 添加对话消息
     * @param role 角色（如 "user", "assistant"）
     * @param content 消息内容
     */
    void add_message(const std::string& role,const std::string& content);
    
    /**
     * @brief 生成 AI 回复
     * @return AI 生成的回复文本
     */
    std::string talk();
    
    
    
    /**
     * @brief 清空对话消息历史
     */
    void clear_message();
    
};