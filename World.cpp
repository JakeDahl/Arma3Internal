#include "World.h"

TransData* World::GetTransData()
{
    uintptr_t world = *(uintptr_t*)(this->offsets.base + this->offsets.gWorld);
    if(!world) return NULL;

    uintptr_t camera = *(uintptr_t*)(world + this->offsets.camera);
    if (!camera) return NULL;

    TransData* trans = (TransData*)(camera + 0x8);
    if(!trans) return NULL;

    return trans;
}

uintptr_t World::GetCameraOnEntity()
{
    uintptr_t world = *(uintptr_t*)(this->offsets.base + this->offsets.gWorld);
    if (!world) return NULL;

    uintptr_t cameraOn = *(uintptr_t*)(world + this->offsets.CameraOn);
    if (!cameraOn) return NULL;

    uintptr_t entity = *(uintptr_t*)(cameraOn + 0x8); //Read out of unitinfo link.
    if (!entity) return NULL;

    return entity;
}

uintptr_t World::GetWorld()
{
    uintptr_t world = *(uintptr_t*)(this->offsets.base + this->offsets.gWorld);

    if(!world) return NULL;
    return world;
}

D3DXVECTOR3* World::GetEntityPosition(uintptr_t entity)
{
    uintptr_t manVisualState = *(uintptr_t*)(entity + this->offsets.feetPos);
    if (!manVisualState) return NULL;

    D3DXVECTOR3 position = *(D3DXVECTOR3*)(manVisualState + 0x2C);

    return &position;
}