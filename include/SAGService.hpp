#pragma once
#include "IRepository.hpp"
#include"SAGRepository.hpp"
#include"model.hpp"
#include"std.hpp"
#include <utility>
#include <vector>

struct SAGQuery
{
    std::string token;
    std::string value;
    std::string id;
};

struct SAGDetail
{
    std::string Name = "";
    std::string Class = "" ;
    std::string Phone = "";
    std::string Remark = "";
    std::string Subject = "";
    std::string Gender = "";
    std::string Student_Id = "";
    std::string ID = "";
    std::string Create_time = "";
    std::string Description = "";
};

struct SAGResult
{
    enum class Status
    {
        SUCCESS,
        INVALID_INPUT,
        ERROR,
        FAILURE
    };
    SAGResult::Status status;
    std::string message;
    std::vector<SAGDetail> data;
    static SAGResult success(std::string message)
    {
        return 
        {
            .status = SAGResult::Status::SUCCESS,
            .message = std::move(message),
            .data = {}
        };
    }
    static SAGResult success(std::string message,std::vector<SAGDetail> data)
    {
        return
        {
            .status = SAGResult::Status::SUCCESS,
            .message = std::move(message),
            .data = std::move(data)
        };
    }
    static SAGResult failure(std::string message)
    {
        return
        {
            .status = SAGResult::Status::FAILURE,
            .message = std::move(message),
            .data = {}
        };
    }
    static SAGResult invalidInput(std::string message)
    {
        return
        {
            .status = SAGResult::Status::INVALID_INPUT,
            .message = std::move(message),
            .data = {}
        };
    }
    static SAGResult error(std::string message)
    {
        return
        {
            .status = SAGResult::Status::ERROR,
            .message = std::move(message),
            .data = {}
        };
    }

};


class SAGService
{
    private:
    IRepository<SAG>& sag_repo;
    IRepository<Student>& stu_repo;
    IRepository<Group>& grp_repo;
    public:
    explicit SAGService(IRepository<SAG>& sag_repo,IRepository<Student>& stu_repo,IRepository<Group>& grp_repo);
    
    SAGResult addSAG(const SAG& sag);
    SAGResult removeSAG(const std::string& id);
    SAGResult updateSAG(const SAGQuery& query);
    SAGResult searchSAG(const SAGQuery& query);
    
};