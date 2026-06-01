#pragma once
#include "IRepository.hpp"
#include"std.hpp"
#include"model.hpp"

struct StudentQuery
{
    std::string token;
    std::string value;
    std::string id = "#";
    enum class SearchMode
    {
        NONE,
        EXACT,
        FUZZY
    };
    SearchMode mode = StudentQuery::SearchMode::NONE;
};
struct StudentResult
{
    enum class Status
    {
        SUCCESS,
        FAILURE,
        INVALID_INPUT,
        ERROR
    };
    Status status;
    std::string message;
    std::vector<Student> data;

    static StudentResult success(std::string msg) {
        return { Status::SUCCESS, std::move(msg), {} };
    }
    static StudentResult success(std::string msg, std::vector<Student> data) {
        return { Status::SUCCESS, std::move(msg), std::move(data) };
    }
    static StudentResult failure(std::string msg) {
        return { Status::FAILURE, std::move(msg), {} };
    }
    static StudentResult invalidInput(std::string msg) {
        return { Status::INVALID_INPUT, std::move(msg), {} };
    }
    static StudentResult error(std::string msg) {
        return { Status::ERROR, std::move(msg), {} };
    }
};

class StudentService
{
    private:
    IRepository<Student>& stu_repo;
    public:
    explicit StudentService(IRepository<Student>& stu_repo);

    StudentResult addStudent(const std::vector<Student>& stus);
    StudentResult removeStudent(const std::string& id);
    StudentResult updateStudent(const StudentQuery& query);
    StudentResult searchStudent(const StudentQuery& query);

};
