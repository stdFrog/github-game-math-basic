#include <windows.h>
#include <cstdio>
#include <cmath>
#define CLASS_NAME "Cross Product"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow){
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME
	};

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
			CLASS_NAME,
			CLASS_NAME,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			(HMENU)NULL,
			hInst,
			NULL);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, nullptr, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

struct Vector{
	float x,y;
};

float GetScalar(Vector V){
	return sqrt(V.x * V.x + V.y * V.y);
}

float Cross(Vector v1, Vector v2){
	// 한쪽 방향을 임의로 지정하면 반대 방향일 경우 음수로 출력될 것이다
	return v1.x * v2.y - v1.y * v2.x;
	
	// 참고로 2차원에서 외적은 역행렬을 판단하는 행렬식과 똑같다.
}

void DrawLine(HDC hdc, Vector from, Vector to){
	MoveToEx(hdc, (int)from.x, (int)from.y, NULL);
	LineTo(hdc, (int)to.x, (int)to.y);
}

void Normalize(Vector *V){
	float length = GetScalar(*V);
	if (length < 0.0000000f) {
			return;
	}
	V->x /= length;
	V->y /= length;
}

Vector arTriangle[] = {
	{400, 150},
	{200, 450},
	{600, 450},
	{400, 150}
};

HBITMAP hBitmap;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	HDC hdc, hMemDC;
	HBITMAP hOld;
	PAINTSTRUCT ps;
	TCHAR lpszText[128];
	BITMAP bmp;
	LPMINMAXINFO lpmi;
	static RECT crt;
	static POINT Mouse;
	static float CrossResult[10];
	static Vector Base, V[10];
	int i;

	switch(iMessage){
		case WM_CREATE:
			SetRect(&crt, 0,0,800,600);
			AdjustWindowRect(&crt, WS_OVERLAPPEDWINDOW, FALSE);
			SetWindowPos(hWnd, NULL, crt.left, crt.top, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
			SetTimer(hWnd, 1, 10, NULL);
			return 0;

		case WM_GETMINMAXINFO:
			lpmi = (LPMINMAXINFO)lParam;

			lpmi->ptMinTrackSize.x = crt.right - crt.left;
			lpmi->ptMinTrackSize.y = crt.bottom - crt.top;
			lpmi->ptMaxTrackSize.x = crt.right - crt.left;
			lpmi->ptMaxTrackSize.y = crt.bottom - crt.top;
			return 0;

		case WM_TIMER:
			hdc = GetDC(hWnd);
			hMemDC = CreateCompatibleDC(hdc);

			if(hBitmap == NULL){
				hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
			}

			hOld = (HBITMAP)SelectObject(hMemDC, hBitmap);
			FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

			GetCursorPos(&Mouse);
			ScreenToClient(hWnd, &Mouse);

			Base = {arTriangle[0].x, arTriangle[0].y};
			V[0] = {Mouse.x - Base.x, Mouse.y - Base.y};	// AP
			V[1] = {arTriangle[1].x - Base.x, arTriangle[1].y - Base.y}; // AB

			Base = {arTriangle[1].x, arTriangle[1].y};
			V[2] = {Mouse.x - Base.x, Mouse.y - Base.y};
			V[3] = {arTriangle[2].x - Base.x, arTriangle[2].y - Base.y};

			Base = {arTriangle[2].x, arTriangle[2].y};
			V[4] = {Mouse.x - Base.x, Mouse.y - Base.y};
			V[5] = {arTriangle[0].x - Base.x, arTriangle[0].y - Base.y};

			Normalize(&V[0]);
			Normalize(&V[1]);
			Normalize(&V[2]);
			Normalize(&V[3]);
			Normalize(&V[4]);
			Normalize(&V[5]);

			CrossResult[0] = Cross(V[0], V[1]);
			CrossResult[1] = Cross(V[2], V[3]);
			CrossResult[2] = Cross(V[4], V[5]);

			for(i=0; i<3; i++){
				DrawLine(hMemDC, arTriangle[i], arTriangle[i+1]);
			}
			/*
				이 연산으로 나오는 결과값은 Z축의 방향을 의미한다.
				즉, 외적 연산은 내적 연산과 달리 순서가 바뀔 경우 Scalar값은 동일할지 모르나 방향이 바뀐다.
				3차원에서도 마찬가지지만, 2차원 좌표계에서도 외적의 결과값은 대부분 방향 판별에 사용된다.
				즉, 법선 벡터(두 벡터로부터 수직인 벡터)가 시계 방향이냐 반시계 방향이냐의 의미를 갖는다는 것이다.
				음수 값일 경우 반시계방향, 양수값일 경우 시계방향이며 이는 월드 좌표계를 어떻게 설정했느냐에 따라
				방향이 달라질 수 있다.

				임의의 벡터 A가 있다고 치자.
				A벡터는 시간이 흘러 어떠한 방향으로 이동하였고 그 벡터를 B라고 부른다.
				이때 B벡터가 되기 위해 A벡터가 어느 방향으로 이동하였는지 판별하고 싶다면 외적의 결과를 불러오면 된다.

				원점(O)으로부터 벡터 A와 벡터 B 즉, OA, OB에 대한 벡터를 만들고 외적 연산을 하면 이 결과값을 보고 방향을 판단할 수 있다는 것이다.

				아마  작성자와 같은 초보자라면 외적 연산을 수행할 때 어떤 벡터끼리 연산해야할지 감이 잡히지 않을 것이다.

				위에서 말한대로 외적의 결과는 방향을 나타낸다는 것만 떠올리면 의외로 쉽게 이해할 수 있다.
				연산의 순서는 중요하지만 점의 순서따위는 중요하지 않다는 점을 기억하자.

				OA라는 벡터와 마우스 벡터를 외적하고 마우스 벡터와 OB라는 벡터를 연산했다고 가정하자.

				이때 연산에 사용된 각각의 벡터와 결과값(말 그대로 특정 수치값)은 중요하지 않으며
				앞서 얘기했듯 OA와 마우스, 마우스와 OB 벡터가 전부 같은 부호인지만 판별한다.

				도출된 결과로부터 부호를 판별하면 같은 방향을 향하고 있는지 알 수 있다.

				현재까지 컴퓨터 그래픽을 다루는데에서 외적은 그저 방향을 나타낸다는 것에 의미를 두고 공부하면 될 것 같다.

				참고로 가장 중요한 월드 공간, 로컬 공간 / 왼손, 오른손 좌표계에 대한 개념도 알고있어야 하는데
				그 범위는 어떻게 되는지, 좌표계를 변환하는 방법에 대해서도 숙지하고 있어야 한다.
			 */

			if(CrossResult[0] >= 0 && CrossResult[1] >= 0 && CrossResult[2] >= 0)
			{
				TRIVERTEX vert[3];
				GRADIENT_TRIANGLE grt;

				vert[0].x = 400; vert[0].y = 150;
				vert[0].Red = 0; vert[0].Green = 0xff00; vert[0].Blue = 0; vert[0].Alpha = 0;
				vert[1].x = 200; vert[1].y = 450;
				vert[1].Red = 0; vert[1].Green = 0; vert[1].Blue = 0xff00; vert[1].Alpha = 0;
				vert[2].x = 600; vert[2].y = 450;
				vert[2].Red = 0xff00; vert[2].Green = 0; vert[2].Blue = 0; vert[2].Alpha = 0;

				grt.Vertex1 = 0;
				grt.Vertex2 = 1;
				grt.Vertex3 = 2;

				GradientFill(hMemDC, vert, 3, &grt, 1, GRADIENT_FILL_TRIANGLE);
			}
			wsprintf(lpszText, TEXT("Mouse Coord (%d, %d)"), Mouse.x, Mouse.y);
			TextOut(hMemDC, 10, 16, lpszText, lstrlen(lpszText));

			sprintf(lpszText, TEXT("Cross (%.3f, %.3f, %.3f)"), CrossResult[0], CrossResult[1], CrossResult[2]);
			TextOut(hMemDC, 10, 32, lpszText, lstrlen(lpszText));

			SelectObject(hMemDC, hOld);
			DeleteDC(hMemDC);
			ReleaseDC(hWnd, hdc);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(hBitmap){
				hMemDC = CreateCompatibleDC(hdc);
				hOld = (HBITMAP)SelectObject(hMemDC, hBitmap);

				GetObject(hBitmap, sizeof(BITMAP), &bmp);

				BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);
				
				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
			}
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			if(hBitmap){DeleteObject(hBitmap);}
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
