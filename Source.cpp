// Made by RequestFX#1541 original Zer0Mem0ry

#include <Windows.h>

typedef struct tagMonData {
	int current;
	MONITORINFOEXW* info;
} MonData;

BOOL EnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	MonData* data = (MonData*)dwData;
	data->info[data->current].cbSize = sizeof(MONITORINFOEXW);
	return GetMonitorInfoW(hMonitor, &(data->info[data->current++]));
}

BOOL GetAllMonitorInfo(MonData* data) {
	return EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)(&EnumProc), (LPARAM)(data));
}

MonData data;
int cMonitors;
int interval = 100;

LRESULT CALLBACK melter(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	for (int i = 0; i < cMonitors; i++) {
		int x = data.info[i].rcMonitor.left;
		int y = data.info[i].rcMonitor.top;
		int w = data.info[i].rcMonitor.right - x;
		int h = data.info[i].rcMonitor.bottom - y;

		switch (Msg) {
		case WM_CREATE: {
			HDC desktop = GetDC(HWND_DESKTOP);
			HDC window = GetDC(hWnd);

			BitBlt(window, x, y, w, h, desktop, x, y, SRCCOPY);
			ReleaseDC(hWnd, window);
			ReleaseDC(HWND_DESKTOP, desktop);

			SetTimer(hWnd, 0, interval, 0);
			ShowWindow(hWnd, SW_SHOW);
			break;
		}
		case WM_PAINT: {
			ValidateRect(hWnd, 0);
			break;
		}
		case WM_TIMER: {
			HDC window = GetDC(hWnd);
			int X = (rand() % w),
				Y = (rand() % 20 + 1),
				W = (rand() % 50 + 1);
			if (X + W > w)
				X -= X + W - w;

			BitBlt(window, X, Y, W, h, window, X, 0, SRCCOPY | SRCPAINT);
			ReleaseDC(hWnd, window);
			break;
		}

		case WM_DESTROY: {
			KillTimer(hWnd, 0);
			PostQuitMessage(0);
			break;
		}
		}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE Inst, HINSTANCE Prev, LPSTR Cmd, int showcmd) {
	WNDCLASS wndClass = { 0, melter, 0,0, Inst, 0, LoadCursorW(0, IDC_ARROW), 0, 0, L"ScreenMelter" };

	if (RegisterClass(&wndClass)) {
		cMonitors = GetSystemMetrics(SM_CMONITORS);
		data.current = 0;
		data.info = (MONITORINFOEXW*)calloc(cMonitors, sizeof(MONITORINFOEXW));

		if (!GetAllMonitorInfo(&data)) return 1;

		for (int i = 0; i < cMonitors; i++) {
			int x = data.info[i].rcMonitor.left;
			int y = data.info[i].rcMonitor.top;
			int w = data.info[i].rcMonitor.right - x;
			int h = data.info[i].rcMonitor.bottom - y;

			// Create the "melter" overlapping window.
			CreateWindowExA(WS_EX_NOACTIVATE | WS_EX_TOPMOST, "ScreenMelter", 0, WS_POPUP, x, y, w, h, 0, 0, Inst, 0);
		}

		// seed for randomization
		srand(GetTickCount());
		MSG msg = { 0 };
		// Run the melter loop
		while (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// SHIFT+CTRL+ALT+DEL_KEY to exit
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 && GetAsyncKeyState(VK_CONTROL) & 0x8000 &&
				GetAsyncKeyState(VK_SHIFT) & 0x8000 && GetAsyncKeyState(VK_MENU) & 0x8000) break;
		}
	}
	free(data.info);

	return 0;
}