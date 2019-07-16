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

void Cheat(std::unique_ptr<Renderer>& renderer)
{
    if (once)
    {
        color.f[0] = 1.0f;
        color.f[1] = 0.f;
        color.f[2] = 0.f;
        color.f[3] = 1.0f;
    }

    uintptr_t worldptr = world.GetWorld();
    TransData* trans = world.GetTransData();
    uintptr_t localPlayer = world.GetCameraOnEntity();

    if(!localPlayer && !worldptr) return;

    uintptr_t entityTable = *(uintptr_t*)(worldptr + world.offsets.nearEntityTable);
    int entityTableSize = *(int*)(worldptr + world.offsets.nearEntityTable + 0x8);

    std::wstringstream all;
    all << entityTableSize << std::endl << std::hex << entityTable;

    renderer->drawText(Vec2(15, 15), all.str().c_str(), color, FW1_LEFT, 8.0f, L"Verdana");

    if (entityTable && entityTableSize > 0)
    {
        for (size_t i = 0; i < entityTableSize; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entityTable + (i * 0x8));
            if(!entity) continue;
            
            D3DXVECTOR3 position = world.GetEntityPosition(entity);

            if(!position) continue;

            D3DXVECTOR3 w2sf = WorldToScreen(position, *trans);

            if(w2sf.z <= 0.19f) continue;
            renderer->drawText(Vec2(w2sf.x, w2sf.y), L"Object", color, FW1_LEFT, 10.0f, L"Verdana");
        }
    }
}