#pragma once
#include <DirectXMath.h>
#include <d3dx9math.h>
#include <Windows.h>
#include <psapi.h>
#include <iostream>
#include "Renderer.hpp"


void Cheat(std::unique_ptr<Renderer>& renderer);

typedef enum _COORD_TYPE
{
    COORD_FEET = 0,
    COORD_HEAD = 1
} COORD_TYPE;

typedef enum _ENTITY_TYPE
{
    ENTITY_PLAYER = 0,
    ENTITY_AI = 1,
    ENTITY_VEHICLE = 2,
    ENTITY_BAD_TYPE = 3
} ENTITY_TYPE;

class Entity
{
    public:
        std::wstring info;
        ENTITY_TYPE type;
        int sideId = -1;
        int playerId = 0;
};

class offsets
{
public:
    uintptr_t base;
    uintptr_t gWorld = 0x2571DB8;
    
    uint32_t engine = 0x480;

    uint32_t InGameUI = 0x488; //World->
    uint32_t CursorTarget = 0x20; //IngameUI->
    uint32_t CursorPos = 0x46C; //IngameUI->
    uint32_t camera = 0x800; //TransData

    uint32_t nearEntityTable = 0x16D8;
    uint32_t farEntityTable = 0x17A0;
    uint32_t farfarEntityTable = 0x1868;
    uint32_t farfarfarEntityTable = 0x1930;

    uint32_t CameraOn = 0x26D8; //World->this + 0x8 = entity
    uint32_t LocalPlayer = 0x26F8; //World->this + 0x8 = entity
    uint32_t Entity = 0x8;
    uint32_t sideid = 0x340;
    uint32_t playerId = 0xB94;
    uint32_t landContact = 0x345; //bool
    uint32_t isDead = 0x504; //bool

    uint32_t manVisualState = 0xD0;
    uint32_t renderVisualState = 0x190;

    uint32_t feetPos = 0x2C;//vec3
    uint32_t headPos = 0x168;//vec3
    

    uint32_t model = 0x150; //entity+ this
    uint32_t modelName = 0x80; //name = 0x10, size = 0x8

    uintptr_t gNetworkManager = 0x252FD98;
    uint32_t networkClient = 0x48;
    uint32_t scoreboard = 0x38; //ScoreboardSz = NetworkClient + scoreboard + 0x8 (int)
    uint32_t usernamePtr = 0x170; //
    uint32_t scoreboardObjectSize = 0x2E0;
};



struct TransData
{
    char unknown0[8]; //0x0000
    D3DXVECTOR3 InvView_right; //0x0004
    D3DXVECTOR3 InvView_up; //0x0010  
    D3DXVECTOR3 InvView_forward; //0x001C  
    D3DXVECTOR3 InvView_Translation; //0x0028  

    D3DXVECTOR2 pad; //0x0034   (x = 1,y = -1)


    D3DXVECTOR3 ViewPortMatrix1; //0x003C  (x= viewPort.Width/2)
    D3DXVECTOR3 ViewPortMatrix2; //0x0048 -(y= viewPort.Height/2)

    D3DXVECTOR3 ViewPortMatrix3;
    //0x0054(x=viewPort.X + CenterScreenX,y=viewPort.Y + CenterScreenY,z=viewPort.MinZ)

    D3DXVECTOR3 ViewPortMatrix_unk; //not sure

    D3DXVECTOR3 ProjO1; //proj1.x  0x006C
    D3DXVECTOR3 ProjO2; //proj2.y  0x0078
    D3DXVECTOR3 ProjO3; //proj3.z  0x0084     =1.0001
    D3DXVECTOR3 ProjO4; //proj4.z  0x0090

    D3DXVECTOR3 Proj1; //proj1.x  0x009C
    D3DXVECTOR3 Proj2; //proj2.y  0x00A8
    D3DXVECTOR3 Proj3; //proj3.z  0x00B4     = 1.0
    D3DXVECTOR3 Proj4; //proj4.z  0x00C0

    D3DXVECTOR3 ProjD1; //   0x00CC     x = 1/Proj._11
    D3DXVECTOR3 ProjD2; //   0x00D8     y = 1/Proj._22
    D3DXVECTOR3 ProjD3; //   0x00E4     z=  1
    D3DXVECTOR3 ProjD4; //   0x00F0

    D3DXVECTOR2 pad1; //.x bigass float  0x00FC

    D3DXVECTOR3 View_right; //0x0104    not the view
    D3DXVECTOR3 View_up; //0x0110  
    D3DXVECTOR3 View_forward; //0x011C  
    D3DXVECTOR3 View_Translation; //0x0128
};


class World
{
public:
    World() { this->offsets.base = (uintptr_t)GetModuleHandle(NULL); }

    char* GetPlayerName(int playerId);
    TransData* GetTransData();
    uintptr_t GetCameraOnEntity();
    uintptr_t GetWorld();
    offsets offsets;
    Entity SetupEntity(uintptr_t entity);
    D3DXVECTOR3 GetEntityPosition(uintptr_t entity, COORD_TYPE type, bool localPlayer);

private:

};