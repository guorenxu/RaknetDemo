//#include "WindowsIncludes.h"
#include <d3d9.h>
#include "dxgraphics.h"
#include "game.h"

int Game_Init(HWND hwnd);

int ChangeX;
int ChangeY;

extern bool Death;

bool DC = true;

bool DKey = false;

LRESULT WINAPI WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;

	DKey = false;

    switch(msg)
    {
		case WM_KEYDOWN:
        {
            switch(wParam)
            {
                case VK_ESCAPE:
					if (DC)
					{
						ServerDisconnect();
						DC = false;
					}
                    break;

				case 'A':
					SetKeyboard(1, true);
					break;

				case 'D':
					SetKeyboard(4, true);
					break;

				case 'S':
					SetKeyboard(19, true);
					break;

				case 'W':
					SetKeyboard(23, true);
					break;
            }
        }
        break;

		case WM_RBUTTONDOWN:
        {
            ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD(lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD(lParam);
            SetMouseClick(1, true);
        }
        break;

        case WM_RBUTTONUP:
        {
            SetMouseClick(1, false);
        }
        break;

		case WM_LBUTTONDOWN:
        {
			SetMouseClick(0, true);
        }
        break;

        case WM_LBUTTONUP:
        {
			SetMouseClick(0, false);
        }
        break;

		case WM_MOUSEMOVE:			//Remember that it only works if the mouse is moved
        {
            ptCurrentMousePosit.x = LOWORD(lParam);
            ptCurrentMousePosit.y = HIWORD(lParam);

            ChangeX = (ptCurrentMousePosit.x - ptLastMousePosit.x);
            ChangeY = (ptCurrentMousePosit.y - ptLastMousePosit.y);

			SetMouseChange(ChangeX, ChangeY);

            ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
        }
        break;

		case WM_MOUSEWHEEL:
		{
			SetMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
		}
		break;

		 case WM_DESTROY:
		{
            PostQuitMessage(0);
            return 0;
		}

		case WM_CLOSE:
        {
            PostQuitMessage(0);
			return 0;
        }
    }

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX); 

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)WinProc;
    wc.cbClsExtra	 = 0;
    wc.cbWndExtra	 = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(0, IDI_APPLICATION);;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = APPTITLE;
    wc.hIconSm       = NULL;

    return RegisterClassEx(&wc);
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;
    HWND hWnd;

	MyRegisterClass(hInstance);

    DWORD style;
    if (FULLSCREEN)
        style = WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP;
    else
        style = WS_OVERLAPPEDWINDOW;

    hWnd = CreateWindow(
       APPTITLE,
       APPTITLE,
       style,
       0,
       0, 
       SCREEN_WIDTH,     
       SCREEN_HEIGHT,     
       NULL,            
       NULL,         
       hInstance,       
       NULL);           

    if (!hWnd)
      return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
	
    Init_Direct3D(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN);
    
	Game_Init(hWnd);

    int done = 0;
	while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
	    {
            if (msg.message == WM_QUIT)
                done = 1;

		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
        else
		{
            Game_Run(hWnd);
		}
    }

	return msg.wParam;
}