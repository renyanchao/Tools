---Generate By Tool. Please Do Not Edit It!

local TableManager = {}
TableManager.__TableList = {}


TableManager.__TableList.TableActivityData = require "TableActivityData"
function TableManager:GetTableActivityDataById(Id)
    for i = 1, self:GetTableActivityDataCount() do
        if self:GetTableActivityDataByIndex(i).Id == Id then
            return self:GetTableActivityDataByIndex(i)
        end
    end
    return nil
end
function TableManager:GetTableActivityDataByIndex(index)
    return self.__TableList.TableTableActivityDataData[index]
end
function TableManager:GetTableActivityDataCount()
    return #self.__TableList.TableActivityData
end

TableManager.__TableList.TableRankData = require "TableRankData"
function TableManager:GetTableRankDataById(Id)
    for i = 1, self:GetTableRankDataCount() do
        if self:GetTableRankDataByIndex(i).Id == Id then
            return self:GetTableRankDataByIndex(i)
        end
    end
    return nil
end
function TableManager:GetTableRankDataByIndex(index)
    return self.__TableList.TableTableRankDataData[index]
end
function TableManager:GetTableRankDataCount()
    return #self.__TableList.TableRankData
end

TableManager.__TableList.TableCopySceneData = require "TableCopySceneData"
function TableManager:GetTableCopySceneDataById(Id)
    for i = 1, self:GetTableCopySceneDataCount() do
        if self:GetTableCopySceneDataByIndex(i).Id == Id then
            return self:GetTableCopySceneDataByIndex(i)
        end
    end
    return nil
end
function TableManager:GetTableCopySceneDataByIndex(index)
    return self.__TableList.TableTableCopySceneDataData[index]
end
function TableManager:GetTableCopySceneDataCount()
    return #self.__TableList.TableCopySceneData
end


return TableManager