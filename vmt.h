#pragma once

#include <Windows.h>
#include <map>

using namespace std;

class VMTHook
{
public:

    void** vmt = nullptr;

    VMTHook(void* vmt);
    VMTHook(DWORD64* vmt_ptr);
    ~VMTHook();

    void* Hook(int index, void* hk);
    void ClearHooks();
private:
    map<int, void*>::iterator func_iterator;
    map<int, void*> hooked_funcs;
};

VMTHook::VMTHook(void* vmt_addy)
{
    vmt = (void**)vmt_addy;
}

void* VMTHook::Hook(int index, void* hk)
{
    hooked_funcs.insert(make_pair(index, vmt[index]));

    DWORD old;
    VirtualProtect(&vmt[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    vmt[index] = hk;
    VirtualProtect(&vmt[index], sizeof(void*), old, NULL);

    return hooked_funcs[index];
}

void VMTHook::ClearHooks()
{
    for (func_iterator = hooked_funcs.begin(); func_iterator != hooked_funcs.end(); func_iterator++)
    {
        DWORD old;
        VirtualProtect(&vmt[func_iterator->first], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
        vmt[func_iterator->first] = func_iterator->second;
        VirtualProtect(&vmt[func_iterator->first], sizeof(void*), old, NULL);
    }
    hooked_funcs.clear();
    vmt = nullptr;
}

VMTHook::~VMTHook() {}