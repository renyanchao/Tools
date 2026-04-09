#include "tocpp/TableManager.h"
#include<iostream>

#include "tocpp/TableActivityData.h"
#include "tocpp/TableCopySceneData.h"

int main()
{
    TableManager::ReadAllTable();

    const TableActivityData* pTableActivityData = TableManager::GetDataById<TableActivityData>(1);
    const TableCopySceneData* pTableCopySceneData = TableManager::GetDataById<TableCopySceneData>(1);
    std::cout<<"hello world"<<std::endl;
}