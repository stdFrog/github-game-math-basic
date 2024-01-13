#include "basewin.h"
#include "vector.h"
#include <vector>
#include <strsafe.h>

using namespace std;

class MainWindow : public BaseWindow<MainWindow>
{
	HANDLE hTimer;
	HBITMAP hBit;
	BOOL InitializeTimer();
	vector<Vector<float>> obj;
	Vector<float> Ball, Move;
	// vector<Vector> obj;

	public:
		MainWindow() : hTimer(NULL), hBit(NULL)
		{
			// TODO : Something Var Initialize
		}

		Vector<float> Input();
		// Vector Input();
		void WaitTimer();
		void DrawLine(HDC hdc, int x, int y, int dx, int dy);
		void DrawLine(HDC hdc, POINT start, POINT end);
		void DrawLine(HDC hdc, Vector<float> start, Vector<float> end);
		void DrawBitmap(HDC hdc, int x, int y);
		void GetSinCos(float *sinf, float *cosf, float angle);

		void OnCreate(WPARAM wParam, LPARAM lParam);
		void OnPaint(WPARAM wParam, LPARAM lParam);
		void OnTimer(WPARAM wParam, LPARAM lParam);

		LPCWSTR ClassName() const {return L"Viewing Angle Example Class";}
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow win;

	if(!win.Create(L"Viewing Angle"))
	{
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);

	MSG msg = {};
	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		win.WaitTimer();
	}

	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
			OnCreate(wParam, lParam);
			
			return 0;

		case WM_DESTROY:
			KillTimer(m_hWnd, 1);
			CloseHandle(hTimer);
DeleteObject(hBit);
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
			OnPaint(wParam, lParam);
			return 0;

		case WM_TIMER:
			OnTimer(wParam, lParam);
			return 0;

		default:
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
}

BOOL MainWindow::InitializeTimer()
{
	hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

	if(hTimer == NULL)
	{
		return FALSE;
	}

	LARGE_INTEGER li = {0};
	if(!SetWaitableTimer(hTimer, &li, (1000/60), NULL, NULL, FALSE))
	{
		CloseHandle(hTimer);
		hTimer = NULL;
		return FALSE;
	}

	return TRUE;
}

void MainWindow::WaitTimer()
{
	if(MsgWaitForMultipleObjects(1, &hTimer, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0)
	{
		InvalidateRect(m_hWnd, NULL, FALSE);
	}
}

void MainWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	if(!InitializeTimer())
	{
		return;
	}

	SetTimer(m_hWnd, 1, 10, NULL);

	if(obj.empty())
	{
		float radius = 10.f;
		for(float x = -radius; x <= radius; x++)
		{
			for(float y = -radius; y <= radius; y++)
			{
				Vector pt = Vector(x,y);
				float squared = pt.Squared();
				if(squared <= radius * radius)
				{
					obj.push_back(Vector(x,y));
				}
			}
		}
	}

	Ball = Vector(50.f, 50.f);
	Move = Vector(6.f, 8.f);
}

void MainWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(hBit != NULL)
	{
		DrawBitmap(hdc, 0,0);
	}

	EndPaint(m_hWnd, &ps);
}

void MainWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
	static RECT crt;
	static Vector<float> InputVector, curPos;
	// static Vector InputVector, curPos;
	static float deltaX = 0.f, deltaY = 0.f, speed = 5.f, fovAngle = 60.f;
	static float halfCos = cos((fovAngle * 0.5f) * (atan(1.0) * 4.0) / 180.f);
	static COLORREF color = RGB(0,0,0);

	switch(wParam)
	{
		case 1:
			{
				HDC hdc = GetDC(m_hWnd);
				HDC hMemDC = CreateCompatibleDC(hdc);
				GetClientRect(m_hWnd, &crt);
				if(hBit == NULL)
				{
					hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
				}
				HGDIOBJ hOld = SelectObject(hMemDC, hBit);
				FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

				// TODO : Ball Object Drawing
				{
					HPEN hPen = CreatePen(PS_INSIDEFRAME, 5, RGB(255,0,0));
					HPEN hOldPen = (HPEN)SelectObject(hMemDC, hPen);
					HBRUSH hBrush = CreateSolidBrush(RGB(0,0,255));
					HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);

					#define R 20
					if(Ball.x <= R || Ball.x >= crt.right - R)
					{
						Move.x *= -1.f;
					}
					if(Ball.y <= R || Ball.y >= crt.bottom - R)
					{
						Move.y *= -1.f;
					}
					Ball.x += Move.x;
					Ball.y += Move.y;
					Ellipse(hMemDC, Ball.x - R, Ball.y - R, Ball.x + R, Ball.y + R);

					DeleteObject(SelectObject(hMemDC, hOldPen));
					DeleteObject(SelectObject(hMemDC, hOldBrush));
				}

				// TODO : Drawing
				{
					float LineWidth = 300.f;
					float sinf, cosf;
					GetSinCos(&sinf, &cosf, fovAngle * 0.5f);
					DrawLine(hMemDC, curPos, curPos + -(Vector(LineWidth * sinf, LineWidth * cosf)));
					DrawLine(hMemDC, curPos, curPos + -(Vector(-LineWidth * sinf, LineWidth * cosf)));
					DrawLine(hMemDC, curPos, curPos + -(Vector(0.f,1.f,0.f) * LineWidth * 0.2f));

					InputVector = Input().GetNormalize() * speed;
					TCHAR str[1024];
					StringCbPrintf(str, sizeof(str), L"InputVector = { %.2f, %.2f }", InputVector.x, InputVector.y);
					TextOut(hMemDC, 10,10, str,lstrlen(str));
					curPos += InputVector;

					Vector f = Vector(0.f, 1.f, 0.f);
					Vector v = (Ball - curPos).GetNormalize();

					if(v.Dot(f) <= (-halfCos))
					{
						// TODO : in sight ball
						color = RGB(255, 0,0);
					}else{
						color = RGB(0,0,0);
					}


					for(auto const& v : obj)
					{
						SetPixel(hMemDC, curPos.x + v.x, curPos.y + v.y, color);
					}
				}
				
				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
				ReleaseDC(m_hWnd, hdc);
			}
			InvalidateRect(m_hWnd, NULL, FALSE);
			break;
	}
}


void MainWindow::DrawBitmap(HDC hdc, int x, int y)
{
	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBit);

	BITMAP bmp;
	GetObject(hBit, sizeof(BITMAP), &bmp);

	BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

Vector<float> MainWindow::Input()
{
	BYTE keyboard[256];

	if(GetKeyboardState(keyboard))
	{
		for(int key = 0; key < sizeof(keyboard)/sizeof(keyboard[0]); key++)
		{
			if(keyboard[key] & 0x80)
			{
				float XAxis = 0.f;
				float YAxis = 0.f;

				switch(key)
				{
					case VK_LEFT:
						XAxis = -1.f;
						break;
					case VK_RIGHT:
						XAxis = 1.f;
						break;

					case VK_UP:
						YAxis = -1.f;
						break;
					case VK_DOWN:
						YAxis = 1.f;
						break;
				}

				return Vector((float)XAxis, (float)YAxis);
			}
		}
	}
	return Vector<float>(0.f, 0.f);
}

/*
Vector MainWindow::Input()
{
	BYTE keyboard[256];

	if(GetKeyboardState(keyboard))
	{
		for(int key = 0; key < sizeof(keyboard)/sizeof(keyboard[0]); key++)
		{
			if(keyboard[key] & 0x80)
			{
				float XAxis = 1.f, YAxis = 1.f;
				return Vector(XAxis, YAxis);
			}
		}
	}
	return Vector(0.f, 0.f);
}
*/

void MainWindow::DrawLine(HDC hdc, int x, int y, int dx, int dy)
{
	MoveToEx(hdc, x, y, NULL);
	LineTo(hdc, dx, dy);
}

void MainWindow::DrawLine(HDC hdc, POINT start, POINT end)
{
	DrawLine(hdc, start.x, start.y, end.x, end.y);
}

void MainWindow::DrawLine(HDC hdc, Vector<float> start, Vector<float> end)
{
	DrawLine(hdc, (int)start.x, (int)start.y, (int)end.x, (int)end.y);
}

void MainWindow::GetSinCos(float *sinf, float *cosf, float angle)
{
	if(angle == 0.f)
	{
		*sinf = 0.f;
		*cosf = 1.f;
	}else if(angle == 90.f)
	{
		*sinf = 1.f;
		*cosf = 0.f;
	}else if(angle == 180.f)
	{
		*sinf = 0.f;
		*cosf = -1.f;
	}else if(angle == 270.f)
	{
		*sinf = -1.f;
		*cosf = 0.f;
	}else{
		float PI = atan(1.f) * 4.f;
		*sinf = sin(angle * PI / 180.f);
		*cosf = cos(angle * PI / 180.f);
	}
}
