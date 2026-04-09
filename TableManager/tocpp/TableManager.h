#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <typeindex>
#include <memory>
#include <cstdint>
namespace TableManager
{
    extern std::unordered_map<std::type_index, std::shared_ptr<void>> g_TableManager;

    void ReadAllTable();

    template<typename T>
    const T* GetDataById(int32_t Id)
    {
        auto it = g_TableManager.find(std::type_index(typeid(T)));
        if(it == g_TableManager.end())
        {
            return nullptr;
        }

        auto typeMap = std::static_pointer_cast<std::unordered_map<int32_t, T>>(it->second);
        auto it_tab = typeMap->find(Id);
        return it_tab != typeMap->end()? &it_tab->second : nullptr;
    }

    template<typename T>
    bool ReadTable()
    {
        std::ifstream file("../totxt/" + T::GetFileName());
        if(!file.is_open())
        {
            std::cout<<"error. 无法打开文件"<<std::endl;
            return false;
        }
        std::string line;
        bool is_first_line = true;
        auto tableMap = std::make_shared<std::unordered_map<int32_t, T>>();

        while(std::getline(file, line))
        {
            if(line.empty())continue;
            if(is_first_line){is_first_line = false; continue;}

            std::vector<std::string> fields;
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, '\t')) {
                fields.push_back(cell);
            }
            T table_data;
            table_data.ReadLine(fields);
            (*tableMap)[table_data.Id] = std::move(table_data);
        }

        g_TableManager[std::type_index(typeid(T))] = tableMap;
        return true;
    }
};

#endif