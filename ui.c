#include "ui.h"
#include "core.h"
#include <stdio.h>
#include <string.h>

static HDC g_hMemDC = NULL;
static HBITMAP g_hBmp = NULL;
static HBRUSH g_brushWall   = NULL;
static HBRUSH g_brushFloor  = NULL;
static HBRUSH g_brushFog    = NULL;
static HBRUSH g_brushPlayer = NULL;
static HBRUSH g_brushHUD    = NULL;
static HBRUSH g_brushEnemy  = NULL;
static HBRUSH g_brushBoss   = NULL;
static HBRUSH g_brushGold   = NULL;
static HBRUSH g_brushShop   = NULL;
static HPEN   g_penGrid     = NULL;
static HPEN   g_penBoss     = NULL;

static void ui_init_resources(HWND hwnd) {
    HDC hDC = GetDC(hwnd);
    g_hMemDC = CreateCompatibleDC(hDC);
    g_hBmp = CreateCompatibleBitmap(hDC, WIN_W, WIN_H);
    SelectObject(g_hMemDC, g_hBmp);
    ReleaseDC(hwnd, hDC);

    g_brushWall   = CreateSolidBrush(RGB(40, 40, 50));
    g_brushFloor  = CreateSolidBrush(RGB(100, 100, 120));
    g_brushFog    = CreateSolidBrush(RGB(20, 20, 25));
    g_brushPlayer = CreateSolidBrush(RGB(0, 200, 100));
    g_brushHUD    = CreateSolidBrush(RGB(60, 60, 70));
    g_brushEnemy  = CreateSolidBrush(RGB(200, 50, 50));
    g_brushBoss   = CreateSolidBrush(RGB(180, 50, 200));
    g_brushGold   = CreateSolidBrush(RGB(255, 200, 0));
    g_brushShop   = CreateSolidBrush(RGB(100, 50, 200));
    g_penGrid = CreatePen(PS_SOLID, 1, RGB(80, 80, 90));
    g_penBoss = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
}

void ui_cleanup(void) {
    if (g_hMemDC) {
        SelectObject(g_hMemDC, GetStockObject(NULL_BRUSH));
        SelectObject(g_hMemDC, GetStockObject(NULL_PEN));
        SelectObject(g_hMemDC, GetStockObject(0));
    }
    if (g_hBmp) DeleteObject(g_hBmp);
    if (g_hMemDC) DeleteDC(g_hMemDC);
    if (g_brushWall) DeleteObject(g_brushWall);
    if (g_brushFloor) DeleteObject(g_brushFloor);
    if (g_brushFog) DeleteObject(g_brushFog);
    if (g_brushPlayer) DeleteObject(g_brushPlayer);
    if (g_brushHUD) DeleteObject(g_brushHUD);
    if (g_brushEnemy) DeleteObject(g_brushEnemy);
    if (g_brushBoss) DeleteObject(g_brushBoss);
    if (g_brushGold) DeleteObject(g_brushGold);
    if (g_brushShop) DeleteObject(g_brushShop);
    if (g_penGrid) DeleteObject(g_penGrid);
    if (g_penBoss) DeleteObject(g_penBoss);
}

static void ui_render(HWND hwnd, GameState* gs) {
    if (!g_hMemDC || !g_hBmp) return;

    HBRUSH hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushWall);
    HPEN hOldPen = (HPEN)SelectObject(g_hMemDC, g_penGrid);
    Rectangle(g_hMemDC, 0, 0, WIN_W, WIN_H);
    SelectObject(g_hMemDC, hOldBrush);
    SelectObject(g_hMemDC, hOldPen);

    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {
            int left = x * CELL_SIZE;
            int top = y * CELL_SIZE;
            int right = left + CELL_SIZE;
            int bottom = top + CELL_SIZE;
            FogState fog = gs->fog[y][x];

            if (fog == FOG_HIDDEN) {
                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushFog);
                FillRect(g_hMemDC, &(RECT){left, top, right, bottom}, g_brushFog);
                SelectObject(g_hMemDC, hOldBrush);
                continue;
            }

            if (gs->map[y][x] == TILE_WALL) {
                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushWall);
                FillRect(g_hMemDC, &(RECT){left, top, right, bottom}, g_brushWall);
                SelectObject(g_hMemDC, hOldBrush);
            } else if (gs->map[y][x] == TILE_FLOOR) {
                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushFloor);
                FillRect(g_hMemDC, &(RECT){left, top, right, bottom}, g_brushFloor);
                SelectObject(g_hMemDC, hOldBrush);
            } else if (gs->map[y][x] == TILE_STAIRS) {
                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushFloor);
                FillRect(g_hMemDC, &(RECT){left, top, right, bottom}, g_brushFloor);
                SelectObject(g_hMemDC, hOldBrush);

                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushGold);
                Ellipse(g_hMemDC, left+4, top+4, right-4, bottom-4);
                SelectObject(g_hMemDC, hOldBrush);
            } else if (gs->map[y][x] == TILE_SHOP) {
                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushFloor);
                FillRect(g_hMemDC, &(RECT){left, top, right, bottom}, g_brushFloor);
                SelectObject(g_hMemDC, hOldBrush);

                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushShop);
                Rectangle(g_hMemDC, left+4, top+4, right-4, bottom-4);
                SelectObject(g_hMemDC, hOldBrush);
            }

            if (fog == FOG_EXPLORED) {
                HBRUSH hDark = CreateSolidBrush(RGB(30, 30, 35));
                hOldBrush = (HBRUSH)SelectObject(g_hMemDC, hDark);
                FillRect(g_hMemDC, &(RECT){left, top, right, bottom}, hDark);
                SelectObject(g_hMemDC, hOldBrush);
                DeleteObject(hDark);
            }
        }
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (gs->enemies[i].active &&
            gs->fog[gs->enemies[i].y][gs->enemies[i].x] != FOG_HIDDEN) {
            int ex = gs->enemies[i].x * CELL_SIZE;
            int ey = gs->enemies[i].y * CELL_SIZE;

            hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushEnemy);
            HPEN hEnemyPen = CreatePen(PS_SOLID, 1, RGB(150, 0, 0));
            HPEN hOldEnemyPen = (HPEN)SelectObject(g_hMemDC, hEnemyPen);
            Ellipse(g_hMemDC, ex+4, ey+4, ex+CELL_SIZE-4, ey+CELL_SIZE-4);
            SelectObject(g_hMemDC, hOldEnemyPen);
            DeleteObject(hEnemyPen);
            SelectObject(g_hMemDC, hOldBrush);
        }
    }

    if (gs->is_boss_floor && gs->boss.active &&
        gs->fog[gs->boss.y][gs->boss.x] != FOG_HIDDEN) {
        int bx = gs->boss.x * CELL_SIZE;
        int by = gs->boss.y * CELL_SIZE;

        hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushBoss);
        hOldPen = (HPEN)SelectObject(g_hMemDC, g_penBoss);
        Rectangle(g_hMemDC, bx+2, by+2, bx+CELL_SIZE-2, by+CELL_SIZE-2);
        SelectObject(g_hMemDC, hOldPen);
        SelectObject(g_hMemDC, hOldBrush);

        int hp_w = (CELL_SIZE - 4) * gs->boss_hp / gs->boss_max_hp;
        if (hp_w > 0) {
            HBRUSH hHP = CreateSolidBrush(RGB(0, 255, 0));
            hOldBrush = (HBRUSH)SelectObject(g_hMemDC, hHP);
            Rectangle(g_hMemDC, bx+2, by+CELL_SIZE-6, bx+2+hp_w, by+CELL_SIZE-3);
            SelectObject(g_hMemDC, hOldBrush);
            DeleteObject(hHP);
        }
    }

    HBRUSH hArrow = CreateSolidBrush(RGB(255, 255, 0));
    hOldBrush = (HBRUSH)SelectObject(g_hMemDC, hArrow);
    for (int i = 0; i < MAX_ARROWS; ++i) {
        if (gs->arrows[i].active) {
            int ax = gs->arrows[i].x * CELL_SIZE + CELL_SIZE/2;
            int ay = gs->arrows[i].y * CELL_SIZE + CELL_SIZE/2;
            Ellipse(g_hMemDC, ax-3, ay-3, ax+3, ay+3);
        }
    }
    SelectObject(g_hMemDC, hOldBrush);
    DeleteObject(hArrow);

    hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushPlayer);
    HPEN hPlayerPen = CreatePen(PS_SOLID, 1, RGB(0, 150, 50));
    HPEN hOldPlayerPen = (HPEN)SelectObject(g_hMemDC, hPlayerPen);
    Ellipse(g_hMemDC,
        gs->px*CELL_SIZE+2, gs->py*CELL_SIZE+2,
        (gs->px+1)*CELL_SIZE-2, (gs->py+1)*CELL_SIZE-2);
    SelectObject(g_hMemDC, hOldPlayerPen);
    DeleteObject(hPlayerPen);
    SelectObject(g_hMemDC, hOldBrush);

    int hud_y = MAP_H * CELL_SIZE;
    hOldBrush = (HBRUSH)SelectObject(g_hMemDC, g_brushHUD);
    FillRect(g_hMemDC, &(RECT){0, hud_y, WIN_W, WIN_H}, g_brushHUD);
    SelectObject(g_hMemDC, hOldBrush);

    SetBkMode(g_hMemDC, TRANSPARENT);
    SetTextColor(g_hMemDC, RGB(255, 255, 255));

    HFONT hFont = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(g_hMemDC, hFont);

    char buf[128];
    if (gs->is_boss_floor && gs->boss.active) {
        snprintf(buf, sizeof(buf), "HP: %d/%d | Gold: %d | Floor: %d | BOSS HP: %d/%d",
            gs->hp, gs->max_hp, gs->gold, gs->level, gs->boss_hp, gs->boss_max_hp);
    } else {
        snprintf(buf, sizeof(buf), "HP: %d/%d | Gold: %d | Floor: %d | SPD:%d FIR:%d FOV:%d",
            gs->hp, gs->max_hp, gs->gold, gs->level,
            (2 - gs->move_cd_max), 15 - gs->shoot_cd_max, gs->fov_radius);
    }
    TextOutA(g_hMemDC, 20, hud_y + 20, buf, (int)strlen(buf));

    SelectObject(g_hMemDC, hOldFont);
    DeleteObject(hFont);

    if (gs->shop_open) {
        HBRUSH hDim = CreateSolidBrush(RGB(0,0,0));
        hOldBrush = (HBRUSH)SelectObject(g_hMemDC, hDim);
        Rectangle(g_hMemDC, 0, 0, WIN_W, WIN_H);
        SelectObject(g_hMemDC, hOldBrush);
        DeleteObject(hDim);

        int mx = WIN_W/2 - 160, my = WIN_H/2 - 130, mw = 320, mh = 260;
        HBRUSH hShopBg = CreateSolidBrush(RGB(40, 30, 60));
        HPEN hShopPen = CreatePen(PS_SOLID, 2, RGB(150, 100, 255));

        hOldBrush = (HBRUSH)SelectObject(g_hMemDC, hShopBg);
        HPEN hOldShopPen = (HPEN)SelectObject(g_hMemDC, hShopPen);
        Rectangle(g_hMemDC, mx, my, mx+mw, my+mh);
        SelectObject(g_hMemDC, hOldShopPen);
        SelectObject(g_hMemDC, hOldBrush);
        DeleteObject(hShopPen);
        DeleteObject(hShopBg);

        SetBkMode(g_hMemDC, TRANSPARENT);
        SetTextColor(g_hMemDC, RGB(200, 150, 255));

        HFONT hTitle = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        hOldFont = (HFONT)SelectObject(g_hMemDC, hTitle);
        TextOutA(g_hMemDC, mx + 70, my + 15, "MAGIC SHOP", 10);
        SelectObject(g_hMemDC, hOldFont);
        DeleteObject(hTitle);

        HFONT hItem = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
        hOldFont = (HFONT)SelectObject(g_hMemDC, hItem);

        const char* items[] = { "Fire Rate", "Move Speed", "FOV" };
        int costs[] = { gs->cost_fire_rate, gs->cost_move_speed, gs->cost_fov };
        for (int i = 0; i < 3; ++i) {
            int iy = my + 60 + i * 45;
            if (gs->shop_cursor == i) {
                SetTextColor(g_hMemDC, RGB(255, 255, 100));
                TextOutA(g_hMemDC, mx + 20, iy, ">", 1);
            } else {
                SetTextColor(g_hMemDC, RGB(200, 200, 200));
            }
            char line[64];
            snprintf(line, sizeof(line), "%s - %d G", items[i], costs[i]);
            TextOutA(g_hMemDC, mx + 40, iy, line, (int)strlen(line));
        }
        SelectObject(g_hMemDC, hOldFont);
        DeleteObject(hItem);

        SetTextColor(g_hMemDC, RGB(150, 150, 150));
        TextOutA(g_hMemDC, mx + 40, my + mh - 30,
            "UP/DOWN: Select | ENTER: Buy | Q: Close", 41);
    }

    if (gs->game_over) {
        SetBkMode(g_hMemDC, OPAQUE);
        SetBkColor(g_hMemDC, RGB(0, 0, 0));
        SetTextColor(g_hMemDC, RGB(255, 0, 0));

        HFONT hBig = CreateFontA(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        hOldFont = (HFONT)SelectObject(g_hMemDC, hBig);
        TextOutA(g_hMemDC, WIN_W/2 - 120, WIN_H/2 - 20, "GAME OVER", 9);
        SelectObject(g_hMemDC, hOldFont);
        DeleteObject(hBig);

        SetTextColor(g_hMemDC, RGB(255, 255, 255));
        HFONT hSmall = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        hOldFont = (HFONT)SelectObject(g_hMemDC, hSmall);
        TextOutA(g_hMemDC, WIN_W/2 - 90, WIN_H/2 + 30, "Press R to Restart", 18);
        SelectObject(g_hMemDC, hOldFont);
        DeleteObject(hSmall);
    }

    HDC hDC = GetDC(hwnd);
    BitBlt(hDC, 0, 0, WIN_W, WIN_H, g_hMemDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hDC);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GameState* gs = (GameState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (!gs) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg) {
        case WM_KEYDOWN:
            if (wParam < 256) gs->keys[wParam] = true;
            break;
        case WM_KEYUP:
            if (wParam < 256) gs->keys[wParam] = false;
            break;
        case WM_CLOSE:
            gs->running = false;
            break;
        case WM_DESTROY:
            gs->running = false;
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int ui_run(GameState* gs) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"RogueliteWinAPI";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc)) return -1;

    HWND hwnd = CreateWindowW(L"RogueliteWinAPI",
        L"2D Roguelite - WASD: Move, SPACE: Shoot",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WIN_W + 16, WIN_H + 39,
        NULL, NULL, wc.hInstance, NULL);

    if (!hwnd) {
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return -1;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gs);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    ui_init_resources(hwnd);

    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    double accumulator = 0.0;
    gs->running = true;

    MSG msg;
    while (gs->running) {
        QueryPerformanceCounter(&now);
        double dt = (double)(now.QuadPart - last.QuadPart) / (double)freq.QuadPart;
        last = now;
        accumulator += dt;

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                gs->running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!gs->running) break;

        while (accumulator >= FIXED_DT) {
            core_handle_input(gs);
            core_update(gs, (float)FIXED_DT);
            accumulator -= FIXED_DT;
        }

        ui_render(hwnd, gs);
    }

    ui_cleanup();
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}