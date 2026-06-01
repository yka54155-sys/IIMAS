#pragma once
#include"std.hpp"
#include <mutex>
#include <random>
#include <shared_mutex>

class TokenStore
{
    private:
    TokenStore() = default;
    std::unordered_set<std::string> tokens;
    mutable std::shared_mutex mutex;
    public:
    static TokenStore& instance()
    {
        static TokenStore ins;
        return ins;
    }

    void insert(const std::string& token)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        tokens.insert(token);
    }
    bool erase(const std::string& token)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        return tokens.erase(token);
    }
    bool contains(const std::string& token) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return tokens.contains(token);
    }

    void clear()
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        tokens.clear();
    }

    std::string generate_token()                                                                             
    {                          
                                                                                     
        thread_local std::mt19937 rng([]
            {
                std::random_device rd;
                std::seed_seq seq{rd(),rd(),rd(),rd()};
                return std::mt19937(seq);
            }());
        
                                        
        
        constexpr const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::string token(32, ' ');
        static std::uniform_int_distribution<> dist(0, sizeof(chars) - 2);                                                                            
        for (auto& c : token) c = chars[dist(rng)];                                                          
        return token;                                                                                        
    }
    
    bool empty()
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return tokens.empty();
    }

};