#include "Homestead/UI/PauseUI.hpp"
#include "Homestead/UI/StatusUI.hpp"
#include "Homestead/UI/MarketUI.hpp"

int main() {
    if (Homestead::PauseItemAt(68, 48) != 0 ||
        Homestead::PauseItemAt(251, 145) != 6 ||
        Homestead::PauseItemAt(67, 48) != -1 ||
        Homestead::PauseItemAt(252, 48) != -1 ||
        Homestead::PauseItemAt(68, 47) != -1 ||
        Homestead::PauseItemAt(68, 146) != -1) return 1;
    if (!Homestead::CompletionContinueAt(124, 90) ||
        !Homestead::CompletionContinueAt(195, 107) ||
        Homestead::CompletionContinueAt(123, 90) ||
        Homestead::CompletionContinueAt(196, 107) ||
        Homestead::CompletionContinueAt(124, 108)) return 2;
    if(Homestead::MarketItemAt(70,62)!=0||Homestead::MarketItemAt(249,151)!=5||
       Homestead::MarketItemAt(69,62)!=-1||Homestead::MarketItemAt(70,152)!=-1)return 3;
    if(Homestead::UpdateMarketFocus(2,true,false,true,80,62)!=1||
       Homestead::UpdateMarketFocus(2,false,true,true,80,62)!=3||
       Homestead::UpdateMarketFocus(2,false,false,true,80,62)!=0)return 4;
    return 0;
}
