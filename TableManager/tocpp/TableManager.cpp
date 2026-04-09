
//Generate By Tool. Please Do Not Edit It!
#include "TableManager.h"
#include "TableActivityData.h" 
#include "TableRankData.h" 
#include "TableCopySceneData.h" 

void TableManager::ReadAllTable()
{
	TableManager::ReadTable<TableActivityData>();
	TableManager::ReadTable<TableRankData>();
	TableManager::ReadTable<TableCopySceneData>();
}

namespace TableManager
{
    std::unordered_map<std::type_index, std::shared_ptr<void>> g_TableManager;  // 定义
}
