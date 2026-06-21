#include "pch.h"
#include <windows.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>

struct CaptureBuffer {
    std::vector<BYTE> pixels;
    int width, height;
    inline BYTE B(int x, int y) const { return pixels[(y * width + x) * 4 + 0]; }
    inline BYTE G(int x, int y) const { return pixels[(y * width + x) * 4 + 1]; }
    inline BYTE R(int x, int y) const { return pixels[(y * width + x) * 4 + 2]; }
};

struct Box {
    int left = -1, top = -1, right = -1, bottom = -1;
    bool valid() const { return left >= 0 && right > left && bottom > top; }
};

struct Blob { Box box; int pixelCount; };

enum Rank {
    RANK_UNKNOWN = 0,
    R_A = 1, R_2, R_3, R_4, R_5, R_6, R_7, R_8, R_9, R_10, R_J, R_Q, R_K
};

template<typename T> T Mn(T a, T b) { return a < b ? a : b; }
template<typename T> T Mx(T a, T b) { return a > b ? a : b; }
template<typename T> T Clampv(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

void LogDebug(const std::wstring& line) {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring path = std::wstring(tempPath) + L"s1142018_debug.log";
    FILE* f = nullptr;
    _wfopen_s(&f, path.c_str(), L"a, ccs=UTF-8");
    if (f) {
        fwprintf(f, L"%s\n", line.c_str());
        fclose(f);
    }
}

std::vector<BYTE> CaptureClientArea(HWND hwnd, int& w, int& h) {
    HDC hdcWin = GetDC(hwnd);
    RECT rc; GetClientRect(hwnd, &rc);
    w = rc.right - rc.left;
    h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) { ReleaseDC(hwnd, hdcWin); return {}; }

    HDC hdcMem = CreateCompatibleDC(hdcWin);
    HBITMAP hbm = CreateCompatibleBitmap(hdcWin, w, h);
    HBITMAP old = (HBITMAP)SelectObject(hdcMem, hbm);
    BitBlt(hdcMem, 0, 0, w, h, hdcWin, 0, 0, SRCCOPY);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<BYTE> buf(w * h * 4);
    GetDIBits(hdcMem, hbm, 0, h, buf.data(), &bmi, DIB_RGB_COLORS);

    SelectObject(hdcMem, old);
    DeleteObject(hbm);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWin);
    return buf;
}

Box FindCardBorder(const CaptureBuffer& buf) {
    int w = buf.width, h = buf.height;
    auto isBlackish = [&](int x, int y) {
        return buf.R(x, y) < 60 && buf.G(x, y) < 60 && buf.B(x, y) < 60;
        };

    std::vector<int> rowMaxRun(h, 0), colMaxRun(w, 0);
    for (int y = 0; y < h; y++) {
        int run = 0, best = 0;
        for (int x = 0; x < w; x++) {
            if (isBlackish(x, y)) { run++; if (run > best) best = run; }
            else run = 0;
        }
        rowMaxRun[y] = best;
    }
    for (int x = 0; x < w; x++) {
        int run = 0, best = 0;
        for (int y = 0; y < h; y++) {
            if (isBlackish(x, y)) { run++; if (run > best) best = run; }
            else run = 0;
        }
        colMaxRun[x] = best;
    }

    int rowPeak = *std::max_element(rowMaxRun.begin(), rowMaxRun.end());
    int colPeak = *std::max_element(colMaxRun.begin(), colMaxRun.end());
    if (rowPeak < 20 || colPeak < 20) return {};

    Box box;
    for (int y = 0; y < h; y++)
        if (rowMaxRun[y] > rowPeak * 3 / 4) { if (box.top < 0) box.top = y; box.bottom = y; }
    for (int x = 0; x < w; x++)
        if (colMaxRun[x] > colPeak * 3 / 4) { if (box.left < 0) box.left = x; box.right = x; }
    return box;
}

COLORREF EstimateBackgroundColor(const CaptureBuffer& buf, Box region) {
    std::map<DWORD, int> hist;
    for (int y = region.top; y <= region.bottom; y += 2)
        for (int x = region.left; x <= region.right; x += 2) {
            DWORD key = (buf.R(x, y) << 16) | (buf.G(x, y) << 8) | buf.B(x, y);
            hist[key]++;
        }
    DWORD best = 0; int bestCount = -1;
    for (auto& kv : hist) if (kv.second > bestCount) { bestCount = kv.second; best = kv.first; }
    return RGB((best >> 16) & 0xFF, (best >> 8) & 0xFF, best & 0xFF);
}

std::vector<Blob> FindBlobs(const CaptureBuffer& buf, Box region, COLORREF bg, int distThresh = 2500) {
    int w = region.right - region.left + 1, h = region.bottom - region.top + 1;
    if (w <= 0 || h <= 0) return {};
    std::vector<char> visited(w * h, 0);
    int bgR = GetRValue(bg), bgG = GetGValue(bg), bgB = GetBValue(bg);
    auto isFg = [&](int gx, int gy) {
        int dr = buf.R(gx, gy) - bgR, dg = buf.G(gx, gy) - bgG, db = buf.B(gx, gy) - bgB;
        return dr * dr + dg * dg + db * db > distThresh;
        };

    std::vector<Blob> blobs;
    std::vector<POINT> stack;
    const int dx[8] = { -1,0,1,-1,1,-1,0,1 };
    const int dy[8] = { -1,-1,-1,0,0,1,1,1 };

    for (int ly = 0; ly < h; ly++) {
        for (int lx = 0; lx < w; lx++) {
            int idx = ly * w + lx;
            if (visited[idx]) continue;
            int gx = region.left + lx, gy = region.top + ly;
            if (!isFg(gx, gy)) { visited[idx] = 1; continue; }

            Box box{ gx,gy,gx,gy };
            int count = 0;
            stack.clear();
            stack.push_back({ lx, ly });
            visited[idx] = 1;
            while (!stack.empty()) {
                POINT p = stack.back(); stack.pop_back();
                int px = region.left + p.x, py = region.top + p.y;
                count++;
                box.left = Mn(box.left, px);
                box.right = Mx(box.right, px);
                box.top = Mn(box.top, py);
                box.bottom = Mx(box.bottom, py);
                for (int k = 0; k < 8; k++) {
                    int nx = p.x + dx[k], ny = p.y + dy[k];
                    if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                    int nidx = ny * w + nx;
                    if (visited[nidx]) continue;
                    visited[nidx] = 1;
                    if (isFg(region.left + nx, region.top + ny)) stack.push_back({ nx, ny });
                }
            }
            if (count > 3) blobs.push_back({ box, count });
        }
    }
    return blobs;
}

std::vector<Box> ClusterBoxes(const std::vector<Blob>& blobs, int gapThresh) {
    std::vector<Box> boxes;
    for (auto& b : blobs) boxes.push_back(b.box);
    bool merged = true;
    while (merged) {
        merged = false;
        for (size_t i = 0; i < boxes.size() && !merged; i++) {
            for (size_t j = i + 1; j < boxes.size(); j++) {
                Box& a = boxes[i]; Box& b = boxes[j];
                bool closeX = !(a.right + gapThresh < b.left || b.right + gapThresh < a.left);
                bool closeY = !(a.bottom + gapThresh < b.top || b.bottom + gapThresh < a.top);
                if (closeX && closeY) {
                    a.left = Mn(a.left, b.left);
                    a.right = Mx(a.right, b.right);
                    a.top = Mn(a.top, b.top);
                    a.bottom = Mx(a.bottom, b.bottom);
                    boxes.erase(boxes.begin() + j);
                    merged = true;
                    break;
                }
            }
        }
    }
    return boxes;
}

Box MergeBoxList(const std::vector<Box>& list) {
    if (list.empty()) return {};
    Box m = list[0];
    for (auto& b : list) {
        m.left = Mn(m.left, b.left);
        m.right = Mx(m.right, b.right);
        m.top = Mn(m.top, b.top);
        m.bottom = Mx(m.bottom, b.bottom);
    }
    return m;
}

Box ExtractGlyphMerged(const std::vector<Blob>& blobs, int gapThresh) {
    return MergeBoxList(ClusterBoxes(blobs, gapThresh));
}

// 提示文字：依垂直位置把色塊分組成「行」，挑總面積最大的那一行，整行內所有色塊合併
Box ExtractDominantGlyph(const std::vector<Blob>& blobs, int gapThresh = 10) {
    auto clusters = ClusterBoxes(blobs, gapThresh);
    if (clusters.empty()) return {};

    std::vector<std::vector<Box>> lines;
    for (auto& c : clusters) {
        bool placed = false;
        for (auto& line : lines) {
            bool overlapsY = false;
            for (auto& existing : line) {
                if (!(c.bottom < existing.top || c.top > existing.bottom)) { overlapsY = true; break; }
            }
            if (overlapsY) { line.push_back(c); placed = true; break; }
        }
        if (!placed) lines.push_back({ c });
    }

    std::vector<Box>* bestLine = &lines[0];
    int bestArea = 0;
    for (auto& line : lines) {
        int area = 0;
        for (auto& b : line) area += (b.right - b.left + 1) * (b.bottom - b.top + 1);
        if (area > bestArea) { bestArea = area; bestLine = &line; }
    }
    return MergeBoxList(*bestLine);
}

std::vector<float> NormalizeGlyph(const CaptureBuffer& buf, Box box, COLORREF bg, int outW, int outH) {
    std::vector<float> out(outW * outH, 0.f);
    if (!box.valid()) return out;
    int srcW = box.right - box.left + 1, srcH = box.bottom - box.top + 1;
    int bgR = GetRValue(bg), bgG = GetGValue(bg), bgB = GetBValue(bg);
    for (int oy = 0; oy < outH; oy++) {
        for (int ox = 0; ox < outW; ox++) {
            int sx = box.left + (int)((ox + 0.5f) / outW * srcW);
            int sy = box.top + (int)((oy + 0.5f) / outH * srcH);
            sx = Clampv(sx, box.left, box.right);
            sy = Clampv(sy, box.top, box.bottom);
            int dr = buf.R(sx, sy) - bgR, dg = buf.G(sx, sy) - bgG, db = buf.B(sx, sy) - bgB;
            out[oy * outW + ox] = sqrtf((float)(dr * dr + dg * dg + db * db)) / 441.7f;
        }
    }
    return out;
}

float Correlation(const std::vector<float>& a, const std::vector<float>& b) {
    int n = (int)a.size();
    float meanA = 0, meanB = 0;
    for (int i = 0; i < n; i++) { meanA += a[i]; meanB += b[i]; }
    meanA /= n; meanB /= n;
    float num = 0, da = 0, db = 0;
    for (int i = 0; i < n; i++) {
        float va = a[i] - meanA, vb = b[i] - meanB;
        num += va * vb; da += va * va; db += vb * vb;
    }
    if (da < 1e-6f || db < 1e-6f) return 0.f;
    return num / sqrtf(da * db);
}

std::vector<float> RenderTemplateGlyph(const std::wstring& label, int fontSize, int weight, int outW, int outH) {
    int canvasW = fontSize * 3 + 20, canvasH = fontSize * 2 + 20;
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, canvasW, canvasH);
    HBITMAP old = (HBITMAP)SelectObject(hdcMem, hbm);

    RECT full{ 0,0,canvasW,canvasH };
    HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdcMem, &full, white);
    DeleteObject(white);

    LOGFONTW lf = {};
    lf.lfHeight = -fontSize;
    lf.lfWeight = weight;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpyW(lf.lfFaceName, L"Segoe UI");
    HFONT hFont = CreateFontIndirectW(&lf);
    HFONT oldFont = (HFONT)SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(0, 0, 0));
    SetBkMode(hdcMem, TRANSPARENT);
    SetTextAlign(hdcMem, TA_LEFT | TA_TOP);
    TextOutW(hdcMem, 10, 10, label.c_str(), (int)label.size());

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = canvasW;
    bmi.bmiHeader.biHeight = -canvasH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    std::vector<BYTE> pixels(canvasW * canvasH * 4);
    GetDIBits(hdcMem, hbm, 0, canvasH, pixels.data(), &bmi, DIB_RGB_COLORS);

    SelectObject(hdcMem, oldFont); DeleteObject(hFont);
    SelectObject(hdcMem, old); DeleteObject(hbm); DeleteDC(hdcMem); ReleaseDC(nullptr, hdcScreen);

    CaptureBuffer cb{ pixels, canvasW, canvasH };
    Box region{ 0,0,canvasW - 1,canvasH - 1 };
    auto blobs = FindBlobs(cb, region, RGB(255, 255, 255));
    if (blobs.empty()) return std::vector<float>(outW * outH, 0.f);
    Box box = ExtractGlyphMerged(blobs, 4);
    return NormalizeGlyph(cb, box, RGB(255, 255, 255), outW, outH);
}

struct TemplateLibrary {
    std::vector<std::pair<int, std::vector<float>>> templates;
    bool calibrated = false;

    void Calibrate(const CaptureBuffer& buf, Box sampleBox, COLORREF bg,
        const std::vector<std::pair<int, std::wstring>>& labelSet,
        int outW, int outH, const wchar_t* tag) {
        auto sample = NormalizeGlyph(buf, sampleBox, bg, outW, outH);
        int inkHeight = sampleBox.bottom - sampleBox.top + 1;
        if (inkHeight < 6) inkHeight = 16;

        static const float scaleFactors[] = { 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f };
        int bestWeight = FW_NORMAL, bestFontSize = inkHeight;
        float bestScore = -2.f;

        for (int weight : {FW_NORMAL, FW_BOLD}) {
            for (float sf : scaleFactors) {
                int fontSize = Mx(6, (int)(inkHeight * sf));
                float localBest = -2.f;
                for (auto& kv : labelSet) {
                    auto tmpl = RenderTemplateGlyph(kv.second, fontSize, weight, outW, outH);
                    float score = Correlation(sample, tmpl);
                    localBest = Mx(localBest, score);
                }
                if (localBest > bestScore) { bestScore = localBest; bestWeight = weight; bestFontSize = fontSize; }
            }
        }

        templates.clear();
        for (auto& kv : labelSet)
            templates.push_back({ kv.first, RenderTemplateGlyph(kv.second, bestFontSize, bestWeight, outW, outH) });
        calibrated = true;

        std::wstringstream ss;
        ss << L"[calibrate:" << tag << L"] weight=" << bestWeight
            << L" fontSize=" << bestFontSize << L" peakScore=" << bestScore
            << L" sampleBox=[" << sampleBox.left << L"," << sampleBox.top << L","
            << sampleBox.right << L"," << sampleBox.bottom << L"]";
        LogDebug(ss.str());
    }

    int Classify(const std::vector<float>& glyph, float& outScore, float& outMargin) const {
        int bestRank = RANK_UNKNOWN;
        float bestScore = -2.f, secondScore = -2.f;
        for (auto& kv : templates) {
            float score = Correlation(glyph, kv.second);
            if (score > bestScore) { secondScore = bestScore; bestScore = score; bestRank = kv.first; }
            else if (score > secondScore) { secondScore = score; }
        }
        outScore = bestScore;
        outMargin = bestScore - secondScore;
        return bestRank;
    }
};

extern "C" __declspec(dllexport) void check(HWND hwnd) {
    static TemplateLibrary hintLib, faceLib;
    static bool hintReady = false, faceReady = false;
    static long long roundCounter = 0;
    long long myRound = ++roundCounter;
    const int OUTW = 20, OUTH = 28;
    const float HINT_MARGIN = 0.10f;
    const float HINT_FLOOR = 0.55f;
    const float FACE_MARGIN = 0.10f;
    const float FACE_FLOOR = 0.55f;

    int w, h;
    auto pixels = CaptureClientArea(hwnd, w, h);
    if (pixels.empty()) { LogDebug(L"#" + std::to_wstring(myRound) + L" skip=capture"); return; }
    CaptureBuffer buf{ pixels, w, h };

    Box cardBox = FindCardBorder(buf);
    if (!cardBox.valid()) { LogDebug(L"#" + std::to_wstring(myRound) + L" skip=no_card_border"); return; }

    int cw = cardBox.right - cardBox.left, ch = cardBox.bottom - cardBox.top;

    // ===== 中央區域：數花色符號顆數（A~10）或辨識孤立大字母（J/Q/K）=====
    Box centerRegion{
        cardBox.left + cw * 20 / 100, cardBox.top + ch * 15 / 100,
        cardBox.right - cw * 20 / 100, cardBox.bottom - ch * 15 / 100
    };
    COLORREF centerBg = EstimateBackgroundColor(buf, centerRegion);
    auto centerBlobs = FindBlobs(buf, centerRegion, centerBg);
    auto centerClusters = ClusterBoxes(centerBlobs, 4);

    int cornerRank = RANK_UNKNOWN;
    float cornerScore = 1.f, cornerMargin = 1.f; // 數顆數的情況直接視為滿分信心
    Box cornerGlyphBox;

    if (centerClusters.empty()) { LogDebug(L"#" + std::to_wstring(myRound) + L" skip=no_center_blob"); return; }

    if (centerClusters.size() >= 2 && centerClusters.size() <= 10) {
        cornerRank = (int)centerClusters.size(); // 顆數直接對應 R_2..R_10
        cornerGlyphBox = MergeBoxList(centerClusters);
    }
    else { // size == 1：可能是 A（單一花色符號）或人臉牌大字母
        Box single = centerClusters[0];
        int hgt = single.bottom - single.top + 1;
        cornerGlyphBox = single;
        if (hgt > 25) {
            if (!faceReady) {
                static const std::vector<std::pair<int, std::wstring>> faceLabels = {
                    {R_J,L"J"},{R_Q,L"Q"},{R_K,L"K"}
                };
                faceLib.Calibrate(buf, single, centerBg, faceLabels, OUTW, OUTH, L"face");
                faceReady = true;
            }
            auto g = NormalizeGlyph(buf, single, centerBg, OUTW, OUTH);
            float s = 0, m = 0;
            cornerRank = faceLib.Classify(g, s, m);
            cornerScore = s; cornerMargin = m;
        }
        else {
            cornerRank = R_A;
        }
    }

    // ===== 右側提示文字：邏輯不變 =====
    Box hintRegion{
        cardBox.right + 10, Mx(0, cardBox.top - 100),
        Mn(w - 1, cardBox.right + 450), Mn(h - 1, cardBox.bottom + 100)
    };
    COLORREF hintBg = EstimateBackgroundColor(buf, hintRegion);
    auto hintBlobs = FindBlobs(buf, hintRegion, hintBg);
    if (hintBlobs.empty()) { LogDebug(L"#" + std::to_wstring(myRound) + L" skip=no_hint_blob"); return; }
    Box hintGlyphBox = ExtractDominantGlyph(hintBlobs);
    if (!hintGlyphBox.valid()) { LogDebug(L"#" + std::to_wstring(myRound) + L" skip=hint_box_invalid"); return; }

    if (!hintReady) {
        static const std::vector<std::pair<int, std::wstring>> hintLabels = {
            {R_A,L"1"},{R_2,L"2"},{R_3,L"3"},{R_4,L"4"},{R_5,L"5"},{R_6,L"6"},{R_7,L"7"},
            {R_8,L"8"},{R_9,L"9"},{R_10,L"10"},{R_J,L"J"},{R_Q,L"Q"},{R_K,L"K"}
        };
        hintLib.Calibrate(buf, hintGlyphBox, hintBg, hintLabels, OUTW, OUTH, L"hint");
        hintReady = true;
    }

    auto hintGlyph = NormalizeGlyph(buf, hintGlyphBox, hintBg, OUTW, OUTH);
    float hintScore = 0, hintMargin = 0;
    int hintRank = hintLib.Classify(hintGlyph, hintScore, hintMargin);

    bool cornerTrusted = (cornerRank != RANK_UNKNOWN && cornerScore >= FACE_FLOOR && cornerMargin >= FACE_MARGIN);
    bool fired = (cornerTrusted && cornerRank == hintRank
        && hintScore >= HINT_FLOOR && hintMargin >= HINT_MARGIN);

    std::wstringstream ss;
    ss << L"#" << myRound
        << L" corner rank=" << cornerRank << L" score=" << cornerScore << L" margin=" << cornerMargin
        << L" pipCount=" << centerClusters.size()
        << L" box=[" << cornerGlyphBox.left << L"," << cornerGlyphBox.top << L","
        << cornerGlyphBox.right << L"," << cornerGlyphBox.bottom << L"]"
        << L" | hint rank=" << hintRank << L" score=" << hintScore << L" margin=" << hintMargin
        << L" box=[" << hintGlyphBox.left << L"," << hintGlyphBox.top << L","
        << hintGlyphBox.right << L"," << hintGlyphBox.bottom << L"]"
        << L" | fired=" << (fired ? L"Y" : L"N");
    LogDebug(ss.str());

    if (fired) {
        SendMessage(hwnd, WM_CHAR, 32, 0);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}