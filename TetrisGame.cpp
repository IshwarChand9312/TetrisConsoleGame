#include<iostream>
#include<vector>
#include<thread>
#include<stdio.h>
#include<Windows.h>
using namespace std;


wstring tetromino[7];


int nFieldWidth = 12;
int nFieldHeight = 18;

int nScreenWidth = 80;
int nScreenHeight = 30;

unsigned char* pField = nullptr;


// To rotate the particular assest in 0 , 90 , 180 , 270
int rotate(int px, int py, int r) {

	int pi = 0;
	switch (r % 4)
	{
	case 0: 
		pi =  py * 4 + px;
		break;
	case 1:
		pi = 12 + py - (px * 4);
		break;
	case 2: 
		pi = 15 - (py * 4) - px;
		break;
	case 3: 
		pi = 3 - py + (px * 4);
		break;
	}

	return pi;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {

	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {
		
			int pi = rotate(px, py, nRotation);

			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosY + py >= 0 && nPosX + px < nFieldWidth && nPosY + py < nFieldHeight) {
			
				if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0) return false;
			}

		}
	}

	return true;
}

int main() {
	
	// Tetrominoics blocks asssest 4*4 
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");



	pField = new unsigned char[nFieldHeight * nFieldWidth]; //  play field buffer
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}


	wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];

	for (int i = 0; i < nScreenHeight * nScreenWidth; i++)  screen[i] = L' ';

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	bool bGameOver = false;
	bool bkeys[4];
	bool bRotateHold = true;
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool forceDown = false;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vLines;

	while (!bGameOver)
	{
		// Game Timing
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		forceDown = (nSpeed == nSpeedCount);

		// Input 
		for (int k = 0; k < 4; k++) {							// R L D Z
			bkeys[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}
		// Game Logic 

		nCurrentX += (bkeys[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bkeys[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bkeys[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		if (bkeys[3]) {
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else bRotateHold = true;
			// Force the peice down if it is the time

		if (forceDown) {
		

			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))	nCurrentY++;
			
			else{
				//Lock the current piece into the field
	
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				//Check have we got the line

				nPieceCount++;

				if (nPieceCount % 50 == 0) if (nSpeed >= 10) nSpeed--;

				for (int py = 0; py < 4; py++) {
					if (nCurrentY + py < nFieldHeight - 1) 
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++) {
						
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}

						if (bLine) {
						
							// Remove Line , set to = 
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}
				}

				nScore += 25;

				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				// choose next piece

				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// if piece does not fit

				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

			}

			nSpeedCount = 0;
		}


		// Render Output 

		
		// Draw field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y+2)*(nScreenWidth) + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}
		
		// Draw Asset

		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] != L'.')
					screen[(nCurrentY + py  + 2) * (nScreenWidth) + (px + nCurrentX + 2)] = nCurrentPiece + 65;


		// Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);


		if (!vLines.empty()) {
			
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for ( auto &v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++) {
					
					for (int py = v; py > 0; py--) {
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
						pField[px] = 0;
					}
				}

			vLines.clear();
		}


		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		 
	}

	CloseHandle(hConsole);
	cout << " Game Over !! Score : " << nScore << endl;
	system("pause");
	return 0;
}