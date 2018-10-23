#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    // TODO: Returns the overlap rectangle between the bounding boxes of entities a and b
    
    Vec2 *delta = new Vec2((abs((a->getComponent<CTransform>()->pos.x)  -  (b->getComponent<CTransform>()->pos.x)))
                           ,(abs((a->getComponent<CTransform>()->pos.y) -  (b->getComponent<CTransform>()->pos.y))));
    
    float ox = (a->getComponent<CBoundingBox>()->halfSize.x) + (b->getComponent<CBoundingBox>()->halfSize.x) - delta->x;
    float oy = (a->getComponent<CBoundingBox>()->halfSize.y) + (b->getComponent<CBoundingBox>()->halfSize.y) - delta->y;
    
    return Vec2(ox, oy);
}
