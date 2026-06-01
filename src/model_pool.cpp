#include"model_pool.hpp"
#include "AI_params.hpp"
#include "llama_wrapper.hpp"
#include <cstddef>
#include <memory>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include"crow.h"

/**
 * @brief 构造函数
 * 
 * 根据 AI_params::pool_size 创建指定数量的模型实例
 */
model_pool::model_pool()
{
    try
    {
        // 获取模型池大小
        int size = AI_params::pool_size;
        // 预留模型列表空间
        intent_models.reserve(size);
        query_models.reserve(size);
        summary_models.reserve(size);
        
        CROW_LOG_INFO << "[Model Pool] Initializing, pool size: " << size;
        
        // 创建指定数量的模型实例
        for(int i  = 0;i < size;i++)
        {
            CROW_LOG_INFO << "[Model Pool] Loading intent_model " << (i + 1) << "/" << size << "...";
            
            // 将模型索引加入可用队列
            available_intent_models.push(i);
            
            // 创建模型实例并添加到模型列表
            intent_models.emplace_back(std::make_unique<llama_wrapper>
                (
                    AI_params::IntentParams::path,
                    AI_params::IntentParams::model_params,
                    AI_params::IntentParams::ctx_params,
                    AI_params::IntentParams::penal_params,
                    AI_params::IntentParams::sam_params,
                    AI_params::IntentParams::max_new_tokens
                ));
            
            CROW_LOG_INFO << "[Model Pool] Intent_model " << (i + 1) << " loaded successfully";
        }
        
        CROW_LOG_INFO << "[Model Pool] All Intent_model loaded successfully!";

        for(int i  = 0;i < size;i++)
        {
            CROW_LOG_INFO << "[Model Pool] Loading query_model " << (i + 1) << "/" << size << "...";
            
            // 将模型索引加入可用队列
            available_query_models.push(i);
            
            // 创建模型实例并添加到模型列表
            query_models.emplace_back(std::make_unique<llama_wrapper>
                (
                    AI_params::QueryParams::path,
                    AI_params::QueryParams::model_params,
                    AI_params::QueryParams::ctx_params,
                    AI_params::QueryParams::penal_params,
                    AI_params::QueryParams::sam_params,
                    AI_params::QueryParams::max_new_tokens
                ));
            
            CROW_LOG_INFO << "[Model Pool] Query_model " << (i + 1) << " loaded successfully";
        }
        
        CROW_LOG_INFO << "[Model Pool] All Query_model loaded successfully!";

        for(int i  = 0;i < size;i++)
        {
            CROW_LOG_INFO << "[Model Pool] Loading Summary_model " << (i + 1) << "/" << size << "...";
            
            // 将模型索引加入可用队列
            available_summary_models.push(i);
            
            // 创建模型实例并添加到模型列表
            summary_models.emplace_back(std::make_unique<llama_wrapper>
                (
                    AI_params::SummaryParams::path,
                    AI_params::SummaryParams::model_params,
                    AI_params::SummaryParams::ctx_params,
                    AI_params::SummaryParams::penal_params,
                    AI_params::SummaryParams::sam_params,
                    AI_params::SummaryParams::max_new_tokens
                ));
            
            CROW_LOG_INFO << "[Model Pool] Summary_model " << (i + 1) << " loaded successfully";
        }
        
        CROW_LOG_INFO << "[Model Pool] All Summary_model loaded successfully!";


    }
    catch(const std::exception& e)
    {
        std::cerr << "[模型池] 初始化失败: " << e.what() << std::endl;
        std::cerr << "[模型池] 请检查模型文件路径: /bin/model" << std::endl;
        throw;
    }
    catch(...)
    {
        std::cerr << "[模型池] 初始化失败: 未知错误" << std::endl;
        throw std::runtime_error("模型池初始化失败");
    }
}

/**
 * @brief 获取可用的模型实例
 * @return 模型实例指针，如果没有可用模型则返回 nullptr
 * 
 * 从模型池中获取一个可用的模型实例
 */
model_set model_pool::acquire()
{
    model_set result{.intent_model = nullptr,.query_model = nullptr,.summary_model = nullptr};
    // 加锁保护
    std::lock_guard<std::mutex> lock(pool_mutex);
    
    // 检查是否有可用模型
    if(available_intent_models.empty() || available_query_models.empty() || available_summary_models.empty())
    {
        return result;
    }
    
    // 获取可用模型的索引
    int intent_idx  = available_intent_models.front();
    // 从队列中移除该索引
    available_intent_models.pop();
    result.intent_model = intent_models[intent_idx].get();

    // 获取可用模型的索引
    int query_idx  = available_query_models.front();
    // 从队列中移除该索引
    available_query_models.pop();
    result.query_model = query_models[query_idx].get();

    // 获取可用模型的索引
    int summary_idx  = available_summary_models.front();
    // 从队列中移除该索引
    available_summary_models.pop();
    result.summary_model = summary_models[summary_idx].get();
    

    // 返回模型实例指针
    // return models[idx].get();

    return result;
}

/**
 * @brief 释放模型实例
 * @param model 要释放的模型实例指针
 * @return 释放是否成功
 * 
 * 将模型实例归还到模型池，并清空其消息历史
 */
bool model_pool::release(model_set set)
{
    // 加锁保护
    std::lock_guard<std::mutex> lock(pool_mutex);
    
    // 检查模型指针是否有效
    if(!set.intent_model && !set.query_model && !set.summary_model) return false;
    
    // 遍历模型列表，找到对应的模型实例
    for(size_t i = 0;i < intent_models.size();i++)
    {
        if(intent_models[i].get() == set.intent_model)
        {
            // 清空模型的消息历史
            set.intent_model->clear_message();
            // 将模型索引加入可用队列
            available_intent_models.push(i);
            set.intent_model = nullptr;     
        }
    }
    // 遍历模型列表，找到对应的模型实例
    for(size_t i = 0;i < query_models.size();i++)
    {
        if(query_models[i].get() == set.query_model)
        {
            // 清空模型的消息历史
            set.query_model->clear_message();
            // 将模型索引加入可用队列
            available_query_models.push(i);
            set.query_model = nullptr;     
        }
    }
    // 遍历模型列表，找到对应的模型实例
    for(size_t i = 0;i < summary_models.size();i++)
    {
        if(summary_models[i].get() == set.summary_model)
        {
            // 清空模型的消息历史
            set.summary_model->clear_message();
            // 将模型索引加入可用队列
            available_summary_models.push(i);
            set.summary_model = nullptr;     
        }
    }

    if(!set.intent_model && !set.query_model && !set.summary_model) return true;
    
    return false;
}