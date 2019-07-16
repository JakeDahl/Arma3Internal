#include "Renderer.hpp"
#include <memory>
#include <winerror.h>
#include "vmt.h"
#include "World.h"

typedef HRESULT(__fastcall* tD3D11Present) (IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall* tD3D11DrawIndexed) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall* tD3D11ClearRenderTargetView) (ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]);

tD3D11Present oD3DPresent;
tD3D11DrawIndexed oD3D11DrawIndexed;
tD3D11ClearRenderTargetView  oD3D11ClearRenderTargetView;

HANDLE hThread;
bool init = true;

ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;

DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pDeviceContextVTable = NULL;

std::unique_ptr<Renderer> renderer;

HRESULT DetourD3D11Present(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags);

void __stdcall DetourD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
    return oD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall DetourD3D11ClearRenderTargetView(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
    return oD3D11ClearRenderTargetView(pContext, pRenderTargetView, ColorRGBA);
}


DWORD __stdcall InitializeHook(LPVOID)
{
    HWND hWnd = FindWindow(L"Arma 3", NULL);
    IDXGISwapChain* pSwapChain;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;//((GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? FALSE : TRUE;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1
        , D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
    {
        MessageBox(hWnd, L"Failed.", L"Error!", MB_ICONERROR);
        return NULL;
    }


    pSwapChainVtable = (DWORD_PTR*)pSwapChain;
    pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

    pDeviceContextVTable = (DWORD_PTR*)pContext;
    pDeviceContextVTable = (DWORD_PTR*)pDeviceContextVTable[0];

    VMTHook hook((void*)((BYTE*)pSwapChainVtable));
    oD3DPresent = (tD3D11Present)hook.Hook(8, DetourD3D11Present);

    VMTHook deviceHook((void*)((BYTE*)pDeviceContextVTable));
    oD3D11DrawIndexed = (tD3D11DrawIndexed)deviceHook.Hook(12, DetourD3D11DrawIndexed);
    oD3D11ClearRenderTargetView = (tD3D11ClearRenderTargetView)deviceHook.Hook(50, DetourD3D11ClearRenderTargetView);

    pDevice->Release();
    pContext->Release();
    pSwapChain->Release();

   
    while (1)
    {
        Sleep(1000);
    }
}


IFW1Factory* pFW1Factory = NULL;
IFW1FontWrapper* pFontWrapper = NULL;

ID3D11Texture2D* RenderTargetTexture = nullptr;
ID3D11RenderTargetView* RenderTargetView = nullptr;


HRESULT DetourD3D11Present(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags)
{
    if (init)
    {
        //TerminateThread(hThread, 0);

        swapChain->GetDevice(__uuidof(pDevice), (void**)& pDevice);
        pDevice->GetImmediateContext(&pContext);

        renderer = std::make_unique<Renderer>(pDevice);

        swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& RenderTargetTexture);
        pDevice->CreateRenderTargetView(RenderTargetTexture, NULL, &RenderTargetView);
        RenderTargetTexture->Release();

        init = false;
    }

    pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);

    renderer->begin();

    Cheat(renderer);

    renderer->draw();
    renderer->end();

    return oD3DPresent(swapChain, SyncInterval, Flags);
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitializeHook, NULL, 0, NULL); //D3DCreateDeviceAndSwapchain only works in a thread

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}