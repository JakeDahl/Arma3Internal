#include "World.h"

TransData* World::GetTransData()
{
    uintptr_t world = *(uintptr_t*)(this->offsets.base + this->offsets.gWorld);
    if(!world) return NULL;

    uintptr_t camera = *(uintptr_t*)(world + this->offsets.camera);
    if (!camera) return NULL;

    TransData* trans = (TransData*)(camera);
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


D3DXVECTOR3 World::GetEntityPosition(uintptr_t entity, COORD_TYPE type, bool localPlayer)
{
    uintptr_t manVisualState;

    if (localPlayer)
    {
        manVisualState = *(uintptr_t*)(entity + this->offsets.manVisualState);
    }

    else
    {
        manVisualState = *(uintptr_t*)(entity + this->offsets.renderVisualState);
    }
    
    if (!manVisualState) D3DXVECTOR3(0,0,0);
    D3DXVECTOR3 position;
    if (type == COORD_FEET)
    {
        position = *(D3DXVECTOR3*)(manVisualState + this->offsets.feetPos);
    }
    
    if (type == COORD_HEAD)
    {
        position = *(D3DXVECTOR3*)(manVisualState + this->offsets.headPos);
    }

    return position;
}


char* World::GetPlayerName(int playerId)
{
    uintptr_t networkManager = *(uintptr_t*)(this->offsets.base + this->offsets.gNetworkManager);
    if(!networkManager) return NULL;

    uintptr_t networkClient = *(uintptr_t*)(networkManager + this->offsets.networkClient);
    if(!networkClient) return NULL;

    uintptr_t scoreboard = *(uintptr_t*)(networkClient + this->offsets.scoreboard);
    if (!scoreboard) return NULL;

    int scoreboardSz = *(int*)(networkClient + this->offsets.scoreboard + 0x8);

    for (size_t i = 0; i < scoreboardSz; i++)
    {
        int scoreboardPId = *(int*)(scoreboard + (i * this->offsets.scoreboardObjectSize) + 0x8);

        if (scoreboardPId == playerId)
        {
            uintptr_t usernamePtr = *(uintptr_t*)(scoreboard + (i * this->offsets.scoreboardObjectSize) + this->offsets.usernamePtr);

            if (usernamePtr)
            {
                char* name = (char*)(usernamePtr + 0x10);

                if (name) return name;

                else return NULL;
            }
        }
    }
    return NULL;
}


#include <locale>
#include <codecvt>
#include <string>

Entity World::SetupEntity(uintptr_t entity)
{
    Entity ent;
    ent.type = ENTITY_BAD_TYPE;

    uintptr_t modelptr = *(uintptr_t*)(entity + this->offsets.model);
    if(!modelptr) return ent;

    uintptr_t modelStrPtr = *(uintptr_t*)(modelptr + this->offsets.modelName);
    if(!modelStrPtr) return ent;

    int nameSz = *(int*)(modelStrPtr + 0x8);
    if(nameSz <= 0) return ent;

    char* modelname = (char*)(modelStrPtr + 0x10);
    if(!modelname) return ent;

    int playerId = *(int*)(entity + this->offsets.playerId);

    if (strstr(modelname, "character"))
    {
        if (playerId > 1)
        {
            ent.type = ENTITY_PLAYER;
            char* playername = this->GetPlayerName(playerId);

            if(!playername)
            {
                ent.type = ENTITY_BAD_TYPE;
                return ent;
            }
            

            std::string tmp(playername);
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wide = converter.from_bytes(tmp);
            ent.info = wide;
            ent.sideId = *(int*)(entity + this->offsets.sideid); //sideid offset
            return ent;
        }

        else
        {
            ent.type = ENTITY_AI;
            ent.info = L"";
            return ent;
        }
    }
    else if ((strstr(modelname, "soft_f") != NULL) || (strstr(modelname, "armor_f") != NULL)
        || (strstr(modelname, "air_f") != NULL) || (strstr(modelname, "boat_f") != NULL)
        || (strstr(modelname, "vehicle") != NULL) || (strstr(modelname, "hmg") != NULL)
        || (strstr(modelname, "exile") != NULL))
    {
        ent.type = ENTITY_VEHICLE;
        std::string tmp(modelname);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(tmp);
        wide = wide.substr(wide.find_last_of(L"\\") + 1, (wide.find_last_of(L"_") - 1 - wide.find_last_of(L"\\")));
        ent.info = wide;
        return ent;
    }
    ent.type = ENTITY_BAD_TYPE;
    return ent;
}