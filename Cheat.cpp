#include "World.h"
#include <sstream>


float Dot(D3DXVECTOR3 left, D3DXVECTOR3 right)
{
    return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
}

int Distance(D3DXVECTOR3 first, D3DXVECTOR3 second)
{
    return (int)sqrtf(((second.x - first.x) * (second.x - first.x)) + ((second.y - first.y) * (second.y - first.y)) + ((second.z - first.z) * (second.z - first.z)));
}

D3DXVECTOR3 Cross(D3DXVECTOR3 a, D3DXVECTOR3 b)
{
    D3DXVECTOR3 product;
    product.x = (a.y * b.z) - (a.z * b.y);
    product.y = (a.z * b.x) - (a.x * b.z);
    product.z = (a.x * b.y) - (a.y * b.x);
    return product;
}

D3DXVECTOR3 WorldToScreen(D3DXVECTOR3 pos, TransData transData)
{
    D3DXVECTOR3 temp;
    temp.x = pos.x - transData.InvView_Translation.x;
    temp.y = pos.y - transData.InvView_Translation.y;
    temp.z = pos.z - transData.InvView_Translation.z;

    float x = Dot(temp, transData.InvView_right);
    float y = Dot(temp, transData.InvView_up);
    float z = Dot(temp, transData.InvView_forward);

    D3DXVECTOR3 ToReturn;
    ToReturn.x = transData.ViewPortMatrix3.x * (1 + ((x / transData.ProjD1.x) / z));
    ToReturn.y = transData.ViewPortMatrix3.y * (1 - ((y / transData.ProjD2.y) / z));
    ToReturn.z = z;
    return ToReturn;
}


World world;

XMVECTORF32 color;
bool once = true;

void DrawPlayer(std::unique_ptr<Renderer>& renderer, Entity ent, D3DXVECTOR3 w2sf, D3DXVECTOR3 w2sh, float rapport)
{
    XMVECTORF32 playerColor;

    switch (ent.sideId)
    {
        case 0: //REDFOR
            color.f[0] = 1.0f;
            color.f[1] = 0.f;
            color.f[2] = 0.f;
            color.f[3] = 1.0f;
            break;

        case 1: //BLUEFOR
            color.f[0] = 0.0f;
            color.f[1] = 0.f;
            color.f[2] = 1.f;
            color.f[3] = 1.0f;
            break;

        case 2: //GREEN
            color.f[0] = 0.0f;
            color.f[1] = 1.f;
            color.f[2] = 0.f;
            color.f[3] = 1.0f;
            break;

        case 3: //PURPLE
            color.f[0] = 1.0f;
            color.f[1] = 0.f;
            color.f[2] = 1.f;
            color.f[3] = 1.0f;
            break;
            
    }

    renderer->drawRect(Vec4(w2sh.x - (rapport / 4), w2sh.y, rapport / 2, rapport), 1, playerColor);
    renderer->drawText(Vec2(w2sf.x, w2sf.y), ent.info, playerColor, FW1_LEFT, 10.0f, L"Verdana");
}

void NoClip(uintptr_t localEntity, TransData* trans);
bool noClip = false;

void HandleHotkey()
{
    if (GetAsyncKeyState(VK_END) & 1)
    {
        noClip = !noClip;
    }
}

void Cheat(std::unique_ptr<Renderer>& renderer)
{
    if (once)
    {
        color.f[0] = 1.0f;
        color.f[1] = 0.f;
        color.f[2] = 0.f;
        color.f[3] = 1.0f;
        once = false;
    }
    HandleHotkey();

    uintptr_t worldptr = world.GetWorld();
    TransData* trans = world.GetTransData();
    uintptr_t localPlayer = world.GetCameraOnEntity();

    if(!localPlayer && !worldptr) return;

    if (noClip && trans)
    {
        NoClip(localPlayer, trans);
    }

    uintptr_t entityTable = *(uintptr_t*)(worldptr + world.offsets.nearEntityTable);
    int entityTableSize = *(int*)(worldptr + world.offsets.nearEntityTable + 0x8);

    if (entityTable && entityTableSize > 0)
    {
        for (size_t i = 0; i < entityTableSize; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entityTable + (i * 0x8));
            
            if(!entity || entity == localPlayer) continue;

            Entity ent = world.SetupEntity(entity);

            if(ent.type == ENTITY_BAD_TYPE) continue;
            
            D3DXVECTOR3 feetposition = world.GetEntityPosition(entity, COORD_FEET);

            if(!feetposition) continue;

            
            D3DXVECTOR3 w2sf = WorldToScreen(feetposition, *trans);
            if (w2sf.z <= 0.19f) continue;

            D3DXVECTOR3 headpos = world.GetEntityPosition(entity, COORD_HEAD);
            D3DXVECTOR3 w2sh = WorldToScreen(headpos, *trans);

            float rapport = w2sf.y - w2sh.y;
            
            switch (ent.type)
            {
                case ENTITY_BAD_TYPE:
                    break;

                case ENTITY_PLAYER:
                    DrawPlayer(renderer, ent, w2sf, w2sh, rapport);
                    break;

                case ENTITY_AI:
                    color.f[0] = 1.0f;
                    color.f[1] = 1.f;
                    color.f[2] = 0.f;
                    color.f[3] = 1.0f;
                    renderer->drawRect(Vec4(w2sh.x - (rapport / 4), w2sh.y, rapport / 2, rapport), 1, color);
                    //renderer->drawText(Vec2(w2sf.x, w2sf.y), ent.info, color, FW1_LEFT, 10.0f, L"Verdana");
                    break;

                case ENTITY_VEHICLE:
                    color.f[0] = .5f;
                    color.f[1] = .5f;
                    color.f[2] = .5f;
                    color.f[3] = 1.0f;
                    renderer->drawText(Vec2(w2sf.x, w2sf.y), ent.info, color, FW1_LEFT, 10.0f, L"Verdana");
                    break;

                  default: break;
            }
        }
    }
}

D3DXVECTOR3 origCoords;
bool coordsUnset = true;
float clipSpeed = 0.2f;

void NoClip(uintptr_t localEntity, TransData* trans)
{
    D3DXVECTOR3 coords = world.GetEntityPosition(localEntity, COORD_FEET);

    uintptr_t manVisualState = *(uintptr_t*)(localEntity + world.offsets.manVisualState);
    if (!manVisualState) return;

    D3DXVECTOR3 zero;
    zero.x = 0;
    zero.y = 0;
    zero.z = 0;
    *(D3DXVECTOR3*)(manVisualState + 0x54) = zero;
    *(D3DXVECTOR3*)(manVisualState + 0x5C) = zero;


    if (coordsUnset)
    {
        origCoords = world.GetEntityPosition(localEntity, COORD_FEET);
        coordsUnset = false;
    }

    //driver.wpm(zero, visualState + 0x54); VELOCITY
   // driver.wpm(zero, visualState + 0x5C); VELOCITY
   // driver.wpm((bool)1, currentObj + off.landContact); LAND CONTACT

    if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState('S') || GetAsyncKeyState('A') || GetAsyncKeyState('D') || GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_CONTROL))
    {
        if (GetAsyncKeyState(VK_SHIFT))
        {
            clipSpeed = 1.0f;
        }

      

        if (GetAsyncKeyState('W') & 0x8000)
        {
            *(float*)(manVisualState + 0x2C) = (coords.x - ((clipSpeed) * sinf(trans->View_forward.x))); //driver.rpm<float>(visualState + 0x2C) - ((clipSpeed)* sinf(trans->View_forward.x))
            *(float*)(manVisualState + 0x34) = (coords.z + ((clipSpeed)* sinf(trans->View_forward.z)));
            *(float*)(manVisualState + 0x30) = (coords.y + ((clipSpeed)* sinf(trans->View_up.z)));
            origCoords = world.GetEntityPosition(localEntity, COORD_FEET);
        }

      /*  if (GetAsyncKeyState('S'))
        {
            driver.wpm(driver.rpm<float>(visualState + 0x2C) + ((clipSpeed)* sinf(transform.View_forward.x)), visualState + 0x2C);
            driver.wpm(driver.rpm<float>(visualState + 0x34) - ((clipSpeed)* sinf(transform.View_forward.z)), visualState + 0x34);
            driver.wpm(driver.rpm<float>(visualState + 0x30) - ((clipSpeed)* transform.View_up.z), visualState + 0x30);
            origCoords = driver.rpm<Vec3>(visualState + 0x2C);
        }

        if (GetAsyncKeyState('A'))
        {
            Vec3 trucking = Utils::Cross(transform.View_up, transform.View_forward);
            driver.wpm(driver.rpm<float>(visualState + 0x2C) - ((clipSpeed)* trucking.x), visualState + 0x2C);
            driver.wpm(driver.rpm<float>(visualState + 0x34) + ((clipSpeed)* trucking.z), visualState + 0x34);
            origCoords = driver.rpm<Vec3>(visualState + 0x2C);
        }

        if (GetAsyncKeyState('D'))
        {
            Vec3 trucking = Utils::Cross(transform.View_up, transform.View_forward);
            driver.wpm(driver.rpm<float>(visualState + 0x2C) + ((clipSpeed)* trucking.x), visualState + 0x2C);
            driver.wpm(driver.rpm<float>(visualState + 0x34) - ((clipSpeed)* trucking.z), visualState + 0x34);
            origCoords = driver.rpm<Vec3>(visualState + 0x2C);
        }

        if (GetAsyncKeyState(VK_SPACE))
        {
            driver.wpm(driver.rpm<float>(visualState + 0x30) + clipSpeed, visualState + 0x30);
            origCoords = driver.rpm<Vec3>(visualState + 0x2C);
        }

        if (GetAsyncKeyState(VK_CONTROL))
        {
            driver.wpm(driver.rpm<float>(visualState + 0x30) - clipSpeed, visualState + 0x30);
            origCoords = driver.rpm<Vec3>(visualState + 0x2C);
        }*/

        clipSpeed = 0.2f;
    }
    else
    {
        *(D3DXVECTOR3*)(manVisualState + 0x2C) = origCoords;
    }
}