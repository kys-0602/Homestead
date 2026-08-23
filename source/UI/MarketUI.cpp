#include "Homestead/UI/MarketUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/Economy.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"

namespace Homestead {
namespace {
constexpr std::uint32_t X=70,Y=34,W=180,RowY=62,RowH=15;

void Append(char* text,std::size_t& length,const char* value) noexcept {
    while(*value!='\0')text[length++]=*value++;
}

void AppendNumber(char* text,std::size_t& length,unsigned value) noexcept {
    char digits[5]{};std::size_t count=0;
    do{digits[count++]=static_cast<char>('0'+value%10U);value/=10U;}while(value!=0);
    while(count!=0)text[length++]=digits[--count];
}
}
int MarketItemAt(std::uint32_t x,std::uint32_t y) noexcept {
    if(x<X||x>=X+W||y<RowY||y>=RowY+MarketItemCount*RowH)return -1;
    return static_cast<int>((y-RowY)/RowH);
}
std::uint8_t UpdateMarketFocus(std::uint8_t focus,bool up,bool down,bool mouseValid,
                               std::uint32_t mouseX,std::uint32_t mouseY) noexcept {
    if(up)return focus==0?MarketItemCount-1:static_cast<std::uint8_t>(focus-1);
    if(down)return static_cast<std::uint8_t>((focus+1)%MarketItemCount);
    if(mouseValid){const int hit=MarketItemAt(mouseX,mouseY);if(hit>=0)return static_cast<std::uint8_t>(hit);}
    return focus;
}
bool AddMarketUI(std::uint16_t gold,std::uint8_t focus,const AssetStore& assets,RenderQueue& queue) noexcept {
    const SpriteAsset* pixel=assets.FindSprite(MakeAssetId("terrain.grass"));
    const SpriteAsset* pointer=assets.FindSprite(MakeAssetId("ui.pointer.idle"));
    if(!pixel||!pointer)return false;
    SpriteCommand panel{};panel.x=static_cast<float>(X);panel.y=static_cast<float>(Y);
    panel.width=static_cast<float>(W);panel.height=124.0F;
    panel.uvX=pixel->x;panel.uvY=pixel->y;panel.uvWidth=pixel->width;panel.uvHeight=pixel->height;
    panel.color=0xE0203020U;panel.layer=SpriteLayer::UI;panel.depth=40;if(!queue.Add(panel))return false;
    char money[]="GOLD 00000"; unsigned value=gold;
    for(int i=9;i>=5;--i){money[i]=static_cast<char>('0'+value%10U);value/=10U;}
    if(!AddBitmapText("MARKET",142,43,0xFF80FFFFU,43,assets,queue)||
       !AddBitmapText(money,184,43,0xFFFFFFFFU,43,assets,queue))return false;
    const auto& entries=MarketEntries();
    for(std::uint8_t i=0;i<MarketItemCount;++i){
        const bool buying=i<MarketCropCount;const MarketEntry& entry=entries[i%MarketCropCount];
        char label[20]{};std::size_t length=0;Append(label,length,buying?"BUY ":"SELL ");
        Append(label,length,entry.name);label[length++]=' ';
        AppendNumber(label,length,buying?entry.seedPrice:entry.sellPrice);
        if(!AddBitmapText(label,94,static_cast<float>(RowY+i*RowH+4),0xFFFFFFFFU,42,assets,queue))return false;
    }
    SpriteCommand cursor{};cursor.x=74.0F+pointer->trimX;cursor.y=static_cast<float>(RowY+focus*RowH)+pointer->trimY;
    cursor.width=pointer->width;cursor.height=pointer->height;cursor.uvX=pointer->x;cursor.uvY=pointer->y;
    cursor.uvWidth=pointer->width;cursor.uvHeight=pointer->height;cursor.layer=SpriteLayer::UI;cursor.depth=44;
    return queue.Add(cursor);
}
} // namespace Homestead
