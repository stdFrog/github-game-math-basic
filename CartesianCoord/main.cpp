#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <algorithm>
#include <vector>
#include <math.h>
#define CLASS_NAME TEXT("Example")

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow){
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL ,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME,
		NULL
	};

	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

HBITMAP hBit;

struct Vector{
	float x,y,z;
	Vector(float _x = 0.f, float _y = 0.f, float _z = 0.f) : x(_x), y(_y), z(_z) {;}
	Vector(POINT pt) : x((int)pt.x), y((int)pt.y), z(0.f) {;}

	float Squared(){
		return x * x + y * y;
	}

	float Length(){
		return sqrt(Squared());
	}

	void Normalize(){
		float ret = Length();
		if(ret < 0.000000f){return;}
		x /= ret;
		y /= ret;
	}
};

vector<Vector> Circle;

void DrawLine(HDC hdc, Vector from, Vector to){
	MoveToEx(hdc, from.x, from.y, NULL);
	LineTo(hdc, to.x, to.y);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HGDIOBJ hOld;
	BITMAP bmp;
	static RECT crt;
	static Vector v1, v2, Org;
	static float Radius, PI;
	static float rad,inc,dec;
	static float Length;

	switch(iMessage){
		case WM_CREATE:
			SetRect(&crt, 0,0, 800, 600);
			AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
			SetWindowPos(hWnd, NULL, crt.left, crt.top, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
			Org = {0,0,0};
			rad = atan(1.0) * 2.0;
			inc = 0.1f;
			dec = 0.1f;
			v1 = {1, 0};
			v2 = {0, 1};
			Radius = 300.f;
			SetTimer(hWnd, 1, 100, NULL);
			return 0;

		case WM_KEYDOWN:
			switch(wParam){
				case VK_LEFT:
					rad -= dec;
					v1.x = Radius * cos(rad);
					v1.y = Radius * sin(rad);
					v2.x = Radius * sin(-rad);
					v2.y = Radius * cos(rad);
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case VK_RIGHT:
					rad += inc;
					v1.x = Radius * cos(rad);
					v1.y = Radius * sin(rad);
					v2.x = Radius * sin(-rad);
					v2.y = Radius * cos(rad);
					InvalidateRect(hWnd, NULL, FALSE);
					break;
			}
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 1:
					hdc = GetDC(hWnd);
					hMemDC = CreateCompatibleDC(hdc);
					
					if(hBit == NULL){
						hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
					}
					hOld = SelectObject(hMemDC, hBit);
					FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
					SetViewportOrgEx(hMemDC, crt.right - crt.left >> 1, crt.bottom - crt.top >> 1, NULL);

					MoveToEx(hMemDC, -(crt.right - crt.left >> 1), 0, NULL);
					LineTo(hMemDC, crt.right - crt.left >> 1, 0);

					MoveToEx(hMemDC, 0, -(crt.bottom - crt.top >> 1), NULL);
					LineTo(hMemDC, 0, crt.bottom - crt.top >> 1);

					DrawLine(hMemDC, Org, v1);
					DrawLine(hMemDC, Org, v2);

					SelectObject(hMemDC, hOld);
					DeleteDC(hMemDC);
					ReleaseDC(hWnd, hdc);
					InvalidateRect(hWnd, NULL, FALSE);
					break;
			}
			return 0;
				
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(hBit){
				GetObject(hBit, sizeof(BITMAP), &bmp);
				hMemDC = CreateCompatibleDC(hdc);
				hOld = SelectObject(hMemDC, hBit);

				BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);


				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
			}
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			if(hBit){DeleteObject(hBit);}
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
