#include "Homestead/UI/UIPanel.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"

namespace Homestead {
namespace {

bool AddPiece(const SpriteAsset& sprite, float x, float y, float width, float height,
              std::uint32_t color, std::uint16_t depth, RenderQueue& queue) noexcept {
    if(sprite.sourceWidth==0||sprite.sourceHeight==0)return false;
    const float scaleX=width/static_cast<float>(sprite.sourceWidth);
    const float scaleY=height/static_cast<float>(sprite.sourceHeight);
    SpriteCommand command{};
    command.x=x+static_cast<float>(sprite.trimX)*scaleX;
    command.y=y+static_cast<float>(sprite.trimY)*scaleY;
    command.width=static_cast<float>(sprite.width)*scaleX;
    command.height=static_cast<float>(sprite.height)*scaleY;
    command.uvX=sprite.x; command.uvY=sprite.y;
    command.uvWidth=sprite.width; command.uvHeight=sprite.height;
    command.color=color; command.layer=SpriteLayer::UI; command.depth=depth;
    return queue.Add(command);
}

} // namespace

bool AddUIFill(float x, float y, float width, float height, std::uint32_t color,
               std::uint16_t depth, const AssetStore& assets, RenderQueue& queue) noexcept {
    const SpriteAsset* center=assets.FindSprite(MakeAssetId("ui.panel.center"));
    return center!=nullptr && AddPiece(*center,x,y,width,height,color,depth,queue);
}

bool AddUIPanel(float x, float y, float width, float height, std::uint32_t color,
                std::uint16_t depth, const AssetStore& assets, RenderQueue& queue) noexcept {
    if(width<16.0F||height<16.0F)return false;
    constexpr const char* names[]={"ui.panel.tl","ui.panel.top","ui.panel.tr",
        "ui.panel.left","ui.panel.center","ui.panel.right",
        "ui.panel.bl","ui.panel.bottom","ui.panel.br"};
    const SpriteAsset* pieces[9]{};
    for(unsigned i=0;i<9;++i){pieces[i]=assets.FindSprite(MakeAssetId(names[i]));if(!pieces[i])return false;}
    const float middleWidth=width-16.0F,middleHeight=height-16.0F;
    return AddPiece(*pieces[0],x,y,8,8,color,depth,queue)&&
        AddPiece(*pieces[1],x+8,y,middleWidth,8,color,depth,queue)&&
        AddPiece(*pieces[2],x+width-8,y,8,8,color,depth,queue)&&
        AddPiece(*pieces[3],x,y+8,8,middleHeight,color,depth,queue)&&
        AddPiece(*pieces[4],x+8,y+8,middleWidth,middleHeight,color,depth,queue)&&
        AddPiece(*pieces[5],x+width-8,y+8,8,middleHeight,color,depth,queue)&&
        AddPiece(*pieces[6],x,y+height-8,8,8,color,depth,queue)&&
        AddPiece(*pieces[7],x+8,y+height-8,middleWidth,8,color,depth,queue)&&
        AddPiece(*pieces[8],x+width-8,y+height-8,8,8,color,depth,queue);
}

} // namespace Homestead
