#include "WindowsIncludes.h"
#include <d3d9.h>
#include "dxgraphics.h"

//variable declarations
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;

D3DXVECTOR3 cameraSource;
D3DXVECTOR3 cameraTarget;

D3DPRESENT_PARAMETERS d3dpp;

int Init_Direct3D(HWND hwnd, int width, int height, int fullscreen)
{
    //initialize Direct3D
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == NULL)
    {
        MessageBox(hwnd, "Error initializing Direct3D", "Error", MB_OK);
        return 0;
    }

    //set Direct3D presentation parameters
    D3DDISPLAYMODE dm;
    d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm); 
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.Windowed = (!fullscreen);
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.BackBufferWidth = width;
    d3dpp.BackBufferHeight = height;
    d3dpp.hDeviceWindow = hwnd;

    //create Direct3D device
    d3d->CreateDevice(
        D3DADAPTER_DEFAULT, 
        D3DDEVTYPE_HAL, 
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, 
        &d3ddev);

    if (d3ddev == NULL)
    {
        MessageBox(hwnd, "Error creating Direct3D device", "Error", MB_OK);
        return 0;
    }

    return 1;
}

LPDIRECT3DTEXTURE9 LoadTexture(char *filename, D3DCOLOR transcolor)
{
    //the texture pointer
    LPDIRECT3DTEXTURE9 texture = NULL;

    //the struct for reading bitmap file info
    D3DXIMAGE_INFO info;

    //standard Windows return value
    HRESULT result;
    
    //get width and height from bitmap file
    result = D3DXGetImageInfoFromFile(filename, &info);
    if (result != D3D_OK)
        return NULL;

    //create the new texture by loading a bitmap image file
	D3DXCreateTextureFromFileEx( 
        d3ddev,              //Direct3D device object
        filename,            //bitmap filename
        info.Width,          //bitmap image width
        info.Height,         //bitmap image height
        1,                   //mip-map levels (1 for no chain)
        0,				     //the type of surface (standard)
        D3DFMT_UNKNOWN,      //surface format (default)
        D3DPOOL_MANAGED,     //memory class for the texture
        D3DX_DEFAULT,        //image filter
        D3DX_DEFAULT,        //mip filter
        transcolor,          //color key for transparency
        &info,               //bitmap file info (from loaded file)
        NULL,                //color palette
        &texture );          //destination texture

    //make sure the bitmap textre was loaded correctly
    if (result != D3D_OK)
        return NULL;

    return texture;
}

D3DXMATRIX SetCamera(float x, float y, float z, float lookx, float looky, float lookz)
{
    D3DXMATRIX matView;
    D3DXVECTOR3 updir(0.0f, 1.0f, 0.0f);

    //move the camera
    cameraSource.x = x;
    cameraSource.y = y;
    cameraSource.z = z;
    
    //point the camera
    cameraTarget.x = lookx;
    cameraTarget.y = looky;
    cameraTarget.z = lookz;

    //set up the camera view matrix
    D3DXMatrixLookAtLH(&matView, &cameraSource, &cameraTarget, &updir);
    d3ddev->SetTransform(D3DTS_VIEW, &matView);

	return matView;
}

D3DXMATRIX SetPerspective(float fieldOfView, float aspectRatio, float nearRange, float farRange)
{
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, fieldOfView, aspectRatio, nearRange, farRange);
    d3ddev->SetTransform(D3DTS_PROJECTION, &matProj);

	return matProj;
}