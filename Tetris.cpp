// Tetris.cpp : Defines the entry point for the application.
//

#include "Tetris.h"
#include <iostream>
#include <stdio.h> 
#include <vector>
#include <Windows.h>
#include <thread>
using namespace std;


wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;

int nScreenWidth = 80;
int nScreenHeight = 30;
unsigned char *pField = nullptr;

int Rotate(int x, int y, int r) {
	switch (r % 4)
	{
	case 0: return y * 4 + x;			//0 degrees
	case 1: return 12 + y - (x * 4);	// 90 degrees
	case 2: return 15 - (y * 4) - x;	// 180 degrees
	case 3: return 3 - y + (x * 4);		// 270 degrees
	default:
		return 0;
	}
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			//Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if ((nPosX + px < 0 || nPosX + px >= nFieldWidth) ||
				(nPosY + py < 0 || nPosY + py >= nFieldHeight) ||
				(tetromino[nTetromino][pi] != L'.' && pField[fi] != 0))
			{
				return false; //Fail on first hit
			}
			
		}
	}
	return true;
}

int main()
{
	cout << "Hello Tetris." << endl;


	wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
	//vector<wchar_t> screen(nScreenWidth * nScreenHeight);
	//string screen;
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		screen[i] = L' ';
	}
	HANDLE hconsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hconsole);
	DWORD dwBytesWritten = 0;


	//Generate Assets
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX..X......");
	tetromino[2].append(L".X...XX...X.....");
	tetromino[3].append(L".....XX..XX.....");
	tetromino[4].append(L"..X..XX...X.....");
	tetromino[5].append(L".....XX...X...X.");
	tetromino[6].append(L".....XX..X...X..");

	pField = new unsigned char[nFieldWidth * nFieldHeight];

	for (int i = 0; i < nFieldWidth; i++)
	{
		for (int j = 0; j < nFieldHeight; j++) {
			pField[j * nFieldWidth + i] = (i == 0 || i == nFieldWidth - 1 || j == nFieldHeight - 1) ? 9 : 0;
		}
	}

	// Game Logic
	bool bKey[4];
	int nCurrentPiece = 2;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHold = true;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vLines;
	bool bGameOver = false;

	while (!bGameOver) {
		// Game Timing ======================
		this_thread::sleep_for(50ms); // Game Tick

		++nSpeedCount;
		bForceDown = (nSpeedCount == nSpeed);
		// INPUT  ===========================
		for (int k = 0; k < 4; k++)
		{														 //R	L	D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}
		// GAME LOGIC =======================
		nCurrentX += (bKey[0]) && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY));
		nCurrentX -= (bKey[1]) && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY));
		nCurrentY += (bKey[2]) && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1));
			
		
		if (bKey[3])
		{
			nCurrentRotation += !bRotateHold && (DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY));
			bRotateHold = true;
		}
		else {
			bRotateHold = false;
		}

		if (bForceDown) {
			nSpeedCount = 0;

			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else {
				// Lock Piece In
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}
				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) --nSpeed;

				// Check for lines
				for (int py = 0; py < 4; py++)
				{
					if (nCurrentY + py < nFieldHeight -1) {
						bool bline = true;
						for (int px = 1; px < nFieldWidth - 1; px++) {
							bline &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}
						if (bline)
						{
							for (int px = 0; px < nFieldWidth; px++)
							{
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							}
							vLines.push_back(nCurrentY + py);
						}
					}
				}

				nScore += 25;
				if (!vLines.empty())
				{
					nScore += (1 << vLines.size()) * 100;
				}
				// Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = rand() % 4;
				nCurrentPiece = rand() % 7;
				// Check if GameOver
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

			}
			
		}
		//RENDER OUTPUT =====================

		// Draw Field 
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		// Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		//Draw Current Piece
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
				}
			}
		}
		if (!vLines.empty()) {
			// Display Frame 
			WriteConsoleOutputCharacterW(hconsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // Chill a bit
			for (auto& v : vLines) {
				for (int  px = 0; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}
			vLines.clear();
		}
		WriteConsoleOutputCharacterW(hconsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	 }

	 // Oh Dear
	 CloseHandle(hconsole);
	 cout << "Game Over!! Score:" << nScore << endl;
	 system("pause");


	return 0;
}
