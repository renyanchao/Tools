
//Generate By Tool. Please Do Not Edit It!
#include"TableManager.h"

struct TableActivityData
{
		int32_t	Id;
		std::string	Name;
		bool	IsClose;
		float	Rate;
		int32_t	RewardItem1;
		int32_t	RewardItem2;
		int32_t	RewardItem3;

        
    void ReadLine(const std::vector<std::string>& fields)
    {
			Id = std::stoi(fields[0]);
			Name = std::string(fields[1]);
			IsClose = std::stoi(fields[2]) == 1;
			Rate = std::stof(fields[3]);
			RewardItem1 = std::stoi(fields[4]);
			RewardItem2 = std::stoi(fields[5]);
			RewardItem3 = std::stoi(fields[6]);

    }
    const static std::string GetFileName()
    {
        return "Activity.txt";
    }
};
