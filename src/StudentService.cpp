#include"StudentService.hpp"
#include "IRepository.hpp"
#include "model.hpp"
#include <iostream>
#include <vector>

StudentService::StudentService(IRepository<Student>& stu_repo) : stu_repo(stu_repo)
{
    ;
}

StudentResult StudentService::addStudent(const std::vector<Student>& stus)
{
    try
    {
        if(stus.empty())
        {
            return StudentResult::invalidInput("addStudent: the Students array is empty");
        }
        for(const auto& stu : stus)
        {
            if(stu.Name.empty() || stu.Student_Id.empty())
            {
                return StudentResult::invalidInput("addStudent: required filed is empty ");
            }
            if(stu_repo.exist(stu.Student_Id))
            {
                return StudentResult::invalidInput("addStudent: student_id is existed ");
            }
        }
        if(!stu_repo.addBatch(stus))
        {
            return StudentResult::error("addStudent: database error, check StudentRepository.addBatch()");
        }
        return StudentResult::success("addStudent: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[StudentService::addStudent] " << e.what() << '\n';
        return StudentResult::error("addStudent: internal error");
    }
}

StudentResult StudentService::removeStudent(const std::string& id)
{
    try
    {
        if(id.empty())
        {
            return StudentResult::invalidInput("removeStudent: id is empty");
        }
        if(!stu_repo.exist(id))
        {
            return StudentResult::failure("removeStudent: student not found");
        }
        if(!stu_repo.remove(id))
        {
            return StudentResult::error("removeStudent: database error, check StudentRepository.remove()");
        }
        return StudentResult::success("removeStudent: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[StudentService::removeStudent] " << e.what() << '\n';
        return StudentResult::error("removeStudent: internal error");
    }
}

StudentResult StudentService::updateStudent(const StudentQuery& query)
{
    try
    {
        if(query.id.empty() || query.id == "#")
        {
            return StudentResult::invalidInput("updateStudent: id param is empty");
        }
        if (StudentMap.find(query.token) == StudentMap.end())
        {
            return StudentResult::invalidInput("updateStudent: unknown token");
        }
        if (!stu_repo.exist(query.id))
        {
            return StudentResult::failure("updateStudent: student not found");
        }
        if (!stu_repo.update(query.token, query.value, query.id))
        {
            return StudentResult::error("updateStudent: database error, check StudentRepository.update()");
        }
        return StudentResult::success("updateStudent: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[StudentService::updateStudent] " << e.what() << '\n';
        return StudentResult::error("updateStudent: internal error");
    }
}

StudentResult StudentService::searchStudent(const StudentQuery& query)
{
    try
    {
        if(query.token.empty() || query.value.empty())
        {
            return StudentResult::invalidInput("searchStudent: token or value is empty");
        }
        if (StudentMap.find(query.token) == StudentMap.end())
        {
            return StudentResult::invalidInput("searchStudent: unknown token");
        }

        std::vector<Student> result;
        switch (query.mode)
        {
            case StudentQuery::SearchMode::EXACT:
                result = stu_repo.search(query.token, query.value);
                break;
            case StudentQuery::SearchMode::FUZZY:
                result = stu_repo.fuzzySearch(query.token, query.value);
                break;
            default:
                return StudentResult::invalidInput("searchStudent: invalid search mode");
        }
        if(result.empty())
        {
            return StudentResult::failure("searchStudent: no results");
        }
        return StudentResult::success("searchStudent: search success", std::move(result));
    }
    catch (const std::exception& e)
    {
        std::cerr << "[StudentService::searchStudent] " << e.what() << '\n';
        return StudentResult::error("searchStudent: internal error");
    }
}
