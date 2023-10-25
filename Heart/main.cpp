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

vector<Vector> Hearts;

void DrawLine(HDC hdc, Vector from, Vector to){
	MoveToEx(hdc, from.x, from.y, NULL);
	LineTo(hdc, to.x, to.y);
}

float Lerp(float p1, float p2, float d1){
	return (1-d1) * p1 + d1 * p2;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HGDIOBJ hOld;
	BITMAP bmp;
	static RECT crt;
	static float Radius, PI;
	static float rad, inc, dec, duration, elapsed, elapInc;

	switch(iMessage){
		case WM_CREATE:
			SetRect(&crt, 0,0, 800, 600);
			AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
			SetWindowPos(hWnd, NULL, crt.left, crt.top, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
			rad = 0.f;
			inc = 0.001f;
			dec = 0.001f;
			elapInc = 0.1f;
			PI = atan(1.0) * 4.0;
			duration = 1.5f;
			elapsed = 0.f;
			SetTimer(hWnd, 1, 100, NULL);
			return 0;

		case WM_KEYDOWN:
			switch(wParam){
				case VK_LEFT:
					rad += inc;
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case VK_RIGHT:
					rad -= dec;
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

					if(Hearts.empty()){
						for(rad = 0.f; rad < PI * 2; rad += inc){
							float sinf = sin(rad);
							float cosf = cos(rad);
							float cos2 = cos(2 * rad);
							float cos3 = cos(3 * rad);
							float cos4 = cos(4 * rad);
							float x = 16.f * sinf * sinf * sinf;
							float y = 13 * cosf - 5 * cos2 - 2 * cos3 - cos4;
							Hearts.push_back(Vector(-x,-y));
						}
					}
					{
						elapsed = fmod(elapsed + elapInc, duration) ;
						float currentRad = (elapsed / duration) * (PI * 2);
						float alpha = ((cos(currentRad) + 1) * 0.5f);

						float sMin = 1.0f;
						float sMax = 5.0f;

						float Scale = Lerp(sMin, sMax, alpha);

						for(auto const &v : Hearts){
							SetPixel(hMemDC, v.x * Scale, v.y * Scale, RGB(0,0,0));
						}
					}

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
