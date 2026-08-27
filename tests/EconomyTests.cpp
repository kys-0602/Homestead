#include "Homestead/Game/Economy.hpp"

int main() {
    Homestead::Inventory inventory; std::uint16_t gold=20;
    if(!Homestead::BuySeed(inventory,gold,0)||gold!=16||inventory.Count(Homestead::ItemId::WheatSeed)!=1)return 1;
    if(!inventory.Remove(Homestead::ItemId::WheatSeed,1)||inventory.Add(Homestead::ItemId::Wheat,3)!=0)return 2;
    if(!Homestead::SellHarvest(inventory,gold,0)||gold!=37||inventory.Count(Homestead::ItemId::Wheat)!=0)return 3;
    gold=0;if(Homestead::BuySeed(inventory,gold,2)||Homestead::SellHarvest(inventory,gold,2))return 4;
    gold=20;
    for(unsigned cycle=0;cycle<3&&gold<Homestead::GoalGold;++cycle){
        std::uint16_t bought=0;while(Homestead::BuySeed(inventory,gold,2))++bought;
        if(bought==0||inventory.Add(Homestead::ItemId::Tomato,bought)!=0||
           !Homestead::SellHarvest(inventory,gold,2))return 5;
        [[maybe_unused]] const bool removed=inventory.Remove(Homestead::ItemId::TomatoSeed,bought);
    }
    if(gold<Homestead::GoalGold)return 6;
    inventory.Clear();gold=65530;
    if(inventory.Add(Homestead::ItemId::Wheat,1)!=0||
       Homestead::SellHarvest(inventory,gold,0)||gold!=65530||
       inventory.Count(Homestead::ItemId::Wheat)!=1)return 7;
    inventory.Clear(); gold=0;
    if(inventory.Add(Homestead::ItemId::Wheat,2,Homestead::ItemQuality::Silver)!=0||
       inventory.Add(Homestead::ItemId::Wheat,1,Homestead::ItemQuality::Gold)!=0||
       !Homestead::SellHarvest(inventory,gold,0)||gold!=27)return 8;
    return 0;
}
