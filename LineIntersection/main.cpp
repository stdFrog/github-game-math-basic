#define UNICODE
#include <windows.h>
#include <cmath>
#define CLASS_NAME TEXT("Example")

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow){
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
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
	while(GetMessage(&msg, nullptr, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


#define WIDTH 800
#define HEIGHT 600
#define WBASE WIDTH / 2
#define HBASE HEIGHT / 2

HBITMAP hBitmap;

POINT Triangle[] = {
	{WBASE, HBASE - 150},
	{WBASE - 200, HBASE + 150},
	{WBASE + 200, HBASE + 150},
};

const int each = sizeof(Triangle)/sizeof(Triangle[0]);

void DrawLine(HDC hdc, POINT from, POINT to){
	MoveToEx(hdc, from.x, from.y, NULL);
	LineTo(hdc, to.x, to.y);
}

/*
	1. 검사하고자 하는 점의 y좌표가 다각형의 선분(점과 점) 사이에 위치해야 한다.
	2. 검사할 점을 지나는 반직선과 선분과의 교차점의 x위치는 언제나 검사할 점의 x위치보다 크다(오른쪽으로 반직선을 긋는다).
	3. 2번 규칙에 의해 교차하는 교점과 현재 검사하고자 하는 점의 높이 즉, y값이 서로 같다.
*/

BOOL LineIntersect(POINT Current/* = Mouse Coord */, POINT *Vertex /* = Triangle Vertex */){
	int cnt=0, i=0, j=0;

	for(i=0; i<each; i++){
		j = (i+1) % each;

		BOOL bCheck = ((Current.y > Vertex[i].y) != (Current.y > Vertex[j].y));

		if(bCheck){
			LONG cpt[3];
			cpt[0] = Vertex[j].x - Vertex[i].x;
			cpt[1] = Vertex[j].y - Vertex[i].y;
			cpt[2] = Current.y - Vertex[i].y;

			// 삼각형의 닮음 특성을 이용한다.
			// 삼각형의 밑변 길이의 비가 같고 높이가 같으면 
			float IntersectPointX = cpt[0] * cpt[2] / cpt[1] + Vertex[i].x;
			if(Current.x < IntersectPointX){cnt++;}
		}
	}

	return (BOOL)(cnt % 2 != 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HGDIOBJ hOld;
	BITMAP bmp;
	TCHAR lpszText[128];
	LPMINMAXINFO lpmi;
	static RECT crt;
	static POINT Mouse;
	int i;

	switch(iMessage){
		case WM_CREATE:
			SetRect(&crt, 0,0, WIDTH, HEIGHT);
			AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
			SetWindowPos(hWnd, NULL, crt.left, crt.top, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
			SetTimer(hWnd, 1, 10, NULL);
			return 0;

		case WM_GETMINMAXINFO:
			lpmi = (LPMINMAXINFO)lParam;
			lpmi->ptMaxTrackSize.x = crt.right - crt.left;
			lpmi->ptMaxTrackSize.y = crt.bottom - crt.top;
			lpmi->ptMinTrackSize.x = crt.right - crt.left;
			lpmi->ptMinTrackSize.y = crt.bottom - crt.top;
			return 0;

		case WM_TIMER:
			hdc = GetDC(hWnd);
			hMemDC = CreateCompatibleDC(hdc);
			if(hBitmap == NULL){
				hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
			}
			
			hOld = SelectObject(hMemDC, hBitmap);
			FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
			
			// TO DO
			GetCursorPos(&Mouse);
			ScreenToClient(hWnd, &Mouse);

			for(i=0; i<each; i++){
				DrawLine(hMemDC, Triangle[i], Triangle[(i+1)%each]);
			}

			if(LineIntersect(Mouse, Triangle)){
				TRIVERTEX vert[3];
				GRADIENT_TRIANGLE gtr;

				vert[0].x = WBASE; vert[0].y = HBASE - 150;
				vert[1].x = WBASE - 200; vert[1].y = HBASE + 150;
				vert[2].x = WBASE + 200; vert[2].y = HBASE + 150;

				vert[0].Red = 0; vert[0].Green = 0xff00; vert[0].Blue = 0;
				vert[1].Red = 0; vert[1].Green = 0; vert[1].Blue = 0xff00;
				vert[2].Red = 0xff00; vert[2].Green = 0; vert[2].Blue = 0;

				vert[0].Alpha = 0;
				vert[1].Alpha = 0;
				vert[2].Alpha = 0;

				gtr.Vertex1 = 0;
				gtr.Vertex2 = 1;
				gtr.Vertex3 = 2;

				GradientFill(hMemDC, vert, 3, &gtr, 1, GRADIENT_FILL_TRIANGLE);
			}

			wsprintf(lpszText, TEXT("Mouse Coord (%d, %d)"), Mouse.x, Mouse.y);
			TextOut(hMemDC, 10, 16, lpszText, lstrlen(lpszText));

			SelectObject(hMemDC, hOld);
			DeleteDC(hMemDC);
			ReleaseDC(hWnd, hdc);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(hBitmap){
				hMemDC = CreateCompatibleDC(hdc);
				GetObject(hBitmap, sizeof(BITMAP), &bmp);
				hOld = SelectObject(hMemDC, hBitmap);

				BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
			}
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			if(hBitmap){
				DeleteObject(hBitmap);
			}
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
