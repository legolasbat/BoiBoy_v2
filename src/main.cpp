#include <SDL.h>
#include <SDL_syswm.h>

#include <commdlg.h>
#include <chrono>

#include "Utils.h"
#include "Memory.h"

SDL_Window* win;
SDL_Renderer* renderer;
SDL_Texture* texture;
SDL_Joystick* gameController;

SDL_Event event;

bool running = true;
bool boiRunning = false;

Cartridge* cart;
BoiBoy* boiBoy;

int cycles = 0;
std::chrono::steady_clock::time_point start;

void handleInput();
HMENU CreateMenuBar();

int main(int argc, char*argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer);
    if (win == nullptr) {
        printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (renderer == nullptr) {
        printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        if (win != nullptr) {
            SDL_DestroyWindow(win);
        }
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_SetWindowSize(win, WIDTH * 3, HEIGHT * 3);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
    SDL_SetWindowResizable(win, SDL_TRUE);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (texture == nullptr) {
        if (win == nullptr) {
            printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
            if (renderer != nullptr) {
                SDL_DestroyRenderer(renderer);
            }
            if (win != nullptr) {
                SDL_DestroyWindow(win);
            }
            SDL_Quit();
            return EXIT_FAILURE;
        }
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    SDL_SetWindowTitle(win, "BoiBoy");
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(win, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    HMENU hMenuBar = CreateMenuBar();
    SetMenu(hwnd, hMenuBar);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    //Check for joysticks
    if (SDL_NumJoysticks() < 1)
    {
        printf("Warning: No joysticks connected!\n");
    }
    else
    {
        //Load joystick
        gameController = SDL_JoystickOpen(0);
        if (gameController == NULL)
        {
            printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
        }
    }

    int frames = 0;

    while (running)
    {
        if (boiRunning) {
            cycles += boiBoy->Clock();
        }
        else {
            // Events management
            handleInput();

            // Graphics
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        if (boiRunning && cycles >= 17556 /*boi.ppu.frameComplete && nextFrame*/) {

            //while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < 16.67) {
            //
            //}
            frames++;
            //std::cout << frames << std::endl;

            // Events management
            handleInput();

            // Graphics
            SDL_RenderClear(renderer);
            uint8_t* frame = boiBoy->ppu.GetScreen();
            SDL_UpdateTexture(texture, NULL, frame, boiBoy->ppu.GetPitch());
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            //boi.ppu.frameComplete = false;
            cycles -= 17556;

            start = std::chrono::high_resolution_clock::now();
        }
        
    }

    //Close game controller
    SDL_JoystickClose(gameController);

    //Destroy window
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

void handleInput() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_QUIT: {
            // handling of close button
            running = false;
            break;
        }
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
            if (!boiRunning) {
                continue;
            }
            switch (event.key.keysym.sym) {
                // B Button (PS: cross / XBOX: A)
            case SDLK_n: {
                boiBoy->PressButton(JOYPAD_B);
                break;
            }
                // A Button (PS: circle / XBOX: B)
            case SDLK_m: {
                boiBoy->PressButton(JOYPAD_A);
                break;
            }
                // Select Button
            case SDLK_q: {
                boiBoy->PressButton(JOYPAD_SELECT);
                break;
            }
                // Start Button
            case SDLK_e:
            case SDLK_RETURN: {
                boiBoy->PressButton(JOYPAD_START);
                break;
            }
                // Up Button
            case SDLK_w: {
                boiBoy->PressButton(JOYPAD_UP);
                break;
            }
                // Down Button
            case SDLK_s: {
                boiBoy->PressButton(JOYPAD_DOWN);
                break;
            }
                // Left Button
            case SDLK_a: {
                boiBoy->PressButton(JOYPAD_LEFT);
                break;
            }
                // Right Button
            case SDLK_d: {
                boiBoy->PressButton(JOYPAD_RIGHT);
                break;
            }
            }
            break;
        }
        case SDL_KEYUP: {
            if (!boiRunning) {
                continue;
            }
            switch (event.key.keysym.sym) {
                // B Button (PS: cross / XBOX: A)
            case SDLK_n: {
                boiBoy->ReleaseButton(JOYPAD_B);
                break;
            }
                // A Button (PS: circle / XBOX: B)
            case SDLK_m: {
                boiBoy->ReleaseButton(JOYPAD_A);
                break;
            }
                // Select Button
            case SDLK_q: {
                boiBoy->ReleaseButton(JOYPAD_SELECT);
                break;
            }
                // Start Button
            case SDLK_e:
            case SDLK_RETURN: {
                boiBoy->ReleaseButton(JOYPAD_START);
                break;
            }
                // Up Button
            case SDLK_w: {
                boiBoy->ReleaseButton(JOYPAD_UP);
                break;
            }
                // Down Button
            case SDLK_s: {
                boiBoy->ReleaseButton(JOYPAD_DOWN);
                break;
            }
                // Left Button
            case SDLK_a: {
                boiBoy->ReleaseButton(JOYPAD_LEFT);
                break;
            }
                // Right Button
            case SDLK_d: {
                boiBoy->ReleaseButton(JOYPAD_RIGHT);
                break;
            }
            }
            break;
        }
        case SDL_JOYBUTTONDOWN: {
            if (!boiRunning) {
                continue;
            }
            switch (event.jbutton.button) {
                // B Button (PS: cross / XBOX: A)
            case 0: {
                boiBoy->PressButton(JOYPAD_B);
                break;
            }
                // A Button (PS: circle / XBOX: B)
            case 1: {
                boiBoy->PressButton(JOYPAD_A);
                break;
            }
                // Y Button (PS: square / XBOX: X)
            case 2: {
                break;
            }
                // X Button (PS: triangle / XBOX: Y)
            case 3: {
                break;
            }
                // Select Button
            case 4: {
                boiBoy->PressButton(JOYPAD_SELECT);
                break;
            }
                // Start Button
            case 6: {
                boiBoy->PressButton(JOYPAD_START);
                break;
            }
                // L Button (PS: L1 / XBOX: LB)
            case 9: {
                break;
            }
                // R Button (PS: R1 / XBOX: RB)
            case 10: {
                break;
            }
                // Up Button
            case 11: {
                boiBoy->PressButton(JOYPAD_UP);
                break;
            }
                // Down Button
            case 12: {
                boiBoy->PressButton(JOYPAD_DOWN);
                break;
            }
                // Left Button
            case 13: {
                boiBoy->PressButton(JOYPAD_LEFT);
                break;
            }
                // Right Button
            case 14: {
                boiBoy->PressButton(JOYPAD_RIGHT);
                break;
            }
            }
            break;
        }
        case SDL_JOYBUTTONUP: {
            if (!boiRunning) {
                continue;
            }
            switch (event.jbutton.button) {
                // B Button (PS: cross / XBOX: A)
            case 0: {
                boiBoy->ReleaseButton(JOYPAD_B);
                break;
            }
                // A Button (PS: circle / XBOX: B)
            case 1: {
                boiBoy->ReleaseButton(JOYPAD_A);
                break;
            }
                // Y Button (PS: square / XBOX: X)
            case 2: {
                break;
            }
                // X Button (PS: triangle / XBOX: Y)
            case 3: {
                break;
            }
                // Select Button
            case 4: {
                boiBoy->ReleaseButton(JOYPAD_SELECT);
                break;
            }
                // Start Button
            case 6: {
                boiBoy->ReleaseButton(JOYPAD_START);
                break;
            }
                // L Button (PS: L1 / XBOX: LB)
            case 9: {
                break;
            }
                // R Button (PS: R1 / XBOX: RB)
            case 10: {
                break;
            }
                // Up Button
            case 11: {
                boiBoy->ReleaseButton(JOYPAD_UP);
                break;
            }
                // Down Button
            case 12: {
                boiBoy->ReleaseButton(JOYPAD_DOWN);
                break;
            }
                // Left Button
            case 13: {
                boiBoy->ReleaseButton(JOYPAD_LEFT);
                break;
            }
                // Right Button
            case 14: {
                boiBoy->ReleaseButton(JOYPAD_RIGHT);
                break;
            }
            }
            break;
        }
        case SDL_JOYDEVICEREMOVED: {
            SDL_JoystickClose(gameController);
            break;
        }
        case SDL_JOYDEVICEADDED: {
            //Check for joysticks
            if (SDL_NumJoysticks() < 1)
            {
                printf("Warning: No joysticks connected!\n");
            }
            else
            {
                //Load joystick
                gameController = SDL_JoystickOpen(0);
                if (gameController == NULL)
                {
                    printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
                }
            }
            break;
        }
        case SDL_SYSWMEVENT: {
            if (event.syswm.msg->msg.win.msg == WM_COMMAND) {
                // Exit
                if (LOWORD(event.syswm.msg->msg.win.wParam) == 1) {
                    running = false;
                }
                // Load ROM
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 2) {
                    char szFile[256];
                    OPENFILENAME ofn;

                    ZeroMemory(&ofn, sizeof(ofn));

                    ofn.lStructSize = sizeof(ofn);
                    ofn.lpstrFile = (LPWSTR)szFile;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = L"GB\0*.gb\0\0";
                    ofn.lpstrTitle = L"ROM Selection";
                    ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (GetOpenFileName(&ofn) == TRUE) {
                        // TODO: Load ROM
                        if (cart != nullptr && boiBoy != nullptr) {
                            boiBoy->~BoiBoy();
                        }
                        cart = new Cartridge(ofn);
                        boiBoy = new BoiBoy();
                        boiBoy->InsertCart(cart);
                        boiRunning = true;
                        start = std::chrono::high_resolution_clock::now();
                        cycles = 0;
                    }
                }
                // Toggle audio channels
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 3) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 3, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 3, MF_UNCHECKED);
                        boiBoy->spu.ToggleChannel(1);
                    }
                    else {
                        CheckMenuItem(menu, 3, MF_CHECKED);
                        boiBoy->spu.ToggleChannel(1);
                    }
                }
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 4) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 4, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 4, MF_UNCHECKED);
                        boiBoy->spu.ToggleChannel(2);
                    }
                    else {
                        CheckMenuItem(menu, 4, MF_CHECKED);
                        boiBoy->spu.ToggleChannel(2);
                    }
                }
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 5) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 5, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 5, MF_UNCHECKED);
                        boiBoy->spu.ToggleChannel(3);
                    }
                    else {
                        CheckMenuItem(menu, 5, MF_CHECKED);
                        boiBoy->spu.ToggleChannel(3);
                    }
                }
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 6) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 6, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 6, MF_UNCHECKED);
                        boiBoy->spu.ToggleChannel(4);
                    }
                    else {
                        CheckMenuItem(menu, 6, MF_CHECKED);
                        boiBoy->spu.ToggleChannel(4);
                    }
                }
                // Toggle layers
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 7) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 7, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 7, MF_UNCHECKED);
                        boiBoy->ppu.ToggleLayer(0);
                    }
                    else {
                        CheckMenuItem(menu, 7, MF_CHECKED);
                        boiBoy->ppu.ToggleLayer(0);
                    }
                }
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 8) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 8, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 8, MF_UNCHECKED);
                        boiBoy->ppu.ToggleLayer(1);
                    }
                    else {
                        CheckMenuItem(menu, 8, MF_CHECKED);
                        boiBoy->ppu.ToggleLayer(1);
                    }
                }
                else if (LOWORD(event.syswm.msg->msg.win.wParam) == 9) {
                    if (!boiRunning) {
                        continue;
                    }

                    SDL_SysWMinfo wmInfo;
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(win, &wmInfo);
                    HWND hwnd = wmInfo.info.win.window;
                    HMENU menu = GetMenu(wmInfo.info.win.window);

                    UINT state = GetMenuState(menu, 9, MF_BYCOMMAND);

                    if (state == MF_CHECKED) {
                        CheckMenuItem(menu, 9, MF_UNCHECKED);
                        boiBoy->ppu.ToggleLayer(2);
                    }
                    else {
                        CheckMenuItem(menu, 9, MF_CHECKED);
                        boiBoy->ppu.ToggleLayer(2);
                    }
                }
            }

            break;
        }
        }
    }
}


HMENU CreateMenuBar() {
    HMENU hMenuBar = CreateMenu();

    // File menu
    HMENU hFile = CreateMenu();
    std::wstring sFile = L"File";
    LPCWSTR shFile = sFile.c_str();
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, shFile);

    std::wstring sLoad = L"Load ROM";
    LPCWSTR shLoad = sLoad.c_str();
    AppendMenu(hFile, MF_STRING, 2, shLoad);

    std::wstring sExit = L"Exit";
    LPCWSTR shExit = sExit.c_str();
    AppendMenu(hFile, MF_STRING, 1, shExit);

    // Debug menu
    HMENU hDebug = CreateMenu();
    std::wstring sDebug = L"Debug";
    LPCWSTR shDebug = sDebug.c_str();
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hDebug, shDebug);

    // Debug Menu - Audio
    HMENU hDAudio = CreateMenu();
    std::wstring sDAudio = L"Toggle Audio";
    LPCWSTR shDAudio = sDAudio.c_str();
    AppendMenu(hDebug, MF_POPUP, (UINT_PTR)hDAudio, shDAudio);

    // Audio Channel 1
    std::wstring sAC1 = L"Channel 1";
    LPCWSTR shAC1 = sAC1.c_str();
    AppendMenu(hDAudio, MF_STRING | MF_CHECKED, 3, shAC1);

    // Audio Channel 2
    std::wstring sAC2 = L"Channel 2";
    LPCWSTR shAC2 = sAC2.c_str();
    AppendMenu(hDAudio, MF_STRING | MF_CHECKED, 4, shAC2);

    // Audio Channel 3
    std::wstring sAC3 = L"Channel 3";
    LPCWSTR shAC3 = sAC3.c_str();
    AppendMenu(hDAudio, MF_STRING | MF_CHECKED, 5, shAC3);

    // Audio Channel 4
    std::wstring sAC4 = L"Channel 4";
    LPCWSTR shAC4 = sAC4.c_str();
    AppendMenu(hDAudio, MF_STRING | MF_CHECKED, 6, shAC4);

    // Debug Menu - Layers
    HMENU hDLayers = CreateMenu();
    std::wstring sDLayers = L"Toggle Layers";
    LPCWSTR shDLayers = sDLayers.c_str();
    AppendMenu(hDebug, MF_POPUP, (UINT_PTR)hDLayers, shDLayers);

    std::wstring sBG = L"Background";
    LPCWSTR shBG = sBG.c_str();
    AppendMenu(hDLayers, MF_STRING | MF_CHECKED, 7, shBG);

    std::wstring sWN = L"Window";
    LPCWSTR shWN = sWN.c_str();
    AppendMenu(hDLayers, MF_STRING | MF_CHECKED, 8, shWN);

    std::wstring sSP = L"Sprites";
    LPCWSTR shSP = sSP.c_str();
    AppendMenu(hDLayers, MF_STRING | MF_CHECKED, 9, shSP);

    return hMenuBar;
}
