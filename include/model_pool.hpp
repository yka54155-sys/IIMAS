#pragma once
#include "AI_params.hpp"
#include"llama.h"
#include"llama_wrapper.hpp"
#include <mutex>
#include<vector>
#include<memory>
#include<thread>
#include<queue>

struct model_set
{
    llama_wrapper* intent_model;
    llama_wrapper* query_model;
    llama_wrapper* summary_model;
    bool exist() const                                                                                       
    {                                                                                                        
        return intent_model != nullptr && query_model != nullptr && summary_model != nullptr;                
    }
};

/**
 * @brief AI 模型池类
 * 
 * 管理多个 AI 模型实例，支持并发访问，使用互斥锁保护并发安全
 */
class model_pool
{
    private:    
        std::vector<std::unique_ptr<llama_wrapper>> intent_models;
        std::vector<std::unique_ptr<llama_wrapper>> query_models;
        std::vector<std::unique_ptr<llama_wrapper>> summary_models;    // 模型实例列表
        std::queue<int> available_intent_models;
        std::queue<int> available_query_models;
        std::queue<int> available_summary_models;                       // 可用模型索引队列
        std::mutex pool_mutex;                                  // 模型池互斥锁
        
    public:
    /**
     * @brief 构造函数
     * 
     * 根据 AI_params::pool_size 创建指定数量的模型实例
     */
    model_pool();
    
    /**
     * @brief 获取可用的模型实例
     * @return 模型实例指针，如果没有可用模型则返回 nullptr
     */
    model_set acquire();
    
    /**
     * @brief 释放模型实例
     * @param model 要释放的模型实例指针
     * @return 释放是否成功
     */
    bool release(model_set);
        
};