#include "World.h"
#include <sstream>

int renderDistance = 1000;
bool showAi = false;
XMVECTORF32 black;

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

void DrawPlayer(std::unique_ptr<Renderer>& renderer, Entity ent, D3DXVECTOR3 w2sf, D3DXVECTOR3 w2sh, float rapport, int distance)
{

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

    std::wstringstream all;

    all << ent.info << std::endl << distance;
    renderer->drawRect(Vec4(w2sh.x - (rapport / 4), w2sh.y, rapport / 2, rapport), 1, color);
    renderer->drawText(Vec2(w2sf.x - 1, w2sf.y - 1), all.str().c_str(), black, FW1_LEFT, 10.0f, L"Verdana");
    renderer->drawText(Vec2(w2sf.x, w2sf.y), all.str().c_str(), color, FW1_LEFT, 10.0f, L"Verdana");

}


void HandleEntity(uintptr_t entity, uintptr_t localPlayer, std::unique_ptr<Renderer>& renderer, TransData* trans)
{
    if (!entity || entity == localPlayer) return;

    Entity ent = world.SetupEntity(entity);
    bool isDead = *(bool*)(entity + world.offsets.isDead);
    if(isDead) return;

    if (ent.type == ENTITY_BAD_TYPE || (showAi == false && ent.type == ENTITY_AI)) return;


    D3DXVECTOR3 feetposition = world.GetEntityPosition(entity, COORD_FEET, false);
    D3DXVECTOR3 localPos = world.GetEntityPosition(localPlayer, COORD_FEET, true);

    int dist = Distance(feetposition, localPos);

    if(dist > renderDistance) return;

    D3DXVECTOR3 w2sf = WorldToScreen(feetposition, *trans);
    if (w2sf.z <= 0.19f) return;

    D3DXVECTOR3 headpos = world.GetEntityPosition(entity, COORD_HEAD, false);
    D3DXVECTOR3 w2sh = WorldToScreen(headpos, *trans);

    float rapport = w2sf.y - w2sh.y;

    std::wstringstream all;

    switch (ent.type)
    {
    case ENTITY_BAD_TYPE:
        break;

    case ENTITY_PLAYER:
        DrawPlayer(renderer, ent, w2sf, w2sh, rapport, dist);
        break;

    case ENTITY_AI:
        color.f[0] = 1.f;
        color.f[1] = 1.f;
        color.f[2] = 0.f;
        color.f[3] = 1.0f;

        all << dist;
        renderer->drawRect(Vec4(w2sh.x - (rapport / 4), w2sh.y, rapport / 2, rapport), 1, color);
        renderer->drawText(Vec2(w2sf.x - 1, w2sf.y - 1), all.str().c_str(), black, FW1_LEFT, 10.0f, L"Verdana");
        renderer->drawText(Vec2(w2sf.x, w2sf.y), all.str().c_str(), color, FW1_LEFT, 10.0f, L"Verdana");
        

        break;

    case ENTITY_VEHICLE:
        color.f[0] = .7f;
        color.f[1] = .7f;
        color.f[2] = .7f;
        color.f[3] = 1.0f;

        all << ent.info << std::endl << dist;
        renderer->drawText(Vec2(w2sf.x-1, w2sf.y-1), all.str().c_str(), black, FW1_LEFT, 10.0f, L"Verdana");
        renderer->drawText(Vec2(w2sf.x, w2sf.y), all.str().c_str(), color, FW1_LEFT, 10.0f, L"Verdana");
        

        break;

    default: break;
    }

    return;
}


//TO-DO: setup nested forloop to iterate over tables vs. 4 seperate iterations. :/
void ESP(std::unique_ptr<Renderer>& renderer, uintptr_t worldptr, uintptr_t localPlayer, TransData* trans)
{
    uintptr_t entityTable = *(uintptr_t*)(worldptr + world.offsets.nearEntityTable);
    int entityTableSize = *(int*)(worldptr + world.offsets.nearEntityTable + 0x8);

    if (entityTable && entityTableSize > 0)
    {
        for (size_t i = 0; i < entityTableSize; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entityTable + (i * 0x8));
            HandleEntity(entity, localPlayer, renderer, trans);
        }
    }

    entityTable = *(uintptr_t*)(worldptr + world.offsets.farEntityTable);
    entityTableSize = *(int*)(worldptr + world.offsets.farEntityTable + 0x8);

    if (entityTable && entityTableSize > 0)
    {
        for (size_t i = 0; i < entityTableSize; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entityTable + (i * 0x8));
            HandleEntity(entity, localPlayer, renderer, trans);
        }
    }

    entityTable = *(uintptr_t*)(worldptr + world.offsets.farfarEntityTable);
    entityTableSize = *(int*)(worldptr + world.offsets.farfarEntityTable + 0x8);

    if (entityTable && entityTableSize > 0)
    {
        for (size_t i = 0; i < entityTableSize; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entityTable + (i * 0x8));
            HandleEntity(entity, localPlayer, renderer, trans);
        }
    }

    entityTable = *(uintptr_t*)(worldptr + world.offsets.farfarfarEntityTable);
    entityTableSize = *(int*)(worldptr + world.offsets.farfarfarEntityTable + 0x8);

    if (entityTable && entityTableSize > 0)
    {
        for (size_t i = 0; i < entityTableSize; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entityTable + (i * 0x8));
            HandleEntity(entity, localPlayer, renderer, trans);
        }
    }
}

D3DXVECTOR3 origCoords;
bool coordsUnset = true;
float clipSpeed = 0.2f;

void NoClip(uintptr_t localEntity, TransData* trans)
{
    if (localEntity && trans)
    {
        D3DXVECTOR3 coords = world.GetEntityPosition(localEntity, COORD_FEET, true);

        uintptr_t manVisualState = *(uintptr_t*)(localEntity + world.offsets.manVisualState);
        if (!manVisualState) return;

        D3DXVECTOR3 zero;
        zero.x = 0;
        zero.y = 0;
        zero.z = 0;

        *(D3DXVECTOR3*)(manVisualState + 0x54) = zero; //Velocity
        *(D3DXVECTOR3*)(manVisualState + 0x5C) = zero; //Velocity
        *(bool*)(localEntity + world.offsets.landContact) = (bool)1;

        if (coordsUnset)
        {
            origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            coordsUnset = false;
        }

        if (GetAsyncKeyState('W') || GetAsyncKeyState('S') || GetAsyncKeyState('A') || GetAsyncKeyState('D') || GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_CONTROL))
        {
            if (GetAsyncKeyState(VK_SHIFT))
            {
                clipSpeed = 1.0f;
            }

            if (GetAsyncKeyState('W'))
            {
                *(float*)(manVisualState + 0x2C) = (coords.x - ((clipSpeed)* sinf(trans->View_forward.x)));
                *(float*)(manVisualState + 0x34) = (coords.z + ((clipSpeed)* sinf(trans->View_forward.z)));
                *(float*)(manVisualState + 0x30) = (coords.y + ((clipSpeed)* sinf(trans->View_up.z)));
                origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            }

            if (GetAsyncKeyState('S'))
            {
                *(float*)(manVisualState + 0x2C) = (coords.x + ((clipSpeed)* sinf(trans->View_forward.x)));
                *(float*)(manVisualState + 0x34) = (coords.z - ((clipSpeed)* sinf(trans->View_forward.z)));
                *(float*)(manVisualState + 0x30) = (coords.y - ((clipSpeed)* sinf(trans->View_up.z)));
                origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            }

            if (GetAsyncKeyState('A'))
            {
                D3DXVECTOR3 trucking = Cross(trans->View_up, trans->View_forward);
                *(float*)(manVisualState + 0x2C) = (coords.x - ((clipSpeed)* trucking.x));
                *(float*)(manVisualState + 0x34) = (coords.z + ((clipSpeed)* trucking.z));
                origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            }

            if (GetAsyncKeyState('D'))
            {
                D3DXVECTOR3 trucking = Cross(trans->View_up, trans->View_forward);
                *(float*)(manVisualState + 0x2C) = (coords.x + ((clipSpeed)* trucking.x));
                *(float*)(manVisualState + 0x34) = (coords.z - ((clipSpeed)* trucking.z));
                origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            }


            if (GetAsyncKeyState(VK_SPACE))
            {
                *(float*)(manVisualState + 0x30) = coords.y + clipSpeed;
                origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            }

            if (GetAsyncKeyState(VK_CONTROL))
            {
                *(float*)(manVisualState + 0x30) = coords.y - clipSpeed;
                origCoords = world.GetEntityPosition(localEntity, COORD_FEET, true);
            }

            clipSpeed = 0.2f;
        }
        else
        {
            *(D3DXVECTOR3*)(manVisualState + 0x2C) = origCoords;
        }
    }
}



void NoClip(uintptr_t localEntity, TransData* trans);
bool noClip = false;


void HandleHotkey()
{

    if (GetAsyncKeyState(VK_END) & 1)
    {
        noClip = !noClip;
        coordsUnset = true;
    }

    if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000)
    {
        renderDistance += 100;
    }

    if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
    {
        renderDistance -= 100;
    }

    if (GetAsyncKeyState(VK_F1) & 1)
    {
        showAi = !showAi;
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

        black.f[0] = 0.f;
        black.f[1] = 0.f;
        black.f[2] = 0.f;
        black.f[3] = 1.f;
        once = false;
    }

    HandleHotkey();

    uintptr_t worldptr = world.GetWorld();
    TransData* trans = world.GetTransData();
    uintptr_t localPlayer = world.GetCameraOnEntity();

    if(!worldptr || !localPlayer || !trans) return;

    if (noClip)
    {
        NoClip(localPlayer, trans);
    }

    ESP(renderer, worldptr, localPlayer, trans);

    color.f[0] = 1.0f;
    color.f[1] = 0.f;
    color.f[2] = 0.f;
    color.f[3] = 1.0f;


    std::wstringstream all;

    all << renderDistance;
    renderer->drawText(Vec2(15, 15), all.str().c_str(), color, FW1_LEFT, 10.0f, L"Verdana");
}