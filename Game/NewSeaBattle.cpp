/* File NewSeaBattle.cpp
Implementation of a special mode of the Sea Battle game
Done by Demchenko Yehor (group Computer mathematics 2)
Date 30.11.2024
Full implementation of the game Sea Battle
*/

// g++ -std=c++17 NewSeaBattle.cpp -o NewSeaBattle.exe -lgdiplus -lcomdlg32 -lgdi32


#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <windowsx.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <set>
#include <queue>
#include <gdiplus.h>
#include <climits>
#pragma comment (lib, "Gdiplus.lib")

ULONG_PTR gdiplusToken;

constexpr int BOARD_SIZE = 10;
constexpr int CELL_SIZE = 30;

constexpr char EMPTY = '-';
constexpr char SHIP = 'S';
constexpr char MISS = 'O';
constexpr char HIT = 'X';

std::vector<std::vector<char>> playerBoard, botBoard;
std::set<std::pair<int, int>> possiblePositions;

int playerX, playerY, botX, botY;
int playerStartX, playerStartY, botStartX, botStartY;
int lastBotShotX = -1, lastBotShotY = -1;
int lastDistance = -1;
int distanceX = 0;
int distanceY = 0;
int lastDistancePlayer1 = -1;
int lastDistancePlayer2 = -1;

bool playerShipPlaced = false;
bool isPlayerHit = false;
bool isBotHit = false;
bool playerMovedAfterBotShot = true;
bool player1Moved = false;
bool player2Moved = false;
bool player1MovedOnce = true;
bool player2MovedOnce = false;
bool player1CanMoveShip = false;
bool player2CanMoveShip = false;
bool playerCanMoveShip = false;
bool gameStarted = false;
bool isHardMode = false;
bool isTwoPlayersMode = false;
bool isPlayer1Turn = true;
bool playerTurnConfirmed = false;
bool hideAllShips = false;
bool player1ShipPlaced = false;
bool player2ShipPlaced = false;

HWND newGame = nullptr;
HWND mainMenu = nullptr;
HWND randomBot = nullptr;
HWND hardBot = nullptr;
HWND twoPlayers = nullptr;
HWND exit_ = nullptr;

void initializeBoard(std::vector<std::vector<char>>& board) {
    board.assign(BOARD_SIZE, std::vector<char>(BOARD_SIZE, EMPTY));
}

void drawGrid(HDC hdc, const std::vector<std::vector<char>>& board, int startX, int startY, bool hideShips) {
    SetBkMode(hdc, TRANSPARENT);
    for (int j = 0; j < BOARD_SIZE; ++j) {
        char num[3];
        if (j == 9) {
            strcpy(num, "10");
            TextOutA(hdc, startX + j * CELL_SIZE + CELL_SIZE / 2 - 10, startY - CELL_SIZE / 2, num, 2);
        } else {
            num[0] = static_cast<char>('1' + j);
            num[1] = '\0';
            TextOutA(hdc, startX + j * CELL_SIZE + CELL_SIZE / 2 - 5, startY - CELL_SIZE / 2, num, 1);
        }
    }
    for (int i = 0; i < BOARD_SIZE; ++i) {
        char letter = static_cast<char>('A' + i);
        if (startX != botStartX) {
            TextOutA(hdc, startX + 8 - CELL_SIZE / 2 - 10, startY - 3 + i * CELL_SIZE + CELL_SIZE / 2 - 5, &letter, 1);
        }
        if (startX == botStartX) {
            TextOutA(hdc, startX - 10 + BOARD_SIZE * CELL_SIZE + CELL_SIZE / 2 + 5, startY - 3 + i * CELL_SIZE + CELL_SIZE / 2 - 5, &letter, 1);
        }
    }
    if (isTwoPlayersMode && startX == botStartX) {
        hideShips = false;
    }
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            int left = startX + j * CELL_SIZE;
            int top = startY + i * CELL_SIZE;
            int right = left + CELL_SIZE;
            int bottom = top + CELL_SIZE;
            COLORREF color;
            if (isTwoPlayersMode) {
                if (hideAllShips) {
                    hideShips = true;
                }
                if ((isPlayer1Turn && startX == botStartX) || (!isPlayer1Turn && startX == playerStartX)) {
                    hideShips = true;
                }
                if (board[i][j] == HIT) {
                    hideShips = false;
                } else if (isPlayer1Turn && startX == playerStartX && player1CanMoveShip &&
                    (std::abs(i - playerX) + std::abs(j - playerY) == 1) && board[i][j] == EMPTY && board[playerX][playerY] != HIT) {
                    color = RGB(102, 255, 153);
                } else if (!isPlayer1Turn && startX == botStartX && player2CanMoveShip &&
                           (std::abs(i - botX) + std::abs(j - botY) == 1) && board[i][j] == EMPTY && board[botX][botY] != HIT) {
                    color = RGB(102, 255, 153);
                } else if (board[i][j] == SHIP && !hideShips) {
                    color = RGB(255, 204, 0);
                } else {
                    switch (board[i][j]) {
                        case EMPTY:
                            color = RGB(230, 242, 255);
                        break;
                        case MISS:
                            color = RGB(230, 242, 255);
                        break;
                        case HIT:
                            color = RGB(255, 102, 102);
                        break;
                        default:
                            color = RGB(230, 242, 255);
                        break;
                    }
                }
            } else {
                if (startX == playerStartX && playerCanMoveShip && !isPlayerHit &&
                    ((std::abs(i - playerX) + std::abs(j - playerY)) == 1) && board[i][j] == EMPTY) {
                    color = RGB(102, 255, 153);
                } else if (board[i][j] == SHIP && !hideShips) {
                    color = RGB(255, 204, 0);
                } else {
                    switch (board[i][j]) {
                        case EMPTY:
                            color = RGB(230, 242, 255);
                        break;
                        case MISS:
                            color = RGB(230, 242, 255);
                        break;
                        case HIT:
                            color = RGB(255, 102, 102);
                        break;
                        default:
                            color = RGB(230, 242, 255);
                        break;
                    }
                }
            }
            HBRUSH brush = CreateSolidBrush(color);
            RECT cellRect = {left, top, right, bottom};
            FillRect(hdc, &cellRect, brush);
            DeleteObject(brush);
            if (board[i][j] == MISS) {
                int radius = CELL_SIZE / 4;
                int centerX = (left + right) / 2;
                int centerY = (top + bottom) / 2;
                HBRUSH missBrush = CreateSolidBrush(RGB(26, 26, 255));
                SelectObject(hdc, missBrush);
                Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
                DeleteObject(missBrush);
            }
            if (board[i][j] == HIT) {
                HBRUSH hitBrush = CreateSolidBrush(RGB(255, 102, 102));
                RECT hitRect = {left, top, right, bottom};
                FillRect(hdc, &hitRect, hitBrush);
                DeleteObject(hitBrush);
                HPEN hitPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                SelectObject(hdc, hitPen);
                MoveToEx(hdc, left, top, nullptr);
                LineTo(hdc, right, bottom);
                MoveToEx(hdc, right, top, nullptr);
                LineTo(hdc, left, bottom);
                DeleteObject(hitPen);
            }
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdc, pen);
            MoveToEx(hdc, left, top, nullptr);
            LineTo(hdc, right, top);
            LineTo(hdc, right, bottom);
            LineTo(hdc, left, bottom);
            LineTo(hdc, left, top);
            DeleteObject(pen);
        }
    }
}

int calculateManhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

void handlePlayerPlacement(int x, int y) {
    if (!playerShipPlaced && x >= playerStartX && x < playerStartX + BOARD_SIZE * CELL_SIZE &&
        y >= playerStartY && y < playerStartY + BOARD_SIZE * CELL_SIZE) {
        int col = (x - playerStartX) / CELL_SIZE;
        int row = (y - playerStartY) / CELL_SIZE;
        if (playerBoard[row][col] == EMPTY) {
            playerBoard[row][col] = SHIP;
            playerX = row;
            playerY = col;
            playerShipPlaced = true;
            InvalidateRect(nullptr, nullptr, TRUE);
        }
        }
}

void placeBotShip(std::vector<std::vector<char>>& board) {
    do {
        botX = rand() % BOARD_SIZE;
        botY = rand() % BOARD_SIZE;
    } while (board[botX][botY] != EMPTY);
    board[botX][botY] = SHIP;
}

void moveBotShipHard() {
    std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    std::pair<int, int> safestCell = {botX, botY};
    int minRisk = INT_MAX;
    auto calculateRisk = [&](int x, int y) {
        int risk = 0;
        for (auto& dir : directions) {
            int newX = x + dir.first;
            int newY = y + dir.second;
            if (newX >= 0 && newX < BOARD_SIZE && newY >= 0 && newY < BOARD_SIZE) {
                if (playerBoard[newX][newY] == HIT || playerBoard[newX][newY] == MISS) {
                    risk++;
                }
            }
        }
        return risk;
    };
    for (auto& dir : directions) {
        int newX = botX + dir.first;
        int newY = botY + dir.second;
        if (newX >= 0 && newX < BOARD_SIZE && newY >= 0 && newY < BOARD_SIZE && botBoard[newX][newY] == EMPTY) {
            int risk = calculateRisk(newX, newY);
            if (risk < minRisk) {
                minRisk = risk;
                safestCell = {newX, newY};
            }
        }
    }
    botBoard[botX][botY] = EMPTY;
    botX = safestCell.first;
    botY = safestCell.second;
    botBoard[botX][botY] = SHIP;
}

void moveBotShip() {
    if (isHardMode) {
        moveBotShipHard();
    } else {
        std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        std::vector<std::pair<int, int>> availableCells;
        availableCells.emplace_back(botX, botY);
        for (auto& dir : directions) {
            int newX = botX + dir.first;
            int newY = botY + dir.second;
            if (newX >= 0 && newX < BOARD_SIZE && newY >= 0 && newY < BOARD_SIZE && botBoard[newX][newY] == EMPTY) {
                availableCells.emplace_back(newX, newY);
            }
        }
        if (!availableCells.empty()) {
            std::pair<int, int> newCell = availableCells[rand() % availableCells.size()];
            botBoard[botX][botY] = EMPTY;
            botX = newCell.first;
            botY = newCell.second;
            botBoard[botX][botY] = SHIP;
        }
    }
}

void startNewGame() {
    initializeBoard(playerBoard);
    initializeBoard(botBoard);
    playerShipPlaced = false;
    isPlayerHit = false;
    isBotHit = false;
    playerMovedAfterBotShot = true;
    playerCanMoveShip = false;
    isPlayer1Turn = true;
    player1Moved = false;
    player2Moved = false;
    player1MovedOnce = true;
    player2MovedOnce = false;
    player1CanMoveShip = false;
    player1ShipPlaced = false;
    player2ShipPlaced = false;
    player2CanMoveShip = false;
    lastDistance = -1;
    lastDistancePlayer1 = -1;
    lastDistancePlayer2 = -1;
    if (!isTwoPlayersMode) {
        placeBotShip(botBoard);
    }
}

void initializeTwoPlayersGame() {
    initializeBoard(playerBoard);
    initializeBoard(botBoard);
    playerShipPlaced = false;
    isPlayerHit = false;
    isBotHit = false;
    isPlayer1Turn = true;
    playerMovedAfterBotShot = true;
    playerCanMoveShip = false;
}

void StartEndGameTimer(HWND hwnd, bool playerWon, bool isTwoPlayersMode) {
    if (isTwoPlayersMode) {
        SetTimer(hwnd, playerWon ? 3 : 4, 1200, nullptr);
    } else {
        SetTimer(hwnd, playerWon ? 1 : 2, 1200, nullptr);
    }
}

void updatePossiblePositions(int distance, int shotX, int shotY) {
    std::set<std::pair<int, int>> newPositions;
    for (int dx = -distance; dx <= distance; ++dx) {
        int dy = distance - abs(dx);
        std::vector<std::pair<int, int>> candidates = {
            {shotX + dx, shotY + dy},
            {shotX + dx, shotY - dy}
        };
        for (const auto& pos : candidates) {
            if (pos.first >= 0 && pos.first < BOARD_SIZE &&
                pos.second >= 0 && pos.second < BOARD_SIZE &&
                playerBoard[pos.first][pos.second] != HIT &&
                playerBoard[pos.first][pos.second] != MISS) {
                newPositions.insert(pos);
            }
        }
    }
    std::set<std::pair<int, int>> movablePositions;
    for (const auto& pos : newPositions) {
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (abs(dx) + abs(dy) == 1) {
                    int newX = pos.first + dx;
                    int newY = pos.second + dy;
                    if (newX >= 0 && newX < BOARD_SIZE &&
                        newY >= 0 && newY < BOARD_SIZE &&
                        playerBoard[newX][newY] != HIT &&
                        playerBoard[newX][newY] != MISS) {
                        movablePositions.insert({newX, newY});
                    }
                }
            }
        }
    }
    newPositions.insert(movablePositions.begin(), movablePositions.end());
    if (!possiblePositions.empty()) {
        std::set<std::pair<int, int>> intersection;
        for (const auto& pos : possiblePositions) {
            if (newPositions.count(pos)) {
                intersection.insert(pos);
            }
        }
        if (!intersection.empty()) {
            possiblePositions = intersection;
        } else {
            possiblePositions = newPositions;
        }
    } else {
        possiblePositions = newPositions;
    }
}

void smartBotShot(HWND hwnd) {
    int row, col;
    if (possiblePositions.empty()) {
        do {
            row = rand() % BOARD_SIZE;
            col = rand() % BOARD_SIZE;
        } while (playerBoard[row][col] == HIT || playerBoard[row][col] == MISS);
    } else {
        auto it = possiblePositions.begin();
        std::advance(it, rand() % possiblePositions.size());
        row = it->first;
        col = it->second;
        possiblePositions.erase(it);
    }
    if (playerBoard[row][col] == SHIP) {
        playerBoard[row][col] = HIT;
        isPlayerHit = true;
        StartEndGameTimer(hwnd, false, false);
    } else {
        playerBoard[row][col] = MISS;
        int distance = calculateManhattanDistance(row, col, playerX, playerY);
        updatePossiblePositions(distance, row, col);
    }
    InvalidateRect(hwnd, nullptr, TRUE);
}

void botShot(HWND hwnd) {
    if (isBotHit) return;
    if (isHardMode) {
        smartBotShot(hwnd);
    } else {
        int row, col;
        do {
            row = rand() % BOARD_SIZE;
            col = rand() % BOARD_SIZE;
        } while (playerBoard[row][col] == HIT || playerBoard[row][col] == MISS);
        if (playerBoard[row][col] == SHIP) {
            playerBoard[row][col] = HIT;
            isPlayerHit = true;
            StartEndGameTimer(hwnd, false, false);
        } else if (playerBoard[row][col] == EMPTY) {
            playerBoard[row][col] = MISS;
        }
        playerMovedAfterBotShot = false;
        InvalidateRect(nullptr, nullptr, TRUE);
    }
}

void showDistance(int distance, int startX, int startY) {
    lastDistance = distance;
    distanceX = startX;
    distanceY = startY;
    if (isPlayer1Turn) {
        lastDistancePlayer2 = distance;
    } else {
        lastDistancePlayer1 = distance;
    }
}

void handlePlayerMove(int x, int y) {
    if (playerShipPlaced && playerCanMoveShip) {
        int col = (x - playerStartX) / CELL_SIZE;
        int row = (y - playerStartY) / CELL_SIZE;
        if (row == playerX && col == playerY) {
            playerCanMoveShip = false;
            playerMovedAfterBotShot = true;
            InvalidateRect(nullptr, nullptr, TRUE);
            return;
        }
        if ((std::abs(row - playerX) + std::abs(col - playerY)) == 1 && playerBoard[row][col] == EMPTY) {
            playerBoard[playerX][playerY] = EMPTY;
            playerBoard[row][col] = SHIP;
            playerX = row;
            playerY = col;
            playerCanMoveShip = false;
            playerMovedAfterBotShot = true;
            InvalidateRect(nullptr, nullptr, TRUE);
        }
    }
}

void handlePlayerShot(HWND hwnd, int x, int y) {
    if (!playerMovedAfterBotShot) return;
    if (playerShipPlaced && x >= botStartX && x < botStartX + BOARD_SIZE * CELL_SIZE &&
        y >= botStartY && y < botStartY + BOARD_SIZE * CELL_SIZE) {
        int col = (x - botStartX) / CELL_SIZE;
        int row = (y - botStartY) / CELL_SIZE;
        if (botBoard[row][col] != HIT && botBoard[row][col] != MISS) {
            if (botBoard[row][col] == SHIP) {
                botBoard[row][col] = HIT;
                isBotHit = true;
                lastDistance = 0;
                StartEndGameTimer(hwnd, true, false);
            } else if (botBoard[row][col] == EMPTY) {
                botBoard[row][col] = MISS;
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            UpdateWindow(hwnd);
            int distance = calculateManhattanDistance(row, col, botX, botY);
            showDistance(distance, botStartX, botStartY);
            playerCanMoveShip = true;
            playerMovedAfterBotShot = false;
            moveBotShip();
            botShot(hwnd);
        }
        }
}

void handleTwoPlayersPlacement(int x, int y, bool isPlayer1) {
    std::vector<std::vector<char>>& currentBoard = isPlayer1 ? playerBoard : botBoard;
    int& currentX = isPlayer1 ? playerX : botX;
    int& currentY = isPlayer1 ? playerY : botY;
    int startX = isPlayer1 ? playerStartX : botStartX;
    int startY = isPlayer1 ? playerStartY : botStartY;
    if (x >= startX && x < startX + BOARD_SIZE * CELL_SIZE &&
        y >= startY && y < startY + BOARD_SIZE * CELL_SIZE) {
        int col = (x - startX) / CELL_SIZE;
        int row = (y - startY) / CELL_SIZE;
        if (currentBoard[row][col] == EMPTY) {
            currentBoard[row][col] = SHIP;
            currentX = row;
            currentY = col;
            playerShipPlaced = true;
            InvalidateRect(nullptr, nullptr, TRUE);
            if (!isPlayer1) {
                isPlayer1Turn = true;
                hideAllShips = true;
                InvalidateRect(nullptr, nullptr, TRUE);
                MessageBoxA(nullptr,"Both players have placed their ships. The game begins!\nPlayer 1's turn.", "Game Start", MB_OK | MB_ICONINFORMATION);
                hideAllShips = false;
                InvalidateRect(nullptr, nullptr, TRUE);
            } else {
                player1Moved = true;
                playerShipPlaced = false;
                isPlayer1Turn = false;
            }
        }
    }
}

void handleTwoPlayersMove(int x, int y, bool isPlayer1) {
    std::vector<std::vector<char>>& currentBoard = isPlayer1 ? playerBoard : botBoard;
    int& currentX = isPlayer1 ? playerX : botX;
    int& currentY = isPlayer1 ? playerY : botY;
    int startX = isPlayer1 ? playerStartX : botStartX;
    int startY = isPlayer1 ? playerStartY : botStartY;
    if ((isPlayer1 && player1MovedOnce) || (!isPlayer1 && player2MovedOnce)) return;
    if (x >= startX && x < startX + BOARD_SIZE * CELL_SIZE &&
        y >= startY && y < startY + BOARD_SIZE * CELL_SIZE) {
        int col = (x - startX) / CELL_SIZE;
        int row = (y - startY) / CELL_SIZE;
        if (row == currentX && col == currentY) {
            if (isPlayer1) {
                player1Moved = true;
                player1MovedOnce = true;
                player1CanMoveShip = false;
                player2CanMoveShip = true;
            } else {
                player2Moved = true;
                player2MovedOnce = true;
                player2CanMoveShip = false;
                player1CanMoveShip = true;
            }
            InvalidateRect(nullptr, nullptr, TRUE);
            return;
        }
        if ((std::abs(row - currentX) + std::abs(col - currentY)) == 1 && currentBoard[row][col] == EMPTY) {
            currentBoard[currentX][currentY] = EMPTY;
            currentBoard[row][col] = SHIP;
            currentX = row;
            currentY = col;
            if (isPlayer1) {
                player1Moved = true;
                player1MovedOnce = true;
                player1CanMoveShip = false;
                player2CanMoveShip = true;
            } else {
                player2Moved = true;
                player2MovedOnce = true;
                player2CanMoveShip = false;
                player1CanMoveShip = true;
            }
            InvalidateRect(nullptr, nullptr, TRUE);
        }
        }
}

void handleTwoPlayersShot(HWND hwnd, int x, int y, bool isPlayer1) {
    if ((isPlayer1 && !player1Moved) || (!isPlayer1 && !player2Moved)) return;
    std::vector<std::vector<char>>& targetBoard = isPlayer1 ? botBoard : playerBoard;
    int startX = isPlayer1 ? botStartX : playerStartX;
    int startY = isPlayer1 ? botStartY : playerStartY;
    if (x >= startX && x < startX + BOARD_SIZE * CELL_SIZE &&
        y >= startY && y < startY + BOARD_SIZE * CELL_SIZE) {
        int col = (x - startX) / CELL_SIZE;
        int row = (y - startY) / CELL_SIZE;
        if (targetBoard[row][col] != HIT && targetBoard[row][col] != MISS) {
            if (targetBoard[row][col] == SHIP) {
                targetBoard[row][col] = HIT;
                InvalidateRect(hwnd, nullptr, TRUE);
                showDistance(0, startX, startY);
                StartEndGameTimer(hwnd, isPlayer1, true);
                return;
            }
            if (targetBoard[row][col] == EMPTY) {
                targetBoard[row][col] = MISS;
            }
            int minDistance = INT_MAX;
            bool shipFound = false;
            for (int i = 0; i < BOARD_SIZE; ++i) {
                for (int j = 0; j < BOARD_SIZE; ++j) {
                    if (targetBoard[i][j] == SHIP) {
                        int distance = calculateManhattanDistance(row, col, i, j);
                        minDistance = std::min(minDistance, distance);
                        shipFound = true;
                    }
                }
            }
            if (!shipFound) {
                StartEndGameTimer(hwnd, isPlayer1, true);
            }
            showDistance(minDistance, startX, startY);
            if (isPlayer1) {
                player1Moved = false;
                player1MovedOnce = false;
                player2CanMoveShip = true;
            } else {
                player2Moved = false;
                player2MovedOnce = false;
                player1CanMoveShip = true;
            }
            hideAllShips = true;
            InvalidateRect(hwnd, nullptr, TRUE);
            const char* message = isPlayer1Turn ? "Player 1's turn! Press OK to continue." : "Player 2's turn! Press OK to continue.";
            MessageBoxA(hwnd, message, "Turn Change", MB_OK | MB_ICONINFORMATION);
            hideAllShips = false;
            playerTurnConfirmed = true;
            isPlayer1Turn = !isPlayer1Turn;
            InvalidateRect(hwnd, nullptr, TRUE);
        }
    }
}

void createStartButton(HWND hwnd) {
    randomBot = CreateWindowA("BUTTON", "Easy Mode", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 420, 200, 130, 30, hwnd, (HMENU)3, GetModuleHandle(nullptr), nullptr);
    hardBot = CreateWindowA("BUTTON", "Hard mode", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 420, 240, 130, 30, hwnd, (HMENU)4, GetModuleHandle(nullptr), nullptr);
    twoPlayers = CreateWindowA("BUTTON", "2 players mode", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 420, 280, 130, 30, hwnd, (HMENU)5, GetModuleHandle(nullptr), nullptr);
    exit_ = CreateWindowA("BUTTON", "Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 420, 320, 130, 30, hwnd, (HMENU)6, GetModuleHandle(nullptr), nullptr);
}

void hideButtons() {
    DestroyWindow(randomBot);
    DestroyWindow(hardBot);
    DestroyWindow(twoPlayers);
    DestroyWindow(exit_);
    DestroyWindow(newGame);
    DestroyWindow(mainMenu);
    randomBot = nullptr;
    hardBot = nullptr;
    twoPlayers = nullptr;
    exit_ = nullptr;
    newGame = nullptr;
    mainMenu = nullptr;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool gameStarted = false;
    static Gdiplus::Image* backgroundImage = nullptr;
    static Gdiplus::Image* gameBackgroundImage = nullptr;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT clientRect;
    int windowWidth, windowHeight, totalBoardWidth, totalBoardHeight;

    switch (uMsg) {
        case WM_CREATE:
            backgroundImage = Gdiplus::Image::FromFile(L"background1.bmp");
            gameBackgroundImage = Gdiplus::Image::FromFile(L"background2.bmp");
            if (!backgroundImage || backgroundImage->GetLastStatus() != Gdiplus::Ok) {
                MessageBoxA(hwnd, "Failed to load background image.", "Error", MB_OK | MB_ICONERROR);
                backgroundImage = nullptr;
            }
            if (!gameBackgroundImage || gameBackgroundImage->GetLastStatus() != Gdiplus::Ok) {
                MessageBoxA(hwnd, "Failed to load game background image.", "Error", MB_OK | MB_ICONERROR);
                gameBackgroundImage = nullptr;
            }
            GetClientRect(hwnd, &clientRect);
            createStartButton(hwnd);
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            {
                GetClientRect(hwnd, &clientRect);
                windowWidth = clientRect.right - clientRect.left;
                windowHeight = clientRect.bottom - clientRect.top;
                totalBoardWidth = (2 * BOARD_SIZE * CELL_SIZE) + 20;
                totalBoardHeight = BOARD_SIZE * CELL_SIZE;
                playerStartX = (windowWidth - totalBoardWidth) / 2;
                botStartX = playerStartX + BOARD_SIZE * CELL_SIZE + 20;
                playerStartY = (windowHeight - totalBoardHeight) / 2;
                botStartY = playerStartY;
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP hbmMem = CreateCompatibleBitmap(hdc, windowWidth, windowHeight);
                HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
                Gdiplus::Graphics graphics(hdcMem);
                if (!gameStarted && backgroundImage) {
                    graphics.DrawImage(backgroundImage, 0, 0, windowWidth, windowHeight);
                } else if (gameStarted && gameBackgroundImage) {
                    graphics.DrawImage(gameBackgroundImage, 0, 0, windowWidth, windowHeight);
                    drawGrid(hdcMem, playerBoard, playerStartX, playerStartY, false);
                    drawGrid(hdcMem, botBoard, botStartX, botStartY, true);
                }
                if (!isTwoPlayersMode) {
                    if (lastDistance >= 0) {
                        SetBkMode(hdc, TRANSPARENT);
                        char distanceText[50];
                        sprintf(distanceText, "Manhattan Distance: %d", lastDistance);
                        TextOutA(hdcMem, botStartX, botStartY + BOARD_SIZE * CELL_SIZE + 10, distanceText, strlen(distanceText));
                    }
                } else {
                    if (lastDistancePlayer1 >= 0) {
                        SetBkMode(hdcMem, TRANSPARENT);
                        char distanceText[50];
                        sprintf(distanceText, "Manhattan Distance: %d", lastDistancePlayer1);
                        TextOutA(hdcMem, playerStartX, playerStartY + BOARD_SIZE * CELL_SIZE + 10, distanceText, strlen(distanceText));
                    }
                    if (lastDistancePlayer2 >= 0) {
                        SetBkMode(hdcMem, TRANSPARENT);
                        char distanceText[50];
                        sprintf(distanceText, "Manhattan Distance: %d", lastDistancePlayer2);
                        TextOutA(hdcMem, botStartX, botStartY + BOARD_SIZE * CELL_SIZE + 10, distanceText, strlen(distanceText));
                    }
                }
                BitBlt(hdc, 0, 0, windowWidth, windowHeight, hdcMem, 0, 0, SRCCOPY);
                SelectObject(hdcMem, hbmOld);
                DeleteObject(hbmMem);
                DeleteDC(hdcMem);
            }
            EndPaint(hwnd, &ps);
            return 0;

        case WM_DESTROY:
            if (backgroundImage) {
                delete backgroundImage;
                backgroundImage = nullptr;
            }
            if (gameBackgroundImage) {
                delete gameBackgroundImage;
                gameBackgroundImage = nullptr;
            }
            Gdiplus::GdiplusShutdown(gdiplusToken);
            PostQuitMessage(0);
            return 0;

        case WM_LBUTTONDOWN:
            if (gameStarted) {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                if (isTwoPlayersMode) {
                    if (!playerShipPlaced) {
                        handleTwoPlayersPlacement(x, y, isPlayer1Turn);
                    } else if (x >= (isPlayer1Turn ? playerStartX : botStartX) &&
                               x < (isPlayer1Turn ? playerStartX + BOARD_SIZE * CELL_SIZE : botStartX + BOARD_SIZE * CELL_SIZE) &&
                               y >= (isPlayer1Turn ? playerStartY : botStartY) &&
                               y < (isPlayer1Turn ? playerStartY + BOARD_SIZE * CELL_SIZE : botStartY + BOARD_SIZE * CELL_SIZE)) {
                        handleTwoPlayersMove(x, y, isPlayer1Turn);
                    } else {
                        handleTwoPlayersShot(hwnd, x, y, isPlayer1Turn);
                    }
                } else {
                    if (!playerShipPlaced) {
                        handlePlayerPlacement(x, y);
                    } else if (x >= playerStartX && x < playerStartX + BOARD_SIZE * CELL_SIZE &&
                               y >= playerStartY && y < playerStartY + BOARD_SIZE * CELL_SIZE) {
                        handlePlayerMove(x, y);
                    } else if (x >= botStartX && x < botStartX + BOARD_SIZE * CELL_SIZE &&
                               y >= botStartY && y < botStartY + BOARD_SIZE * CELL_SIZE) {
                        handlePlayerShot(hwnd, x, y);
                    }
                }
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1: {
                    if (MessageBoxA(hwnd, "Are you sure? The current game will be lost.", "Confirm New Game", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        startNewGame();
                        InvalidateRect(hwnd, nullptr, TRUE);
                    }
                    break;
                }
                case 2: {
                    if (MessageBoxA(hwnd, "Return to the main menu?", "Confirm Exit", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        lastDistance = -1;
                        lastDistancePlayer1 = -1;
                        lastDistancePlayer2 = -1;
                        gameStarted = false;
                        isHardMode = false;
                        isTwoPlayersMode = false;
                        hideButtons();
                        createStartButton(hwnd);
                        InvalidateRect(hwnd, nullptr, TRUE);
                    }
                    break;
                }
                case 3:
                    gameStarted = true;
                    startNewGame();
                    hideButtons();
                    newGame = CreateWindowA("BUTTON", "New game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 710, 50, 100, 30, hwnd, (HMENU)1, nullptr, nullptr);
                    mainMenu = CreateWindowA("BUTTON", "Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 820, 50, 100, 30, hwnd, (HMENU)2, nullptr, nullptr);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                case 4:
                    gameStarted = true;
                    isHardMode = true;
                    startNewGame();
                    hideButtons();
                    newGame = CreateWindowA("BUTTON", "New game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 710, 50, 100, 30, hwnd, (HMENU)1, nullptr, nullptr);
                    mainMenu = CreateWindowA("BUTTON", "Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 820, 50, 100, 30, hwnd, (HMENU)2, nullptr, nullptr);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                case 5:
                    gameStarted = true;
                    isTwoPlayersMode = true;
                    initializeTwoPlayersGame();
                    hideButtons();
                    newGame = CreateWindowA("BUTTON", "New game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 710, 50, 100, 30, hwnd, (HMENU)1, nullptr, nullptr);
                    mainMenu = CreateWindowA("BUTTON", "Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 820, 50, 100, 30, hwnd, (HMENU)2, nullptr, nullptr);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                case 6:
                    PostQuitMessage(0);
                    break;
            }
            break;

        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                int result = MessageBoxA(hwnd, "You won! Start a new game?", "Game Over", MB_YESNO | MB_ICONINFORMATION);
                if (result == IDYES) {
                    startNewGame();
                    InvalidateRect(hwnd, nullptr, TRUE);
                } else if (result == IDNO) {
                    PostQuitMessage(0);
                }
            } else if (wParam == 2) {
                KillTimer(hwnd, 2);
                int result = MessageBoxA(hwnd, "You lost! Start a new game?", "Game Over", MB_YESNO | MB_ICONINFORMATION);
                if (result == IDYES) {
                    startNewGame();
                    InvalidateRect(hwnd, nullptr, TRUE);
                } else if (result == IDNO) {
                    PostQuitMessage(0);
                }
            } else if (wParam == 3) {
                KillTimer(hwnd, 3);
                int result = MessageBoxA(hwnd, "Player 1 won! Start a new game?", "Game Over", MB_YESNO | MB_ICONINFORMATION);
                if (result == IDYES) {
                    startNewGame();
                    InvalidateRect(hwnd, nullptr, TRUE);
                } else if (result == IDNO) {
                    PostQuitMessage(0);
                }
            } else if (wParam == 4) {
                KillTimer(hwnd, 4);
                int result = MessageBoxA(hwnd, "Player 2 won! Start a new game?", "Game Over", MB_YESNO | MB_ICONINFORMATION);
                if (result == IDYES) {
                    startNewGame();
                    InvalidateRect(hwnd, nullptr, TRUE);
                } else if (result == IDNO) {
                    PostQuitMessage(0);
                }
            }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetConsoleOutputCP(CP_UTF8);
    srand(static_cast<unsigned>(time(nullptr)));
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    startNewGame();
    initializeBoard(playerBoard);
    initializeBoard(botBoard);
    placeBotShip(botBoard);
    const wchar_t CLASS_NAME[] = L"SeaBattleWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"New SeaBattle",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 600,
        nullptr, nullptr, hInstance, nullptr);
    if (hwnd == nullptr) return 0;
    ShowWindow(hwnd, nCmdShow);
    InvalidateRect(hwnd, nullptr, TRUE);
    UpdateWindow(hwnd);
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}
