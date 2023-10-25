#define _WIN32_WINNT 0x0A00
#include <windows.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define CLASS_NAME TEXT("Camera Object")

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow)
{
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

	RECT crt;
	SetRect(&crt, 0,0, 800, 600);
	AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE | WS_EX_COMPOSITED,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				crt.right - crt.left, crt.bottom - crt.top,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(1)
	{
		if(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT){return 0;}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// TODO : Update, Render, Time Tick
			WaitMessage();
		}
	}

	return (int)msg.wParam;
}


struct Player
{
	int x,y;
};

struct Camera
{
	int x,y;
};

void* loadbmp(LPCTSTR Path, BITMAPINFOHEADER* pih);
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask);

Player P1;
Camera cam;


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hBit = NULL, bmpBack = NULL, hPlayer = NULL, hCamera = NULL, bmpBlt = NULL;
	static int iRadius;
	static int cx, cy;
	LPCTSTR lpszPath = TEXT("test.bmp");
	LPCTSTR lpszPath2 = TEXT("player.bmp");

	switch(iMessage)
	{
		case WM_CREATE:
			SetTimer(hWnd, 1, 10, NULL);
			SendMessage(hWnd, WM_TIMER, 1, 0);
			{
				BITMAPINFOHEADER ih;
				BYTE* pData = (BYTE*)loadbmp(lpszPath, &ih);

				BITMAPINFOHEADER nih;
				BYTE* pPlayer = (BYTE*)loadbmp(lpszPath2, &nih);
				
				if(pData)
				{
					HDC hdc = GetDC(hWnd);
					HDC hMemDC= CreateCompatibleDC(hdc);
					hBit = CreateCompatibleBitmap(hdc, ih.biWidth, ih.biHeight);

					HGDIOBJ hOld = SelectObject(hMemDC, hBit);
					SetDIBitsToDevice(hMemDC, 0,0, ih.biWidth, ih.biHeight, 0,0,0, ih.biHeight, pData, (BITMAPINFO*)&ih, DIB_RGB_COLORS);
					SelectObject(hMemDC, hOld);

					HDC hdcPlayer = CreateCompatibleDC(hdc);
					hPlayer = CreateCompatibleBitmap(hdc, 210, 100);
					hOld = SelectObject(hdcPlayer, hPlayer);
					SetDIBitsToDevice(hdcPlayer, 0,0, nih.biWidth, nih.biHeight, 0,0,0, nih.biHeight, pPlayer, (BITMAPINFO*)&nih, DIB_RGB_COLORS);
					SelectObject(hdcPlayer, hOld);
					
					DeleteDC(hMemDC);
					DeleteDC(hdcPlayer);
					ReleaseDC(hWnd, hdc);

					P1 = {400,300};
					cam = {400,300};
					iRadius = 50;

					free(pData);
				}
			}
			return 0;

		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
				lpmmi->ptMaxTrackSize.x = 800;
				lpmmi->ptMaxTrackSize.y = 600;
				lpmmi->ptMinTrackSize.y = 800;
				lpmmi->ptMinTrackSize.y = 600;
				lpmmi->ptMaxSize.x = 800;
				lpmmi->ptMaxSize.y = 600;
			}
			return 0;

		case WM_TIMER:
			if(hBit != NULL) {
				RECT crt;
				GetClientRect(hWnd, &crt);

				cx = crt.right - crt.left;
				cy = crt.bottom - crt.top;

				BITMAP bmp;
				GetObject(hBit, sizeof(BITMAP), &bmp);

				HDC hdc = GetDC(hWnd);
				HDC hMemDC = CreateCompatibleDC(hdc);
				HGDIOBJ hOld = SelectObject(hMemDC, hBit);

				HDC hdcMem = CreateCompatibleDC(hdc);
				if(bmpBack == NULL)
				{
					bmpBack = CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);
					// bmpBack = CreateCompatibleBitmap(hdc, cx, cy);
				}
				HGDIOBJ hBackOld = SelectObject(hdcMem, bmpBack);

				if(GetAsyncKeyState(VK_LEFT) & 0x8000)
				{
					P1 = {P1.x - 10, P1.y};
				}
				if(GetAsyncKeyState(VK_RIGHT) & 0x8000)
				{
					P1 = {P1.x + 10, P1.y};
				}
				if(GetAsyncKeyState(VK_UP) & 0x8000)
				{
					P1 = {P1.x, P1.y - 10};
				}
				if(GetAsyncKeyState(VK_DOWN) & 0x8000)
				{
					P1 = {P1.x, P1.y + 10};
				}

				BitBlt(hdcMem, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0,SRCCOPY);
				
				#define max(a,b) ((a) ^ ((a)^(b)) & -((a)<(b)))
				#define min(a,b) ((b) ^ ((a)^(b)) & -((a)<(b)))
				P1.x = max(0, min(P1.x, bmp.bmWidth - 210));
				P1.y = max(0, min(P1.y, bmp.bmHeight - 100));
				cam = {P1.x, P1.y};
				cam.x = max(400, min(cam.x, bmp.bmWidth - 400));
				cam.y = max(300, min(cam.y, bmp.bmHeight - 300));

				HDC hdcPlayer = CreateCompatibleDC(hdc);
				HGDIOBJ hOldPlayer = SelectObject(hdcPlayer, hPlayer);
				TransparentBlt(hdcMem, P1.x, P1.y, 210, 100, hdcPlayer, 0, 0, 210, 100, RGB(255, 0, 255));
				SelectObject(hdcPlayer, hOldPlayer);
				DeleteDC(hdcPlayer);

				HDC hdcCamera = CreateCompatibleDC(hdc);
				if(hCamera == NULL)
				{
					hCamera = CreateCompatibleBitmap(hdc, iRadius, iRadius);
				}

				HGDIOBJ hOldCamera = SelectObject(hdcCamera, hCamera);
				RECT srt;
				SetRect(&srt, 0,0,iRadius, iRadius);
				FillRect(hdcCamera, &srt, GetSysColorBrush(COLOR_WINDOW));
				Ellipse(hdcCamera, 0,0, iRadius, iRadius);
				DrawText(hdcCamera, TEXT("\ncamera"), -1, &srt, DT_CENTER);
				StretchBlt(hdcMem, cam.x, cam.y, iRadius, iRadius, hdcCamera, 0,0, iRadius, iRadius, SRCCOPY);
				SelectObject(hdcCamera, hOldCamera);
				DeleteDC(hdcCamera);

				TCHAR lpszText[128];
				wsprintf(lpszText, TEXT("P1 = (%d, %d)"), P1.x, P1.y);
				TextOut(hdcMem, 10, 16, lpszText, lstrlen(lpszText));

				HDC hdcBlt = CreateCompatibleDC(hdc);
				if(bmpBlt == NULL)
				{
					bmpBlt = CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);
				}
				HGDIOBJ hOldBlt = SelectObject(hdcBlt, bmpBlt);
				StretchBlt(hdcBlt, -(cam.x - cx / 2), -(cam.y - cy / 2), bmp.bmWidth, bmp.bmHeight, hdcMem, 0,0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
				SelectObject(hdcBlt, hOldBlt);
				DeleteDC(hdcBlt);

				SelectObject(hdcMem, hBackOld);
				SelectObject(hMemDC, hOld);
				DeleteDC(hdcMem);
				DeleteDC(hMemDC);
				ReleaseDC(hWnd, hdc);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				
				if(hBit)
				{
					BITMAP bmp;
					GetObject(hBit, sizeof(BITMAP), &bmp);

					HDC hdcBlt= CreateCompatibleDC(hdc);
					HGDIOBJ hOldBlt = SelectObject(hdcBlt, bmpBlt);

					BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hdcBlt, 0,0, SRCCOPY);

					SelectObject(hdcBlt, hOldBlt);
					DeleteDC(hdcBlt);
				}
				
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_DESTROY:
			if(hBit){DeleteObject(hBit);}
			if(hPlayer){DeleteObject(hPlayer);}
			if(bmpBack){DeleteObject(bmpBack);}
			if(hCamera){DeleteObject(hCamera);}
			if(bmpBlt){DeleteObject(bmpBlt);}
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}
	
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void* loadbmp(LPCTSTR Path, BITMAPINFOHEADER* pih)
{
	void* buf = NULL;

	HANDLE hFile = CreateFile(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwRead;
		SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);
		ReadFile(hFile, pih, sizeof(BITMAPINFOHEADER), &dwRead, NULL);
		buf = malloc(pih->biSizeImage);
		ReadFile(hFile, buf, pih->biSizeImage, &dwRead, NULL);
	}else
	{
		MessageBox(NULL, TEXT("cannot open the file"), TEXT("Warning"), MB_OK);
	}

	CloseHandle(hFile);

	return buf;
}

void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask)
{
	BITMAP bm;
	COLORREF cColor;
	HBITMAP bmAndBack, bmAndObject, bmAndMem, bmSave;
	HBITMAP bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
	HDC		hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
	POINT	ptSize;

	hdcTemp = CreateCompatibleDC(hdc);
	SelectObject(hdcTemp, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP(hdcTemp, &ptSize,1);

	hdcBack   = CreateCompatibleDC(hdc);
	hdcObject = CreateCompatibleDC(hdc);
	hdcMem    = CreateCompatibleDC(hdc);
	hdcSave   = CreateCompatibleDC(hdc);

	bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
	bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
	bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
	bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

	// 참고로 bmAndBack, bmAndObject는 전부 흑백 비트맵이다
	bmBackOld   = (HBITMAP) SelectObject(hdcBack, bmAndBack);
	bmObjectOld = (HBITMAP) SelectObject(hdcObject, bmAndObject);

	// 얘네 둘은 화면 dc에 호환되는 색상면과 비트수를 가진 비트맵
	bmMemOld    = (HBITMAP) SelectObject(hdcMem, bmAndMem);
	bmSaveOld   = (HBITMAP) SelectObject(hdcSave, bmSave);

	// 좌표체계를 화면 DC와 동일하게 만든다.
	SetMapMode(hdcTemp, GetMapMode(hdc));

	BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

	// 기본 배경색은 흰색이며 clrMask 값으로 배경색을 바꾼 후 아래 ROP모드로 비트 연산 한다.
	// ROP 모드의 연산이 화면의 배경, 미리 출력된 비트맵, 현재 비트맵, 브러시 등과 행해지므로 이런 설정이 필요하다.

	// 참고로, ROP모드는 비트맵의 비트, 화면에 이미 출력된 비트, 그리고 현재 DC에 선택된 브러시 비트를 논리 조합하는 연산을 일컫는다.
	// 모든 DC는 GDI 오브젝트를 모두 기본값으로 기억해둔다(비트맵은 0, 브러시는 WHITE, 텍스트는 BLACK 등).
	cColor = SetBkColor(hdcTemp, clrMask);

	BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);
	// 여기서 표현하고자 하는 물체외에 비트맵의 배경이 흰색으로 채워진다.

	SetBkColor(hdcTemp, cColor);

	// 투명하게 처리할 배경색을 미리 설정해두고 그 DC를 hdcBack이라는 이름의 DC에 반전해서 복사한다.
	// 반전이란 색이 켜져있으면 끈다는 것이지 색을 반전하는게 아니다.
	BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

	// 비트맵을 출력하고자 하는 목적지 즉, 인자로 전달받은 hdc를 그대로 복사한다.
	BitBlt(hdcMem , 0, 0, ptSize.x, ptSize.y, hdc      , x, y, SRCCOPY);

	// 이 함수에서 중요한 네 가지 연산이다
	BitBlt(hdcMem , 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack  , 0, 0, SRCAND);
	BitBlt(hdcMem , 0, 0, ptSize.x, ptSize.y, hdcTemp  , 0, 0, SRCPAINT);
	BitBlt(hdc    , x, y, ptSize.x, ptSize.y, hdcMem   , 0, 0, SRCCOPY);

	// 얘는 그냥 복구용
	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave  , 0, 0, SRCCOPY);


	DeleteObject(SelectObject(hdcBack, bmBackOld));
	DeleteObject(SelectObject(hdcObject, bmObjectOld));
	DeleteObject(SelectObject(hdcMem, bmMemOld));
	DeleteObject(SelectObject(hdcSave, bmSaveOld));
	
	DeleteDC(hdcMem);
	DeleteDC(hdcBack);
	DeleteDC(hdcObject);
	DeleteDC(hdcSave);
	DeleteDC(hdcTemp);
}
