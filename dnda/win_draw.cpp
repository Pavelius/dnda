#include "crt.h"
#include "draw.h"
#include "win.h"

#pragma pack(push)
#pragma pack(1)
static struct video_8t
{
	BITMAPINFO			bmp;
	unsigned char		bmp_pallette[256*4];
} video_descriptor;
#pragma pack(pop)

static draw::window*	current_window;
static bool				use_mouse = true;

static int tokey(int vk)
{
	switch(vk)
	{
	case VK_CONTROL: return Ctrl;
	case VK_MENU: return Alt;
	case VK_SHIFT: return Shift;
	case VK_LEFT: return KeyLeft;
	case VK_RIGHT: return KeyRight;
	case VK_UP: return KeyUp;
	case VK_DOWN: return KeyDown;
	case VK_PRIOR: return KeyPageUp;
	case VK_NEXT: return KeyPageDown;
	case VK_HOME: return KeyHome;
	case VK_END: return KeyEnd;
	case VK_BACK: return KeyBackspace;
	case VK_DELETE: return KeyDelete;
	case VK_RETURN: return KeyEnter;
	case VK_ESCAPE: return KeyEscape;
	case VK_SPACE: return KeySpace;
	case VK_TAB: return KeyTab;
	case VK_F1: return F1;
	case VK_F2: return F2;
	case VK_F3: return F3;
	case VK_F4: return F4;
	case VK_F5: return F5;
	case VK_F6: return F6;
	case VK_F7: return F7;
	case VK_F8: return F8;
	case VK_F9: return F9;
	case VK_F10: return F10;
	case VK_F11: return F11;
	case VK_F12: return F12;
	case VK_MULTIPLY: return Alpha+(unsigned)'*';
	case VK_DIVIDE: return Alpha+(unsigned)'/';
	case VK_ADD: return Alpha+(unsigned)'+';
	case VK_SUBTRACT: return Alpha+(unsigned)'-';
	case VK_OEM_COMMA: return Alpha+(unsigned)',';
	case VK_OEM_PERIOD: return Alpha+(unsigned)'.';
	default: return Alpha+vk;
	}
}

static void set_cursor(cursors e)
{
	static void* data[] =
	{
		LoadCursorA(0, (char*)32512),//IDC_ARROW
		LoadCursorA(0, (char*)32649),//IDC_HAND
		LoadCursorA(0, (char*)32644),//IDC_SIZEWE
		LoadCursorA(0, (char*)32645),//IDC_SIZENS
		LoadCursorA(0, (char*)32646),//IDC_SIZEALL
		LoadCursorA(0, (char*)32648),//IDC_NO
		LoadCursorA(0, (char*)32513),//IDC_IBEAM
		LoadCursorA(0, (char*)32514),//IDC_WAIT
	};
	SetCursor(data[e]);
}

extern bool		sys_optimize_mouse_move;
extern rect		sys_static_area;

static int handle(HWND hwnd, MSG& msg)
{
	POINT pt;
	TRACKMOUSEEVENT tm;
	switch(msg.message)
	{
	case WM_MOUSEMOVE:
		if(msg.hwnd!=hwnd)
			break;
		if(!use_mouse)
			break;
		memset(&tm, 0, sizeof(tm));
		tm.cbSize = sizeof(tm);
		tm.dwFlags = TME_LEAVE|TME_HOVER;
		tm.hwndTrack = hwnd;
		tm.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tm);
		hot::mouse.x = LOWORD(msg.lParam);
		hot::mouse.y = HIWORD(msg.lParam);
		if(!draw::drag::id && draw::mouseinput && sys_optimize_mouse_move)
		{
			if(hot::mouse.in(sys_static_area))
				return InputNoUpdate;
		}
		return MouseMove;
	case WM_MOUSELEAVE:
		if(msg.hwnd!=hwnd)
			break;
		if(!use_mouse)
			break;
		GetCursorPos(&pt);
		ScreenToClient(msg.hwnd, &pt);
		hot::mouse.x = (short)pt.x;
		if(hot::mouse.x<0)
			hot::mouse.x = -10000;
		hot::mouse.y = (short)pt.y;
		if(hot::mouse.y<0)
			hot::mouse.y = -10000;
		return MouseMove;
	case WM_MOUSEHOVER:
		if(!use_mouse)
			break;
		return InputIdle;
	case WM_LBUTTONDOWN:
		if(msg.hwnd!=hwnd)
			break;
		if(!use_mouse)
			break;
		hot::pressed = true;
		return MouseLeft;
	case WM_LBUTTONDBLCLK:
		if(msg.hwnd!=hwnd)
			break;
		if(!use_mouse)
			break;
		hot::pressed = true;
		return MouseLeftDBL;
	case WM_LBUTTONUP:
		if(msg.hwnd!=hwnd)
			break;
		if(!use_mouse)
			break;
		if(!hot::pressed)
			break;
		hot::pressed = false;
		return MouseLeft;
	case WM_RBUTTONDOWN:
		if(!use_mouse)
			break;
		hot::pressed = true;
		return MouseRight;
	case WM_RBUTTONUP:
		if(!use_mouse)
			break;
		hot::pressed = false;
		return MouseRight;
	case WM_MOUSEWHEEL:
		if(!use_mouse)
			break;
		if(msg.wParam&0x80000000)
			return MouseWheelDown;
		else
			return MouseWheelUp;
		break;
	case WM_TIMER:
		if(msg.hwnd!=hwnd)
			break;
		if(msg.wParam==InputTimer)
			return InputTimer;
		break;
	case WM_KEYDOWN:
		return tokey(msg.wParam);
	case WM_KEYUP:
		return InputKeyUp;
	case WM_CHAR:
		hot::param = msg.wParam;
		return InputSymbol;
	case WM_MY_SIZE:
	case WM_SIZE:
		return InputUpdate;
	case WM_COMMAND:
		if(HIWORD(msg.wParam)==0)
			return msg.lParam;
		break;
	}
	return 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	switch(uMsg)
	{
	case WM_ERASEBKGND:
		if(draw::canvas)
		{
			RECT rc; GetClientRect(hwnd, &rc);
			video_descriptor.bmp.bmiHeader.biSize = sizeof(video_descriptor.bmp.bmiHeader);
			video_descriptor.bmp.bmiHeader.biWidth = draw::canvas->width;
			video_descriptor.bmp.bmiHeader.biHeight = -draw::canvas->height;
			video_descriptor.bmp.bmiHeader.biBitCount = draw::canvas->bpp;
			video_descriptor.bmp.bmiHeader.biPlanes = 1;
			if(draw::canvas->bpp == 8 && draw::palt)
				memcpy(video_descriptor.bmp_pallette, draw::palt, 4*256);
			if(rc.right!= draw::canvas->width || rc.bottom!= draw::canvas->height)
				StretchDIBits((void*)wParam,
					0, 0, rc.right, rc.bottom,
					0, 0, draw::canvas->width, draw::canvas->height,
					draw::canvas->bits, &video_descriptor.bmp, DIB_RGB_COLORS, SRCCOPY);
			else
				SetDIBitsToDevice((void*)wParam,
					0, 0, rc.right, rc.bottom,
					0, 0, 0, draw::canvas->height,
					draw::canvas->bits, &video_descriptor.bmp, DIB_RGB_COLORS);
		}
		return 1;
	case WM_CLOSE:
		PostQuitMessage(-1);
		return 0;
	case WM_EXITSIZEMOVE:
	case WM_SIZE:
		if(!PeekMessageA(&msg, hwnd, WM_MY_SIZE, WM_MY_SIZE, 0))
			PostMessageA(hwnd, WM_MY_SIZE, 0, 0);
		return 0;
	case WM_GETMINMAXINFO:
		if(current_window)
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = current_window->minimum.x;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = current_window->minimum.y;
		}
		return 0;
	case WM_SETCURSOR:
		if(LOWORD(lParam)==HTCLIENT)
		{
			set_cursor(hot::cursor);
			return 1;
		}
		break;
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static const char* register_class(const char* class_name)
{
	WNDCLASS wc;
	if(!GetClassInfoA(GetModuleHandleA(0), class_name, &wc))
	{
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; // Own DC For Window.
		wc.lpfnWndProc = WndProc;					// WndProc Handles Messages
		wc.hInstance = GetModuleHandleA(0);			// Set The Instance
		wc.hIcon = (void*)LoadIconA(wc.hInstance, (const char*)1); // WndProc Handles Messages
		wc.lpszClassName = class_name;				// Set The Class Name
		RegisterClassA(&wc);						// Attempt To Register The Window Class
	}
	return class_name;
}

static void update_main_window()
{
	if(!current_window)
		return;
	HWND hwnd = current_window->hwnd;
	if(!hwnd)
		return;
	if(!IsWindowVisible(hwnd))
		ShowWindow(hwnd, SW_SHOW);
	InvalidateRect(hwnd, 0, 1);
	UpdateWindow(hwnd);
}

draw::window* draw::getwindow()
{
	return current_window;
}

void draw::syscursor(bool enable)
{
	ShowCursor(enable ? 1 : 0);
}

void draw::sysmouse(bool enable)
{
	use_mouse = enable;
}

draw::window::window(int x, int y, int width, int height, unsigned flags, int bpp, const char* identifier):
	parent(current_window), flags(flags), identifier(identifier), focus(0),
	hwnd(0), gc(0)
{
	if(!bpp)
		bpp = draw::canvas->bpp;
	if(!width)
		width = (GetSystemMetrics(SM_CXFULLSCREEN)/3)*2;
	if(!height)
		height = (GetSystemMetrics(SM_CYFULLSCREEN)/3)*2;
	unsigned dwStyle = WS_CAPTION|WS_VISIBLE; // Windows Style;
	HWND parent = GetActiveWindow();
	if(parent)
		dwStyle |= WS_POPUP;
	else
		dwStyle |= WS_SYSMENU;
	// custom
	if(flags&WFResize)
		dwStyle |= WS_THICKFRAME;
	else
		dwStyle |= WS_BORDER;
	if(flags&WFMinmax)
	{
		dwStyle |= WS_MINIMIZEBOX;
		if(flags&WFResize)
			dwStyle |= WS_MAXIMIZEBOX;
	}
	if(flags&WFMaximized)
		dwStyle |= WS_MAXIMIZE;
	RECT MinimumRect = {0, 0, width, height};
	AdjustWindowRectEx(&MinimumRect, dwStyle, 0, 0);
	minimum.x = (short)(MinimumRect.right-MinimumRect.left);
	minimum.y = (short)(MinimumRect.bottom-MinimumRect.top);
	if(x==-1)
		x = (GetSystemMetrics(SM_CXFULLSCREEN) - minimum.x)/2;
	if(y==-1)
		y = (GetSystemMetrics(SM_CYFULLSCREEN) - minimum.y)/2;
	position.x1 = x;
	position.y1 = y;
	position.x2 = position.x1 + minimum.x;
	position.y2 = position.y1 + minimum.y;
	// Set default window colors
	hot::cursor = CursorArrow;
	draw::font = metrics::font;
	draw::fore = colors::text;
	opening();
	// Update current surface
	current_window = this;
	resize(width, height, bpp, true);
	draw::canvas = this;
	setclip();
	// Create The Window
	hwnd = CreateWindowExA(0, register_class("CFaceWindow"), 0, dwStyle,
		position.x1, position.y1,
		position.width(), position.height(),
		parent,
		0, GetModuleHandleA(0), 0);
	if(!hwnd)
		return;
	// Disable parent
	if(parent)
	{
		SetFocus(hwnd);
		EnableWindow(parent, 0);
	}
	// Update mouse coordinates
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);
	hot::mouse.x = (short)pt.x;
	hot::mouse.y = (short)pt.y;
}

static void update_size(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	if(current_window)
		current_window->resizing((rect&)rc);
}

draw::window::~window()
{
	if(GetCapture()==hwnd)
		ReleaseCapture();
	else
	{
		HWND parent = GetParent(hwnd);
		if(parent)
		{
			EnableWindow(parent, 1);
			SetFocus(parent);
		}
	}
	if(hwnd)
	{
		if(identifier)
		{
			WINDOWPLACEMENT wp;
			GetWindowPlacement(hwnd, &wp);
			position.x1 = wp.rcNormalPosition.left;
			position.x2 = wp.rcNormalPosition.right;
			position.y1 = wp.rcNormalPosition.top;
			position.y2 = wp.rcNormalPosition.bottom;
			closing();
		}
		DestroyWindow(hwnd);
		hwnd = 0;
	}
	current_window = parent;
	if(current_window)
		update_size(current_window->hwnd);
}

void draw::sysredraw()
{
	MSG	msg;
	if(!current_window)
		return;
	update_main_window();
	void* hwnd = current_window->hwnd;
	if(!hwnd)
		return;
	while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		handle(hwnd, msg);
	}
}

int draw::rawinput()
{
	MSG	msg;
	if(!current_window)
		return 0;
	update_main_window();
	void* hwnd = current_window->hwnd;
	if(!hwnd)
		return 0;
	while(GetMessageA(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		hot::key = handle(hwnd, msg);
		if(hot::key == InputNoUpdate)
			continue;
		if(hot::key)
		{
			if(hot::key != MouseMove && hot::key >= (int)MouseLeft)
			{
				if(GetKeyState(VK_SHIFT)<0)
					hot::key |= Shift;
				if(GetKeyState(VK_MENU)<0)
					hot::key |= Alt;
				if(GetKeyState(VK_CONTROL)<0)
					hot::key |= Ctrl;
			}
			if(hot::key == InputUpdate)
				update_size(hwnd);
			return hot::key;
		}
	}
	return 0;
}

void draw::setcaption(const char* string)
{
	if(!draw::getwindow())
		return;
	HWND hwnd = draw::getwindow()->hwnd;
	SetWindowTextA(hwnd, string);
}

void draw::settimer(unsigned milleseconds)
{
	if(!draw::getwindow())
		return;
	HWND hwnd = draw::getwindow()->hwnd;
	if(milleseconds)
		SetTimer(hwnd, InputTimer, milleseconds, 0);
	else
		KillTimer(hwnd, InputTimer);
}

void draw::clipboard::copy(const void* string, int lenght)
{
	if(!lenght)
		return;
	if(!OpenClipboard(0))
		return;
	EmptyClipboard();
	void* hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (lenght + 1) * sizeof(char));
	if(!hglbCopy)
	{
		CloseClipboard();
		return;
	}
	// Lock the handle and copy the text to the buffer.
	auto ps = (char*)GlobalLock(hglbCopy);
	memcpy(ps, string, lenght); ps[lenght] = 0;
	GlobalUnlock(ps);
	SetClipboardData(CF_TEXT, hglbCopy);
	CloseClipboard();
}

int draw::clipboard::paste(void* data, int maxlenght)
{
	if(!IsClipboardFormatAvailable(CF_TEXT))
		return 0;
	if(!OpenClipboard(0))
		return 0;
	int result = 0;
	void* hglb = GetClipboardData(CF_TEXT);
	if(hglb)
	{
		result = GlobalSize(hglb);
		if(maxlenght && result>maxlenght)
			result = maxlenght;
		if(data && result)
		{
			char* p = (char*)GlobalLock(hglb);
			if(p)
			{
				memcpy(data, p, result);
				GlobalUnlock(hglb);
			}
		}
	}
	CloseClipboard();
	return result;
}

char** szcmdargv(int& argc)
{
	static char* argv[128];
	memset(argv, 0, sizeof(argv));
	argc = 0;
	const char* p = GetCommandLineA();
	while(*p)
	{
		if(argc >= sizeof(argv) / sizeof(argv[0]))
			break;
		p = szskipcr(p);
		const char* pe = 0;
		if(*p == '\"' || *p == '\'')
		{
			char t = *p++;
			pe = zchr(p, t);
		}
		else
			pe = zchr(p, ' ');
		if(!pe)
			pe = zend(p);
		char* psz = new char[pe - p + 1];
		zcpy(psz, p, pe - p);
		argv[argc++] = psz;
		if(pe[0] == 0)
			break;
		p = zskipspcr(pe + 1);
	}
	return argv;
}