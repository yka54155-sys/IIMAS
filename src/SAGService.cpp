#include"SAGService.hpp"
#include "IRepository.hpp"
#include "model.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <vector>

SAGService::SAGService(IRepository<SAG>& sag_repo,IRepository<Student>& stu_repo,IRepository<Group>& grp_repo) : sag_repo(sag_repo) , stu_repo(stu_repo) , grp_repo(grp_repo)
{
    ;
}

/*
using Status = SAGResult::Status;
try
{
    
}
catch (const std::exception& e)
{
    std::cerr << "" << e.what() << '\n';
    return 
    {
        .status = Status::ERROR,
        .message = "SAG() ERROR , please check SAGService.",
        .data = {}
    };
}
*/


SAGResult SAGService::addSAG(const SAG& sag)
{
    
    try
    {
        if(sag.Group_Id <= 0 || sag.Student_Id <= 0)
        {
             return SAGResult::invalidInput("addSAG: InvalidInput;");
        }
        int m_stu = stu_repo.search("id", std::to_string(sag.Student_Id)).size();
        int m_grp = grp_repo.search("id",std::to_string(sag.Group_Id)).size();
        if(!m_grp || !m_stu)
        {
            return SAGResult::invalidInput("addSAG: InvalidInput , not found input student_id or group_id.");
        }
        if(!sag_repo.add(sag))
        {
            return SAGResult::error("addSAG: add Failure , check the SAGRepository.add().");
        }

        return SAGResult::success("addSAG: success");
    }   
    catch (const std::exception& e)
    {
        std::cerr << "[SAGService::addSAG] " << e.what() << '\n';
        return SAGResult::error("SAG() ERROR , please check SAGService.");
    }
}
SAGResult SAGService::removeSAG(const std::string& id)
{
    
    try
    {
        if(id.empty())
        {
            return SAGResult::invalidInput("removeSAG: InvalidInput id.");
        }
        if(!sag_repo.exist(id))
        {
            return SAGResult::failure("removeSAG: Failure,sag id not exist.");
        }

        if(!sag_repo.remove(id))
        {
            return SAGResult::error("removeSAG: Error,check the SAGRepository.remove.");
        }
        return SAGResult::success("removeSAG: success.");
    }   
    catch (const std::exception& e)
    {
        std::cerr << "[SAGService::removeSAG] " << e.what() << '\n';
        return SAGResult::error("SAG() ERROR , please check SAGService.");
    }

}
SAGResult SAGService::updateSAG(const SAGQuery& query)
{
    try
    {
        if(query.id.empty() || query.id == "#")
        {
            return SAGResult::invalidInput("updateSAG: id param is empty");
        }
        if (SAGMap.find(query.token) == SAGMap.end())
        {
            return SAGResult::invalidInput("updateSAG: unknown token");
        }
        if (!sag_repo.exist(query.id))
        {
            return SAGResult::failure("updateSAG: SAG record not found");
        }
        if (!sag_repo.update(query.token, query.value, query.id))
        {
            return SAGResult::error("updateSAG: database error, check SAGRepository.update()");
        }
        return SAGResult::success("updateSAG: success");
    }
    catch (const std::exception& e)
    {
        std::cerr << "[SAGService::updateSAG] " << e.what() << '\n';
        return SAGResult::error("updateSAG: internal error");
    }
}

SAGResult SAGService::searchSAG(const SAGQuery& query)
{
    try
    {
        std::optional<int> ID;
        if(query.token.empty() || query.value.empty())
        {
            return SAGResult::invalidInput("searchSAG: token or value is empty");
        }
        auto it = SAGMap.find(query.token);
        if (it == SAGMap.end())
        {
            return SAGResult::invalidInput("searchSAG: unknown token");
        }
        
        switch(it->second)
        {
            case SAGToken::STUDENT_ID:
                ID = stu_repo.getDatabaseID(query.value);
                break;
            case SAGToken::GROUP_ID:
                ID = grp_repo.getDatabaseID(query.value);
                break;
            default:
                return SAGResult::invalidInput("searchSAG: unknown token");
        }
        if(ID == std::nullopt)
        {
            return SAGResult::failure("searchSAG: no match value result");
        }

        std::vector<SAG> sag_vec = sag_repo.search(query.token,std::to_string(ID.value()));
        if(sag_vec.empty())
        {
            return SAGResult::failure("searchSAG: no match value results");
        }
        std::vector<SAGDetail> result;
        switch (it->second)
        {
            case SAGToken::STUDENT_ID:
                for(const auto& sag : sag_vec)
                {
                    SAGDetail temp;
                    std::vector<Group> temp_grp = grp_repo.search("id", std::to_string(sag.Group_Id));
                    if(temp_grp.size() == 1)
                    {
                        temp.Name = temp_grp.front().Group_Name;    
                        temp.Create_time = temp_grp.front().Create_Time;
                        temp.Description = temp_grp.front().Description;
                        temp.ID = std::to_string(temp_grp.front().Id); 
                         
                    }
                    else
                    {
                        return SAGResult::failure("searchSAG: Database Error");
                    }
                    result.emplace_back(temp);
                }
                break;
            case SAGToken::GROUP_ID:
                for(const auto& sag : sag_vec)
                {
                    SAGDetail temp;
                    std::vector<Student> temp_stu = stu_repo.search("id", std::to_string(sag.Student_Id));
                    if(temp_stu.size() == 1)
                    {
                        temp.Name = temp_stu.front().Name;
                        temp.Student_Id = temp_stu.front().Student_Id;
                        temp.Subject = temp_stu.front().Subject;
                        temp.Class = std::to_string(temp_stu.front().Class);
                        temp.Create_time = temp_stu.front().Create_Time;
                        temp.Gender = temp_stu.front().Gender;
                        temp.Remark = sag.Remark;
                        temp.Phone = temp_stu.front().Phone;
                        temp.ID = std::to_string(temp_stu.front().Id); 
                    }
                    else
                    {
                        return SAGResult::failure("searchSAG: Database Error");
                    }
                    result.emplace_back(temp);
                }
                break;
            default:
                return SAGResult::invalidInput("searchSAG: unknown token");
        }
        return SAGResult::success("searchSAG: search success", std::move(result));
    }
    catch (const std::exception& e)
    {
        std::cerr << "[SAGService::searchSAG] " << e.what() << '\n';
        return SAGResult::error("searchSAG: internal error");
    }
}