﻿#pragma comment( lib, "d3d9.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib")
#else
#pragma comment( lib, "d3dx9.lib")
#endif

#pragma comment (lib, "winmm.lib")

#pragma comment( lib, "command.lib")

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
struct LeakChecker
{
    LeakChecker()
    {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }
} _leakChecker;

#endif

#include "..\command\command.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <tchar.h>
#include <atlbase.h>
#include <memory>

using namespace NSCommand;

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos {(float)x, (float)y, 0.f};
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width),
            static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();

    }

    void Load(const std::wstring& filepath) override
    {
        LPD3DXSPRITE tempSprite { nullptr };
        if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
        {
            throw std::exception("Failed to create a sprite.");
        }

        if (FAILED(D3DXCreateTextureFromFile(m_pD3DDevice,
                                             filepath.c_str(),
                                             &m_pD3DTexture)))
        {
            throw std::exception("Failed to create a texture.");
        }

        D3DSURFACE_DESC desc { };
        if (FAILED(m_pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ~Sprite()
    {
    }

private:

    CComPtr<IDirect3DDevice9> m_pD3DDevice = NULL;
    CComPtr<ID3DXSprite> m_D3DSprite = NULL;
    CComPtr<IDirect3DTexture9> m_pD3DTexture = NULL;
    UINT m_width { 0 };
    UINT m_height { 0 };
};

class Font : public NSCommand::IFont
{
public:

    Font(LPDIRECT3DDEVICE9 pD3DDevice)
        : m_pD3DDevice(pD3DDevice)
    {
    }

    void Init(const bool bEnglish)
    {
        if (!bEnglish)
        {
            HRESULT hr = D3DXCreateFont(m_pD3DDevice,
                                        18,
                                        0,
                                        FW_NORMAL,
                                        1,
                                        false,
                                        DEFAULT_CHARSET,
                                        OUT_TT_ONLY_PRECIS,
                                        CLEARTYPE_NATURAL_QUALITY,
                                        FF_DONTCARE,
                                        _T("BIZ UDMincho Medium"),
                                        &m_pFont);
        }
        else
        {
            HRESULT hr = D3DXCreateFont(m_pD3DDevice,
                                        18,
                                        0,
                                        FW_NORMAL,
                                        1,
                                        false,
                                        DEFAULT_CHARSET,
                                        OUT_TT_ONLY_PRECIS,
                                        CLEARTYPE_NATURAL_QUALITY,
                                        FF_DONTCARE,
                                        _T("Calibri"),
                                        &m_pFont);
        }
    }

    virtual void DrawText_(const std::wstring& msg,
                           const int x,
                           const int y,
                           const int transparent)
    {
        RECT rect = { x, y, x + 100, y + 100 };
        m_pFont->DrawText(NULL, msg.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_NOCLIP,
            D3DCOLOR_ARGB(transparent, 255, 255, 255));
    }

    ~Font()
    {
    }

private:

    CComPtr<IDirect3DDevice9> m_pD3DDevice = NULL;
    CComPtr<ID3DXFont> m_pFont = NULL;
};

class SoundEffect : public ISoundEffect
{
    virtual void PlayMove() override
    {
        PlaySound(_T("cursor_move.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
    virtual void PlayClick() override
    {
        PlaySound(_T("cursor_confirm.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
    virtual void PlayBack() override
    {
        PlaySound(_T("cursor_cancel.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
    virtual void Init() override
    {

    }
};

CComPtr<IDirect3D9> g_pD3D = NULL;
CComPtr<IDirect3DDevice9> g_pd3dDevice = NULL;
CComPtr<ID3DXFont> g_pFont = NULL;
CComPtr<ID3DXMesh> pMesh = NULL;
std::vector<CComPtr<IDirect3DTexture9>> pTextures;
DWORD dwNumMaterials = 0;
CComPtr<ID3DXEffect> pEffect = NULL;
D3DXMATERIAL* d3dxMaterials = NULL;
float f = 0.0f;
bool bShowMenu = true;

CommandLib menu;

void TextDraw(LPD3DXFONT pFont, wchar_t* text, int X, int Y)
{
    RECT rect = { X,Y,0,0 };
    pFont->DrawText(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 0, 0));
}

HRESULT InitD3D(HWND hWnd)
{
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
    {
        return E_FAIL;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
    {
        if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
        {
            return(E_FAIL);
        }
    }

    HRESULT hr = D3DXCreateFont(g_pd3dDevice,
                                20,
                                0,
                                FW_NORMAL,
                                1,
                                false,
                                SHIFTJIS_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                ANTIALIASED_QUALITY,
                                FF_DONTCARE,
                                _T("BIZ UDMincho Medium"),
                                &g_pFont);
    if FAILED(hr)
    {
        return(E_FAIL);
    }

    LPD3DXBUFFER pD3DXMtrlBuffer = NULL;

    if (FAILED(D3DXLoadMeshFromX(_T("cube.x"), D3DXMESH_SYSTEMMEM,
        g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL,
        &dwNumMaterials, &pMesh)))
    {
        MessageBox(NULL, _T("Xファイルの読み込みに失敗しました"), NULL, MB_OK);
        return E_FAIL;
    }

    d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();

    std::vector<D3DMATERIAL9> pMaterials(dwNumMaterials);
    pTextures.resize(dwNumMaterials);

    for (DWORD i = 0; i < dwNumMaterials; i++)
    {
        int len = MultiByteToWideChar(CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, NULL, 0);
        std::wstring texFilename(len, 0);
        MultiByteToWideChar(CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, &texFilename[0], len);

        pMaterials[i] = d3dxMaterials[i].MatD3D;
        pMaterials[i].Ambient = pMaterials[i].Diffuse;
        pTextures[i] = NULL;
        if (d3dxMaterials[i].pTextureFilename != NULL && len > 0)
        {
            if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice,
                                                 texFilename.c_str(),
                                                 &pTextures[i])))
            {
                MessageBox(NULL, _T("テクスチャの読み込みに失敗しました"), NULL, MB_OK);
            }
        }
    }
    pD3DXMtrlBuffer->Release();

    D3DXCreateEffectFromFile(
        g_pd3dDevice,
        _T("simple.fx"),
        NULL,
        NULL,
        D3DXSHADER_DEBUG,
        NULL,
        &pEffect,
        NULL
    );

    Sprite* sprCursor = new Sprite(g_pd3dDevice);
    sprCursor->Load(_T("command_cursor.png"));

    NSCommand::IFont* pFont = new Font(g_pd3dDevice);

    ISoundEffect* pSE = new SoundEffect();

    bool bEnglish = false;

    menu.Init(pFont, pSE, sprCursor, bEnglish, L"commandName.csv");
    
    menu.UpsertCommand(L"cutTree", true);
    menu.UpsertCommand(L"pickPlant", true);
    menu.UpsertCommand(L"rest3Hours", true);
    menu.UpsertCommand(L"lieDown", true);
    menu.UpsertCommand(L"sit", true);
    menu.UpsertCommand(L"escape", true);
    menu.UpsertCommand(L"craft", true);
    menu.UpsertCommand(L"patchTest", true);

    return S_OK;
}

VOID Cleanup()
{
    menu.Finalize();
}

VOID Render()
{
    if (NULL == g_pd3dDevice)
    {
        return;
    }
    f += 0.010f;

    D3DXMATRIX mat;
    D3DXMATRIX View, Proj;
    D3DXMatrixPerspectiveFovLH(&Proj, D3DXToRadian(45), 1600.0f / 900.0f, 1.0f, 1000.0f);
    D3DXVECTOR3 vec1(3 * sinf(f), 3, -3 * cosf(f));
    D3DXVECTOR3 vec2(0, 0, 0);
    D3DXVECTOR3 vec3(0, 1, 0);
    D3DXMatrixLookAtLH(&View, &vec1, &vec2, &vec3);
    D3DXMatrixIdentity(&mat);
    mat = mat * View * Proj;
    pEffect->SetMatrix("matWorldViewProj", &mat);

    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(70, 50, 30), 1.0f, 0);

    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        wchar_t msg[128];
        wcscpy_s(msg, 128, _T("Cキーでステータスを表示"));
        TextDraw(g_pFont, msg, 0, 0);

        pEffect->SetTechnique("BasicTec");
        UINT numPass;
        pEffect->Begin(&numPass, 0);
        pEffect->BeginPass(0);
        for (DWORD i = 0; i < dwNumMaterials; i++)
        {
            pEffect->SetTexture("texture1", pTextures[i]);
            pMesh->DrawSubset(i);
        }
        if (bShowMenu)
        {
            menu.Draw();
        }
        pEffect->CommitChanges();
        pEffect->EndPass();
        pEffect->End();
        g_pd3dDevice->EndScene();
    }

    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        Render();
        return 0;
    case WM_SIZE:
        InvalidateRect(hWnd, NULL, true);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'C':
            if (bShowMenu == false)
            {
                bShowMenu = true;
            }
            break;
        case VK_LEFT:
        {
            menu.Previous();
            break;
        }
        case VK_RIGHT:
        {
            menu.Next();
            break;
        }
        case VK_RETURN:
        {
            menu.Into();
            break;
        }
        case VK_ESCAPE:
            Cleanup();
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_MOUSEMOVE:
    {
        POINTS mouse_p = MAKEPOINTS(lParam);
        menu.MouseMove(mouse_p.x, mouse_p.y);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        POINTS mouse_p = MAKEPOINTS(lParam);
        menu.Click(mouse_p.x, mouse_p.y);
        break;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ INT)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      _T("Window1"), NULL };
    RegisterClassEx(&wc);

    RECT rect;
    SetRect(&rect, 0, 0, 1600, 900);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    HWND hWnd = CreateWindow(_T("Window1"),
                             _T("Hello DirectX9 World !!"),
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             rect.right,
                             rect.bottom, 
                             NULL,
                             NULL,
                             wc.hInstance,
                             NULL);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UnregisterClass(_T("Window1"), wc.hInstance);
    return 0;
}
