#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "imm32.lib")

struct Suit {
	const wchar_t* symbol;
	const wchar_t* name;
	COLORREF color;
};
struct CardValue {
	const wchar_t* label;
	int value;
};
struct PipPos {
	int dx;
	int dy;
};
class TextRenderer {
public:
	static void Draw(
		HDC hdc,
		int x, int y,
		const std::wstring& text,
		int fontSize,
		COLORREF color,
		UINT textAlign,
		int angle10 = 0,
		const wchar_t* fontName = L"Segoe UI Symbol",
		int fontWeight = FW_NORMAL)
	{
		LOGFONTW lf = {};
		lf.lfHeight = -fontSize;
		lf.lfEscapement = angle10;
		lf.lfOrientation = angle10;
		lf.lfWeight = fontWeight;
		lf.lfCharSet = DEFAULT_CHARSET;
		lstrcpyW(lf.lfFaceName, fontName);
		HFONT hFont = CreateFontIndirectW(&lf);
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
		COLORREF oldColor = SetTextColor(hdc, color);
		int oldBkMode = SetBkMode(hdc, TRANSPARENT);
		UINT oldAlign = SetTextAlign(hdc, textAlign);
		TextOutW(hdc, x, y, text.c_str(), static_cast<int>(text.length()));
		SetTextAlign(hdc, oldAlign);
		SetBkMode(hdc, oldBkMode);
		SetTextColor(hdc, oldColor);
		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
	}
};
class PipLayout {
public:
	static std::vector<PipPos> GetPositions(int count) {
		int top = -55;
		int bottom = 55;
		int v31 = top / 2;
		int v32 = -v31;
		int v41 = top / 3;
		int v42 = -v41;
		int v51 = (top + v41) / 2;
		int v52 = -v51;
		int mid = 0;
		int cen = 0;
		int left = -25;
		int right = 25;
		std::vector<std::vector<PipPos>> positions(11);
		positions[1] = { {cen, mid} };
		positions[2] = { {cen, top}, {cen, bottom} };
		positions[3] = { {cen, top}, {cen, mid}, {cen, bottom} };
		positions[4] = { {left, top}, {right, top}, {left, bottom}, {right, bottom} };
		positions[5] = { {left, top}, {right, top}, {cen, mid}, {left, bottom}, {right, bottom} };
		positions[6] = { {left, top}, {right, top}, {left, mid}, {right, mid}, {left, bottom}, {right, bottom} };
		positions[7] = { {left, top}, {right, top}, {cen, v31}, {left, mid}, {right, mid}, {left, bottom}, {right, bottom} };
		positions[8] = { {left, top}, {right, top}, {cen, v31}, {left, mid}, {right, mid}, {cen, v32}, {left, bottom}, {right, bottom} };
		positions[9] = { {left, top}, {right, top}, {cen, v51}, {left, v41}, {right, v41}, {left, v42}, {right, v42}, {left, bottom}, {right, bottom} };
		positions[10] = { {left, top}, {right, top}, {cen, v51}, {left, v41}, {right, v41}, {left, v42}, {right, v42}, {cen, v52}, {left, bottom}, {right, bottom} };
		if (count < 1 || count > 10) return {};
		return positions[count];
	}
};
class Card {
private:
	Suit suit_;
	CardValue value_;
	static constexpr int WIDTH = 120;
	static constexpr int HEIGHT = 180;
	static constexpr int PADDING = 10;
public:
	Card(const CardValue& value, const Suit& suit)
		: suit_(suit), value_(value) {
	}
	void Draw(HDC hdc, int left, int top) const {
		DrawCardFrame(hdc, left, top);
		int innerLeft = left + PADDING;
		int innerTop = top + PADDING;
		int innerW = WIDTH - 2 * PADDING;
		int innerH = HEIGHT - 2 * PADDING;
		int innerCx = innerLeft + innerW / 2;
		int innerCy = innerTop + innerH / 2;
		DrawTopLeftCorner(hdc, innerLeft, innerTop);
		DrawBottomRightCorner(hdc, left, top);
		if (value_.value >= 1 && value_.value <= 10) {
			DrawPips(hdc, value_.value, innerCx, innerCy);
		}
		else {
			DrawFaceCard(hdc, innerCx, innerCy);
		}
	}
	static int GetWidth() { return WIDTH; }
	static int GetHeight() { return HEIGHT; }

private:
	void DrawCardFrame(HDC hdc, int left, int top) const {
		HBRUSH hWhite = CreateSolidBrush(RGB(255, 255, 255));
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hWhite);
		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		RoundRect(hdc, left, top, left + WIDTH, top + HEIGHT, 10, 10);
		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		DeleteObject(hWhite);
		DeleteObject(hPen);
	}
	void DrawTopLeftCorner(HDC hdc, int innerLeft, int innerTop) const {
		TextRenderer::Draw(
			hdc,
			innerLeft,
			innerTop + 16,
			value_.label,
			16,
			suit_.color,
			TA_LEFT | TA_BASELINE,
			0,
			L"Segoe UI",
			FW_NORMAL
		);
		TextRenderer::Draw(
			hdc,
			innerLeft,
			innerTop + 32,
			suit_.symbol,
			16,
			suit_.color,
			TA_LEFT | TA_BASELINE,
			0,
			L"Segoe UI Symbol",
			FW_NORMAL
		);
	}
	void DrawBottomRightCorner(HDC hdc, int left, int top) const {
		TextRenderer::Draw(
			hdc,
			left + WIDTH - PADDING * 2,
			top + HEIGHT - PADDING * 2 - 16,
			suit_.symbol,
			16,
			suit_.color,
			TA_RIGHT | TA_BASELINE,
			1800,
			L"Segoe UI Symbol",
			FW_NORMAL
		);
		TextRenderer::Draw(
			hdc,
			left + WIDTH - PADDING * 2,
			top + HEIGHT - PADDING * 2,
			value_.label,
			16,
			suit_.color,
			TA_RIGHT | TA_BASELINE,
			1800,
			L"Segoe UI",
			FW_NORMAL
		);
	}
	void DrawPips(HDC hdc, int count, int cx, int cy) const {
		auto positions = PipLayout::GetPositions(count);
		for (const auto& p : positions) {
			int x = cx + p.dx;
			int y = cy + p.dy;
			if (p.dy < 5) {
				TextRenderer::Draw(
					hdc,
					x, y,
					suit_.symbol,
					24,
					suit_.color,
					TA_CENTER | TA_BASELINE,
					0,
					L"Segoe UI Symbol",
					FW_NORMAL
				);
			}
			else {
				TextRenderer::Draw(
					hdc,
					x, y,
					suit_.symbol,
					24,
					suit_.color,
					TA_CENTER | TA_BASELINE,
					1800,
					L"Segoe UI Symbol",
					FW_NORMAL
				);
			}
		}
	}
	void DrawFaceCard(HDC hdc, int cx, int cy) const {
		TextRenderer::Draw(
			hdc,
			cx,
			cy + 14,
			value_.label,
			40,
			suit_.color,
			TA_CENTER | TA_BASELINE,
			0,
			L"Segoe UI",
			FW_BOLD
		);
	}
};

class PokerWindowApp {
	enum class GameMode {
		NONE,
		SINGLE_PLAYER,
		TWO_PLAYER
	};

	enum class Player {
		NONE,
		PLAYER_A,
		PLAYER_B
	};
private:
	HINSTANCE hInstance_;
	HWND hwnd_;
	int currentSuitIndex_ = 0;
	int currentValueIndex_ = 0;
	int currentShownSeqIndex_ = 0;
	int nextSeqIndex_ = 0;
	bool running_ = false;
	bool hasCurrentItem_ = false;
	ULONGLONG shownTick_ = 0;
	std::mt19937 rng_;
	std::vector<int> deckOrder_;
	size_t deckPos_ = 0;
	bool anyMatchOccurred_ = false;
	static constexpr const wchar_t* CLASS_NAME = L"PokerCardWin32MenuReaction";
	static constexpr UINT TIMER_ID = 1001;
	static constexpr UINT ID_MENU_START = 40001;
	static constexpr UINT ID_MENU_STOP = 40002;
	static constexpr UINT ID_MENU_EXIT = 40003;
	static constexpr UINT ID_SINGLE = 10001;
	static constexpr UINT ID_DOUBLE = 10002;
	static constexpr UINT ID_MULTIPLE = 10003;
	static constexpr UINT ID_HELP_ABOUT = 10004;

	GameMode mode_ = GameMode::NONE;
	int matchWinner_ = 0;
	bool currentCardProcessed_ = false;

	// 將 DLL 載入常駐於記憶體
	HMODULE hDll_ = nullptr;
	typedef void (*CheckFunc)(HWND);
	CheckFunc checkFunc_ = nullptr;

public:
	explicit PokerWindowApp(HINSTANCE hInstance)
		: hInstance_(hInstance), hwnd_(nullptr), rng_(std::random_device{}()) {
	}
	bool Create(int nCmdShow) {
		WNDCLASSW wc = {};
		wc.lpfnWndProc = PokerWindowApp::WndProc;
		wc.hInstance = hInstance_;
		wc.lpszClassName = CLASS_NAME;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		RegisterClassW(&wc);
		hwnd_ = CreateWindowExW(
			0,
			CLASS_NAME,
			L"Win32 API 撲克牌反應測試",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			1000, 700,
			nullptr,
			nullptr,
			hInstance_,
			this
		);
		if (!hwnd_) return false;

		hDll_ = LoadLibraryW(L"s1142018.dll");
		if (hDll_) {
			checkFunc_ = (CheckFunc)GetProcAddress(hDll_, "check");
		}

		SetMenu(hwnd_, CreateAppMenu());
		ShowWindow(hwnd_, nCmdShow);
		UpdateWindow(hwnd_);
		return true;
	}
	int Run() {
		MSG msg = {};
		while (GetMessageW(&msg, nullptr, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam);
	}
private:
	HMENU CreateAppMenu() {
		HMENU hMenuBar = CreateMenu();
		HMENU hGameMenu = CreatePopupMenu();
		AppendMenuW(hGameMenu, MF_STRING, ID_MENU_START, L"開始");
		AppendMenuW(hGameMenu, MF_STRING, ID_MENU_STOP, L"停止");
		AppendMenuW(hGameMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenuW(hGameMenu, MF_STRING, ID_MENU_EXIT, L"結束");
		AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hGameMenu, L"單人計時賽");
		AppendMenuW(hMenuBar, MF_STRING, ID_DOUBLE, L"雙人鍵盤競速");
		AppendMenuW(hMenuBar, MF_STRING, ID_MULTIPLE, L"多人連線競賽");
		AppendMenuW(hMenuBar, MF_STRING, ID_HELP_ABOUT, L"說明");
		return hMenuBar;
	}
	static const std::array<Suit, 4>& GetSuits() {
		static const std::array<Suit, 4> suits = {
		Suit{ L"\x2660", L"spade", RGB(0, 0, 0) },
		Suit{ L"\x2665", L"heart", RGB(220, 0, 0) },
		Suit{ L"\x2666", L"diamond", RGB(220, 0, 0) },
		Suit{ L"\x2663", L"club", RGB(0, 0, 0) }
		};
		return suits;
	}
	static const std::array<CardValue, 13>& GetCardValues() {
		static const std::array<CardValue, 13> values = {
		CardValue{ L"A", 1 }, CardValue{ L"2", 2 }, CardValue{ L"3", 3 },
		CardValue{ L"4", 4 }, CardValue{ L"5", 5 }, CardValue{ L"6", 6 },
		CardValue{ L"7", 7 }, CardValue{ L"8", 8 }, CardValue{ L"9", 9 },
		CardValue{ L"10", 10 }, CardValue{ L"J", 11 }, CardValue{ L"Q", 12 },
		CardValue{ L"K", 13 }
		};
		return values;
	}
	static const std::array<const wchar_t*, 13>& GetSequenceLabels() {
		static const std::array<const wchar_t*, 13> seq = {
		L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10", L"J", L"Q", L"K"
		};
		return seq;
	}
	int RandomIntervalMs() {
		int minMs = (mode_ == GameMode::TWO_PLAYER) ? 1500 : 100;
		int maxMs = (mode_ == GameMode::TWO_PLAYER) ? 2500 : 1000;
		std::uniform_int_distribution<int> dist(minMs, maxMs);
		return dist(rng_);
	}
	void BuildShuffledDeck() {
		deckOrder_.clear();
		deckOrder_.reserve(52);
		for (int i = 0; i < 52; ++i) {
			deckOrder_.push_back(i);
		}
		std::shuffle(deckOrder_.begin(), deckOrder_.end(), rng_);
		deckPos_ = 0;
	}
	bool IsCardAndTextMatched() const {
		const auto& values = GetCardValues();
		const auto& seq = GetSequenceLabels();
		std::wstring cardText = values[currentValueIndex_].label;
		std::wstring textShown = seq[currentShownSeqIndex_];
		if (cardText == L"A" && textShown == L"1") {
			return true;
		}
		return cardText == textShown;
	}
	void StartSequence(GameMode selectedMode) {
		StopSequence();
		mode_ = selectedMode;
		running_ = true;
		hasCurrentItem_ = false;
		nextSeqIndex_ = 0;
		currentShownSeqIndex_ = 0;
		anyMatchOccurred_ = false;
		matchWinner_ = 0;
		currentCardProcessed_ = false;
		BuildShuffledDeck();
		ShowNextItem();
		ScheduleNextTimer();
		InvalidateRect(hwnd_, nullptr, TRUE);
	}
	void StopSequence() {
		KillTimer(hwnd_, TIMER_ID);
		running_ = false;
	}
	void HandleTwoPlayerKeyPress(Player player) {
		if (!running_ || !hasCurrentItem_ || currentCardProcessed_) return;
		bool isMatch = IsCardAndTextMatched();
		currentCardProcessed_ = true;
		if (isMatch) {
			matchWinner_ = (player == Player::PLAYER_A) ? 1 : 2;
		}
		else {
			matchWinner_ = (player == Player::PLAYER_A) ? 2 : 1;
		}
		StopSequence();
		InvalidateRect(hwnd_, nullptr, TRUE);
		HandleGameEnd();
	}
	void ScheduleNextTimer() {
		KillTimer(hwnd_, TIMER_ID);
		SetTimer(hwnd_, TIMER_ID, RandomIntervalMs(), nullptr);
	}
	bool ShowNextItem() {
		currentCardProcessed_ = false;
		if (deckPos_ >= deckOrder_.size()) {
			return false;
		}
		int cardId = deckOrder_[deckPos_++];
		currentSuitIndex_ = cardId / 13;
		currentValueIndex_ = cardId % 13;
		currentShownSeqIndex_ = nextSeqIndex_;
		nextSeqIndex_ = (nextSeqIndex_ + 1) % 13;
		hasCurrentItem_ = true;
		if (IsCardAndTextMatched()) {
			anyMatchOccurred_ = true;
		}
		InvalidateRect(hwnd_, nullptr, TRUE);
		return true;
	}
	void HandleGameEnd() {
		StopSequence();
		if (mode_ == GameMode::SINGLE_PLAYER) {
			if (!anyMatchOccurred_) {
				MessageBoxW(hwnd_, L"52 張牌全部顯示完畢，結果：失敗", L"失敗", MB_OK);
			}
		}
		else if (mode_ == GameMode::TWO_PLAYER) {
			std::wstringstream ss;
			ss << L"【一局定勝負 賽事結果】\n\n";
			if (matchWinner_ == 1) {
				ss << L"🎉 恭喜 玩家 A (A鍵) 獲勝！";
			}
			else if (matchWinner_ == 2) {
				ss << L"🎉 恭喜 玩家 B (L鍵) 獲勝！";
			}
			else {
				ss << L"🤝 52張牌發完，無人觸發勝負條件。平手！";
			}
			MessageBoxW(hwnd_, ss.str().c_str(), L"比賽結束", MB_OK | MB_ICONINFORMATION);
		}
	}
	void HandleSpacePressed() {
		if (!running_ || !hasCurrentItem_) return;
		ULONGLONG now = GetTickCount64();
		ULONGLONG elapsed = now - shownTick_;
		bool correct = IsCardAndTextMatched();
		StopSequence();
		const auto& seq = GetSequenceLabels();
		const auto& values = GetCardValues();
		std::wstring textShown = seq[currentShownSeqIndex_];
		std::wstring cardText = values[currentValueIndex_].label;
		std::wstringstream ss;
		ss << L"撲克牌：" << cardText << L"\n"
			<< L"右側數字：" << textShown << L"\n"
			<< L"結果：" << (correct ? L"答對" : L"失敗") << L"\n"
			<< L"反應時間：" << elapsed << L" 毫秒";
		MessageBoxW(
			hwnd_,
			ss.str().c_str(),
			correct ? L"答對" : L"失敗",
			MB_OK | (correct ? MB_ICONINFORMATION : MB_ICONWARNING)
		);
	}
	void OnPaint() {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd_, &ps);
		RECT rc;
		GetClientRect(hwnd_, &rc);
		HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
		FillRect(hdc, &rc, bg);
		DeleteObject(bg);
		if (hasCurrentItem_) {
			if (mode_ == GameMode::TWO_PLAYER) {
				TextRenderer::Draw(
					hdc, rc.right / 2, 50, L"雙人模式", 24,
					RGB(255, 60, 60), TA_CENTER | TA_BASELINE, 0, L"Microsoft JhengHei UI", FW_BOLD
				);
			}
			const auto& suits = GetSuits();
			const auto& values = GetCardValues();
			const auto& seq = GetSequenceLabels();
			Card card(values[currentValueIndex_], suits[currentSuitIndex_]);
			int cardW = Card::GetWidth();
			int cardH = Card::GetHeight();
			int centerX = (rc.left + rc.right) / 2;
			int centerY = (rc.top + rc.bottom) / 2 + 20;
			int cardLeft = centerX - cardW / 2;
			int cardTop = centerY - cardH / 2;
			card.Draw(hdc, cardLeft, cardTop);
			TextRenderer::Draw(
				hdc,
				cardLeft + cardW + 100,
				centerY + 20,
				seq[currentShownSeqIndex_],
				64,
				RGB(0, 90, 180),
				TA_CENTER | TA_BASELINE,
				0,
				L"Segoe UI",
				FW_BOLD
			);
			std::wstringstream ss;
			ss << L"第 " << deckPos_ << L" / 52 張";
			TextRenderer::Draw(
				hdc,
				30,
				rc.bottom - 30,
				ss.str(),
				18,
				RGB(80, 80, 80),
				TA_LEFT | TA_BASELINE,
				0,
				L"Microsoft JhengHei UI",
				FW_NORMAL
			);
		}
		EndPaint(hwnd_, &ps);
	}
	LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
			case ID_MENU_START:
				StartSequence(GameMode::SINGLE_PLAYER);
				return 0;
			case ID_DOUBLE:
				StartSequence(GameMode::TWO_PLAYER);
				return 0;
			case ID_MENU_STOP:
				StopSequence();
				return 0;
			case ID_MENU_EXIT:
				DestroyWindow(hwnd_);
				return 0;
			case ID_HELP_ABOUT:
				MessageBoxW(
					hwnd_,
					L"玩法：\n1. 單人賽：點選【開始】，數字吻合時按下 [空白鍵]\n2. 雙人賽：點選【雙人鍵盤競速】，進入刺激的一局定勝負模式！\n   當數字吻合時，玩家A按 [A]、玩家B按 [L] 搶答。\n   率先按對者直接獲勝！若亂按（不吻合卻按），則直接判對手獲勝！",
					L"說明",
					MB_OK | MB_ICONINFORMATION
				);
				return 0;
			}
			break;
		}
					   // 將原本的 WM_KEYDOWN 判定遷移至 WM_CHAR，徹底對齊 DLL 的開槍格式
		case WM_CHAR: {
			if (!running_ || !hasCurrentItem_) break;
			if (mode_ == GameMode::SINGLE_PLAYER) {
				// 接收來自 DLL 發送的 32 (空白鍵)
				if (wParam == 32) {
					HandleSpacePressed();
					return 0;
				}
			}
			else if (mode_ == GameMode::TWO_PLAYER) {
				if (wParam == 'A' || wParam == 'a') {
					HandleTwoPlayerKeyPress(Player::PLAYER_A);
					return 0;
				}
				else if (wParam == 'L' || wParam == 'l') {
					HandleTwoPlayerKeyPress(Player::PLAYER_B);
					return 0;
				}
			}
			break;
		}
		case WM_TIMER: {
			if (wParam == TIMER_ID && running_) {
				if (deckPos_ < deckOrder_.size()) {
					ShowNextItem();
					UpdateWindow(hwnd_);

					if (checkFunc_) {
						shownTick_ = GetTickCount64();
						checkFunc_(hwnd_);
					}

					ScheduleNextTimer();
				}
				else {
					HandleGameEnd();
				}
			}
			return 0;
		}
		case WM_PAINT:
			OnPaint();
			return 0;
		case WM_DESTROY:
			KillTimer(hwnd_, TIMER_ID);
			if (hDll_) FreeLibrary(hDll_);
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProcW(hwnd_, msg, wParam, lParam);
	}
public:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		PokerWindowApp* pThis = nullptr;
		if (msg == WM_NCCREATE) {
			CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
			pThis = static_cast<PokerWindowApp*>(cs->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
			pThis->hwnd_ = hwnd;
		}
		else {
			pThis = reinterpret_cast<PokerWindowApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		}
		if (pThis) {
			return pThis->HandleMessage(msg, wParam, lParam);
		}
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
};
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	PokerWindowApp app(hInstance);
	if (!app.Create(nCmdShow)) {
		return 0;
	}

	HWND hwndGame = FindWindowW(L"PokerCardWin32MenuReaction", nullptr);
	if (hwndGame) {
		ImmAssociateContext(hwndGame, NULL);
	}
	return app.Run();
}