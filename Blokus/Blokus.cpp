// Blokus.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>

using namespace std;

#define ESQ_SUP_IZQ (char) 201
#define ESQ_SUP_DER (char) 187

#define ESQ_INF_IZQ (char) 200
#define ESQ_INF_DER (char) 188

#define BOR_VER (char) 186
#define BOR_HOR (char) 205

#define ESQ_SUP_IZQ_F (char) 218
#define ESQ_SUP_DER_F (char) 191

#define ESQ_INF_IZQ_F (char) 192
#define ESQ_INF_DER_F (char) 217

#define BOR_VER_F (char) 179
#define BOR_HOR_F (char) 196

#define CON_CEN (char) 206
#define CON_IZQ (char) 204
#define CON_DER (char) 185

#define CON_SUP (char) 203
#define CON_INF (char) 202

#define AZUL 25
#define AZUL_2 144
#define AZUL_3 1
#define AMARILLO 110
#define AMARILLO_2 224
#define AMARILLO_3 6
#define ROJO 76
#define ROJO_2 192
#define ROJO_3 4
#define VERDE 42
#define VERDE_2 160
#define VERDE_3 2

#define BLANCO 7
#define BLANCO_2 112

#define FILL_1 (char) 220
#define FILL_2 (char) 219
#define FILL_3 (char) 223
#define FILL_4 (char) 254

#define BLANK_1 (char) 176
#define BLANK_2 (char) 177
#define BLANK_3 (char) 178

#define RE_PAG 33
#define AV_PAG 34
#define INICIO 36
#define FIN 35
#define LEFT_ARR 37
#define UP_ARR 38
#define RIGHT_ARR 39
#define DOWN_ARR 40
#define FLIP_KEY 70
#define ROTATE_KEY 82
#define ESCAPE 27
#define ENTER 13

int PressAnyKey(const char *prompt)
{
	DWORD        mode;
	HANDLE       hstdin;
	INPUT_RECORD inrec;
	DWORD        count;

	/* Set the console mode to no-echo, raw input, */
	/* and no window or mouse events.              */
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hstdin == INVALID_HANDLE_VALUE
		|| !GetConsoleMode(hstdin, &mode)
		|| !SetConsoleMode(hstdin, 0))
		return 0;
	FlushConsoleInputBuffer(hstdin);

	/* Get a single key RELEASE */
	do ReadConsoleInput(hstdin, &inrec, 1, &count);
	while ((inrec.EventType != KEY_EVENT) || inrec.Event.KeyEvent.bKeyDown);

	/* Restore the original console mode */
	SetConsoleMode(hstdin, mode);

	return inrec.Event.KeyEvent.wVirtualKeyCode;
}

void goTo(int x, int y) {
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void showInstructions(int type) {
	goTo(94, 35);
	if (type == 1) {
		cout << "                                                  ";
		goTo(94, 37);
		cout << "TECLAS DE MOVIMIENTO: Seleccionar opciones.       ";
		goTo(94, 39);
		cout << "ENTER: Confirmar seleccion.                       ";
		goTo(94, 41);
		cout << "                                                  ";
	}
	else if (type == 2) {
		goTo(94, 35);
		cout << "R: Rotar la pieza.                                ";
		goTo(94, 37);
		cout << "F: Voltear horizontalmente.                       ";
		goTo(94, 39);
		cout << "ENTER: Confirmar posicion(si es valida).          ";
		goTo(94, 41);
		cout << "ESCAPE: Volver para seleccionar otra opcion.      ";
	}
	else {
		goTo(48, 25);
		cout << "El juego consiste en colocar todas las piezas posibles en el tablero.\n\n";
		goTo(48, 27);
		cout << "Las reglas son sencillas:\n";
		goTo(48, 28);
		cout << "  * La primera pieza debera ser colocada en una esquina del tablero.\n";
		goTo(48, 29);
		cout << "  * El resto de las piezas deben ser colocadas de manera tal que alguna de";
		goTo(48, 30);
		cout << "    sus esquinas toque la de otra pieza del mismo color.\n";
		goTo(48, 31);
		cout << "  * No se puede colocar una pieza de manera tal que alguno de sus bordes";
		goTo(48, 32);
		cout << "    toque el de otra pieza del mismo color.\n";
		goTo(48, 33);
		cout << "  * No existen restricciones con respecto a las piezas de distintos colores.\n";
		goTo(48, 34);
		cout << "  * El juego termina cuando el ultimo jugador se rinde.\n";
		goTo(48, 35);
		cout << "  * Cada pieza colocada otorga 1 PUNTO por cada cuadro que la compone.\n";
		goTo(48, 36);
		cout << "  * Al final del juego, cada pieza no colocada resta 1 PUNTO por cada cuadro";
		goTo(48, 37);
		cout << "    que la compone.\n";
		goTo(48, 38);
		cout << "  * Si un jugador logra colocar todas sus pieas, recibe 15 PUNTOS extra, mas";
		goTo(48, 39);
		cout << "    5 PUNTOS adicionales si la ultima pieza colocada fue de 1x1.";
		goTo(48, 41);
	}
}

struct player {
	int color;
	int invalidColor;
	bool pieces[22][5][5];
	bool first = true;
	bool surrender = false;
	bool lastPlayed1x1 = false;
	int score = 0;
	int taken[22];
	int piecesPlaced = 0;
};

void updatePlayerScore(struct player &guy) {
	int total = 0;
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				total += guy.pieces[i][j][k];
			}
		}
	}
	total = 89 - total - total;
	if (total == 89) {
		total += 15;
		if (guy.lastPlayed1x1) total += 5;
	}
	guy.score = total;
}

void rotate(bool shape[5][5]) {
	int copy[5][5];
	for (int h = 0; h < 2; h++) {
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				if (h == 0)copy[i][j] = shape[i][j];
				else shape[j][4 - i] = copy[i][j];
			}
		}
	}
}

void flip(bool shape[5][5]) {
	int aux;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 2; j++) {
			aux = shape[i][j];
			shape[i][j] = shape[i][4 - j];
			shape[i][4 - j] = aux;
		}
	}
}

void offTheEdges(bool piece[5][5], int &x, int &y) {
	bool perfect = false;
	if (x < 0 || y < 0 || x > 14 || y > 14)
		while (!perfect) {//Va a mover la ficha de a un lugar hasta que no se encuentren fuera
			perfect = true;
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					if (piece[i][j]) {
						if (i + y < 0) {
							y++;
							perfect = false;
						}
						else if (i + y > 19) {
							y--;
							perfect = false;
						}
						if (j + x < 0) {
							x++;
							perfect = false;
						}
						else if (j + x > 19) {
							x--;
							perfect = false;
						}
					}
				}
			}
		}

}

bool valid(int b[20][20], int x, int y, struct player &subj, int index, bool first) {
	bool corner = false;
	bool cornerpiece = false;
	int color = subj.color;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (subj.pieces[index][i][j]) {
				if ((i + y >= 20) || (i + y < 0) || (j + x >= 20) || (j + x < 0)) return false;
				else if (b[i + y][j + x] != BLANCO_2) return false;
				else if (i + y > 0) {
					if (b[i + y - 1][j + x] == color) {
						return false;
					}
				}
				if (i + y < 19) {
					if (b[i + y + 1][j + x] == color) {
						return false;
					}
				}
				if (j + x > 0) {
					if (b[i + y][j + x - 1] == color) {
						return false;
					}
				}
				if (j + x < 19) {
					if (b[i + y][j + x + 1] == color) {
						return false;
					}
				}
				//vamos bien, falta chequear esquina con esquina
				if (first) {
					if ((i + y == 0 || i + y == 19) && (j + x == 0 || j + x == 19)) cornerpiece = true;
				}
				else {
					if (j + x > 0) {
						if (i + y > 0) {
							if (b[i + y - 1][j + x - 1] == color) corner = true;
						}
						if (i + y < 19) {
							if (b[i + y + 1][j + x - 1] == color) corner = true;
						}
					}
					if (j + x < 19) {
						if (i + y > 0) {
							if (b[i + y - 1][j + x + 1] == color) corner = true;
						}
						if (i + y < 19) {
							if (b[i + y + 1][j + x + 1] == color) corner = true;
						}
					}
				}
			}
		}
	}
	return corner || (cornerpiece && first);
}

void assignShape(bool dest[5][5], bool templ[5][5]) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			dest[i][j] = templ[i][j];
		}
	}
}

void showButton(int x, int y, char message[], int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	int length = strlen(message);
	goTo(x, y);
	cout << ESQ_SUP_IZQ;
	for (int i = 0; i < length; i++) {
		cout << BOR_HOR << BOR_HOR;
	}
	cout << ESQ_SUP_DER;
	goTo(x, y + 1);
	cout << BOR_VER << " ";
	goTo(x + length * 2, y + 1);
	cout << " " << BOR_VER;
	goTo(x, y + 2);
	cout << ESQ_INF_IZQ;
	for (int i = 0; i < length; i++) {
		cout << BOR_HOR << BOR_HOR;
	}
	cout << ESQ_INF_DER;
	goTo(x + length / 2 + 1, y + 1);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
	cout << message;
	goTo(0, 0);
}

void setPiece(int b[20][20], int ind, int x, int y, struct player &subject) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (subject.pieces[ind][i][j])b[i + y][j + x] = subject.color;
		}
	}
	subject.taken[subject.piecesPlaced] = ind;
	subject.piecesPlaced++;
	assignShape(subject.pieces[ind], subject.pieces[21]);
}

void printSquare(int x, int y) {
	goTo(x, y);
	cout << "  " << FILL_1 << " ";
	goTo(x, y + 1);
	cout << " " << FILL_3 << FILL_3 << " ";
	goTo(x, y);
}

void refreshPiece(bool pieza[5][5], int x, int y, int newX, int newY, int board[20][20], int color) {
	goTo(x, y);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), board[i + y][j + x]);
			if (i + y <= 19 && i + y >= 0 && j + x <= 19 && j + x >= 0)printSquare((x + j) * 4 + 7, (y + i) * 2 + 2);
		}
	}
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
			if (pieza[i][j] && i + newY <= 19 && i + newY >= 0 && j + newX <= 19 && j + newX >= 0)printSquare((newX + j) * 4 + 7, (i + newY) * 2 + 2);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO_2);
		}
	}
	goTo(92, 43);
}

void showBoard(int b[20][20]) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO_2);
	goTo(2, 0);
	cout << " " << ESQ_SUP_IZQ;
	for (int i = 0; i < 21; i++) {
		cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
	}
	cout << BOR_HOR << BOR_HOR << ESQ_SUP_DER << " ";
	goTo(2, 1);
	cout << " " << BOR_VER;
	for (int i = 0; i < 21; i++) {
		cout << "    ";
	}
	cout << "  " << BOR_VER << " ";
	for (int i = 0; i < 20; i++) {
		goTo(2, i * 2 + 2);
		cout << " " << BOR_VER << "   ";
		goTo(2, i * 2 + 3);
		cout << " " << BOR_VER << "   ";
		for (int j = 0; j < 20; j++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), b[i][j]);
			printSquare(j * 4 + 7, i * 2 + 2);
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO_2);
		goTo(87, i * 2 + 2);
		cout << "   " << BOR_VER << " ";
		goTo(87, i * 2 + 3);
		cout << "   " << BOR_VER << " ";
	}
	goTo(2, 42);
	cout << " " << BOR_VER;
	for (int i = 0; i < 21; i++) {
		cout << "    ";
	}
	cout << "  " << BOR_VER << " ";
	goTo(2, 43);
	cout << " " << ESQ_INF_IZQ;
	for (int i = 0; i < 21; i++) {
		cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
	}
	cout << BOR_HOR << BOR_HOR << ESQ_INF_DER << " ";
}

void showPiece(int color, bool piece[5][5], int x, int y, bool highlight) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (piece[i][j]) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
				goTo(x + j * 2, y + i);
				cout << FILL_2 << FILL_2;
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			}
			else if (highlight) {
				goTo(x + j * 2, y + i);
				cout << FILL_2 << FILL_2;
			}
			else {
				goTo(x + j * 2, y + i);
				cout << "  ";
			}
		}
	}
}

void showPlayer(struct player subject) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5 - (i == 4) * 4; j++) {
			showPiece(subject.color, subject.pieces[i * 5 + j], 93 + j * 14, 2 + i * 7, false);
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
}

bool previewBoard(int b[20][20], struct player &subject, int selected) {
	int x = 8, y = 8, previewColor, invalidColor;
	showInstructions(2);
	switch (subject.color) {
	case AZUL:
		previewColor = AZUL_2;
		invalidColor = AZUL_3;
		break;
	case VERDE:
		previewColor = VERDE_2;
		invalidColor = VERDE_3;
		break;
	case AMARILLO:
		previewColor = AMARILLO_2;
		invalidColor = AMARILLO_3;
		break;
	default:
		previewColor = ROJO_2;
		invalidColor = ROJO_3;
	}

	int key;
	bool cool = valid(b, x, y, subject, selected, subject.first);
	int useThisColor;
	if (cool) {
		useThisColor = previewColor;
	}
	else {
		useThisColor = invalidColor;
	}
	refreshPiece(subject.pieces[selected], x, y, x, y, b, useThisColor);
	bool finished = false;
	bool back = false;
	do {
		key = PressAnyKey("");
		int prevX = x, prevY = y;
		switch (key) {
		case RE_PAG:
			y -= 5;
			break;
		case UP_ARR:
			y--;
			break;
		case AV_PAG:
			y += 5;
			break;
		case DOWN_ARR:
			y++;
			break;
		case INICIO:
			x -= 5;
			break;
		case LEFT_ARR:
			x--;
			break;
		case FIN:
			x += 5;
			break;
		case RIGHT_ARR:
			x++;
			break;
		case ROTATE_KEY:
			rotate(subject.pieces[selected]);
			break;
		case FLIP_KEY:
			flip(subject.pieces[selected]);
			break;
		case ENTER:
			if (valid(b, x, y, subject, selected, subject.first)) {
				setPiece(b, selected, x, y, subject);
				subject.first = false;
				if (selected == 1) subject.lastPlayed1x1 == true;
				finished = true;
			}
			break;
		case ESCAPE:
			selected = 21;
			refreshPiece(subject.pieces[selected], prevX, prevY, x, y, b, BLANCO);
			back = true;
			break;
		}
		if (x < -2) x = -2;
		if (x > 17) x = 17;
		if (y < -2) y = -2;
		if (y > 17) y = 17;
		offTheEdges(subject.pieces[selected], x, y);
		int useThisColor;
		if (valid(b, x, y, subject, selected, subject.first)) {
			useThisColor = previewColor;
		}
		else {
			useThisColor = invalidColor;
		}
		refreshPiece(subject.pieces[selected], prevX, prevY, x, y, b, useThisColor);
	} while (!finished && !back);
	return finished;
}

int getValidCharacter() {
	int key = PressAnyKey("");
	if (key == LEFT_ARR || key == RIGHT_ARR || key == UP_ARR || key == DOWN_ARR || key == ENTER || key == FLIP_KEY || key == ROTATE_KEY || key == ESCAPE) return key;
	else return -1;
}

int showUserInterface(struct player &subject) {
	showPlayer(subject);
	showInstructions(1);
	int x = 0, y = 0;
	int pressedKey;
	char passButton[9] = "PASO";
	char surrenButton[9] = "ME RINDO";
	showButton(93 + 2 * 14, 2 + 4 * 7, passButton, BLANCO);
	showButton(93 + 4 * 14, 2 + 4 * 7, surrenButton, BLANCO);
	showPiece(subject.color, subject.pieces[y * 5 + x], 93 + x * 14, 2 + y * 7, true);
	goTo(92, 43);
	bool taken;
	do {
		pressedKey = getValidCharacter();
		if (y < 4 || (y == 4 && x == 0)) {
			showPiece(subject.color, subject.pieces[y * 5 + x], 93 + x * 14, 2 + y * 7, false);
		}
		else if (x == 2) {
			showButton(93 + x * 14, 2 + y * 7, passButton, BLANCO);
		}
		else {
			showButton(93 + x * 14, 2 + y * 7, surrenButton, BLANCO);
		}
		switch (pressedKey) {
		case UP_ARR:
			y--;
			break;
		case DOWN_ARR:
			if (!((x == 1 || x == 3) && y == 3)) y++;
			break;
		case LEFT_ARR:
			if (y == 4) x--;
			x--;
			break;
		case RIGHT_ARR:
			if (y == 4) x++;
			x++;
			break;
		case ESCAPE:
			y = 4;
			x = 2;
			break;
		}
		if (x > 4) x = 4;
		if (x < 0) x = 0;
		if (y > 4) y = 4;
		if (y < 0) y = 0;

		if (y < 4 || (y == 4 && x == 0)) {
			showPiece(subject.color, subject.pieces[y * 5 + x], 93 + x * 14, 2 + y * 7, true);
		}
		else if (x == 2) {
			showButton(93 + x * 14, 2 + y * 7, passButton, subject.color);
		}
		else {
			showButton(93 + x * 14, 2 + y * 7, surrenButton, subject.color);
		}
		taken = false;
		for (int i = 0; i < subject.piecesPlaced; i++) {
			if (subject.taken[i] == y * 5 + x) taken = true;
		}
		goTo(92, 43);
	} while (pressedKey != ENTER || taken);
	return y * 5 + x;
}

void jokeMenu() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
	goTo(69, 100);
	cout << "Holisss";
	char trash[1];
	goTo(0, 0);
	cout << "                 BLOKUS\n\n";
	cout << "Ingrese la cantidad de jugadores:\n";
	cin.getline(trash, 1);
	cout << "CARGANDO. . . ";
	for (int i = 0; i < 32; i++) {
		Sleep(rand() % 200);
		cout << ". ";
	}
	system("cls");
	Sleep(2000);
}

bool mainMenu(bool fake, bool wait) {
	if (fake)jokeMenu();
		short int sign[7][24] = { { AZUL, AZUL, AZUL, 0,    AMARILLO, 0, 0, 0                     , 0, ROJO, ROJO, 0, VERDE, 0, 0, VERDE, BLANCO_2, 0, 0,           BLANCO_2, 0, BLANCO_2, BLANCO_2, BLANCO_2 },
															{ AZUL, 0   , 0   , AZUL, AMARILLO, 0, 0, 0                     , ROJO, 0, 0, ROJO, VERDE, 0, VERDE, 0, BLANCO_2, 0, 0,           BLANCO_2, BLANCO_2, 0, 0, 0 },
															{ AZUL, 0   , 0   , AZUL, AMARILLO, 0, 0, 0                     , ROJO, 0, 0, ROJO, VERDE, 0, VERDE, 0, BLANCO_2, 0, 0,           BLANCO_2, BLANCO_2, 0, 0, 0 },
															{ AZUL, AZUL, AZUL, 0,    AMARILLO, 0, 0, 0                     , ROJO, 0, 0, ROJO, VERDE, VERDE, 0, 0, BLANCO_2, 0, 0,           BLANCO_2, 0, BLANCO_2, BLANCO_2, 0 },
															{ AZUL, 0   , 0   , AZUL, AMARILLO, 0, 0, 0                     , ROJO, 0, 0, ROJO, VERDE, VERDE, VERDE, 0, BLANCO_2, 0, 0,           BLANCO_2, 0, 0, 0,                BLANCO_2 },
															{ AZUL, 0   , 0   , AZUL, AMARILLO, 0, 0, 0                     , ROJO, 0, 0, ROJO, VERDE, 0, 0, VERDE, BLANCO_2, 0, 0,           BLANCO_2, 0, 0, 0,                BLANCO_2 },
															{ AZUL, AZUL, AZUL, 0,    AMARILLO, AMARILLO, AMARILLO, AMARILLO, 0, ROJO, ROJO, 0, VERDE, 0, 0, VERDE, 0, BLANCO_2, BLANCO_2, 0, BLANCO_2, BLANCO_2, BLANCO_2, 0 } };

		for (int i = 0; i < 7; i++) {
			int add = 0;
			for (int j = 0; j < 24; j++) {
				if (j % 4 == 0) add += 4;
				if (sign[i][j]) {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), sign[i][j]);
					printSquare(j * 4 + 22 + add, i * 2 + 5);
				}
			}
		}
		if(wait){
		goTo(70, 30);
		Sleep(2000);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
		cout << "PRESIONE CUALQUIER TECLA";
		PressAnyKey("");
		goTo(70, 30);
		cout << "                        ";
	}
	do {
		int x = 0, y = 0;
		int pressedKey;
		char playButton[14] = "    JUGAR    ";
		char tutorialButton[14] = "INSTRUCCIONES";
		char exitButton[14] = "    SALIR    ";
		showButton(70, 25, playButton, BLANCO_2);
		showButton(70, 30, tutorialButton, BLANCO);
		showButton(70, 35, exitButton, BLANCO);
		do {
			pressedKey = getValidCharacter();
			if (y == 0) {
				showButton(70, 25, playButton, BLANCO);
			}
			else if (y == 1) {
				showButton(70, 30, tutorialButton, BLANCO);
			}
			else {
				showButton(70, 35, exitButton, BLANCO);
			}
			switch (pressedKey) {
			case UP_ARR:
				y--;
				break;
			case DOWN_ARR:
				y++;
				break;
			case ESCAPE:
				y = 2;
				break;
			}
			if (y > 2) y = 2;
			if (y < 0) y = 0;
			if (y == 0) {
				showButton(70, 25, playButton, BLANCO_2);
			}
			else if (y == 1) {
				showButton(70, 30, tutorialButton, BLANCO_2);
			}
			else {
				showButton(70, 35, exitButton, BLANCO_2);
			}
			goTo(92, 42);
		} while (pressedKey != ENTER);
		switch (y) {
		case 0:
			for (int i = 25; i <= 41; i++) {
				goTo(65, i);
				cout << "                                             ";
			}
			return true;
			break;
		case 1:
			for (int i = 25; i <= 41; i++) {
				goTo(65, i);
				cout << "                                             ";
			}
			showInstructions(3);
			cout << "PRESIONE UNA TECLA PARA CONTINUAR";
			PressAnyKey("");
			for (int i = 25; i <= 41; i++) {
				goTo(48, i);
				cout << "                                                                                    ";
			}
			break;
		default:
			for (int i = 25; i <= 41; i++) {
				goTo(65, i);
				cout << "                                             ";
			}
			return(false);
			break;
		}
	} while (true);
}

bool secondMenu(int &maxPlayers, int &maxColors) {
	do {
		int x = 0, y = 0;
		int pressedKey;
		char p4Button[24] = "      4 JUGADORES      ";
		char p2c4Button[24] = "2 JUGADORES / 4 COLORES";
		char p2c2Button[24] = "2 JUGADORES / 2 COLORES";
		char backButton[24] = "         ATRAS         ";
		showButton(60, 25, p4Button, BLANCO_2);
		showButton(60, 30, p2c4Button, BLANCO);
		showButton(60, 35, p2c2Button, BLANCO);
		showButton(60, 40, backButton, BLANCO);
		do {
			pressedKey = getValidCharacter();
			if (y == 0) {
				showButton(60, 25, p4Button, BLANCO);
			}
			else if (y == 1) {
				showButton(60, 30, p2c4Button, BLANCO);
			}
			else if (y == 2) {
				showButton(60, 35, p2c2Button, BLANCO);
			}
			else {
				showButton(60, 40, backButton, BLANCO);
			}
			switch (pressedKey) {
			case UP_ARR:
				y--;
				break;
			case DOWN_ARR:
				y++;
				break;
			case ESCAPE:
				y = 3;
				break;
			}
			if (y > 3) y = 3;
			if (y < 0) y = 0;
			if (y == 0) {
				showButton(60, 25, p4Button, BLANCO_2);
			}
			else if (y == 1) {
				showButton(60, 30, p2c4Button, BLANCO_2);
			}
			else if (y == 2) {
				showButton(60, 35, p2c2Button, BLANCO_2);
			}
			else {
				showButton(60, 40, backButton, BLANCO_2);
			}
			goTo(92, 43);
		} while (pressedKey != ENTER);
		switch (y) {
		case 0:
			maxColors = 4;
			maxPlayers = 4;
			break;
		case 1:
			maxColors = 4;
			maxPlayers = 2;
			break;
		case 2:
			maxColors = 2;
			maxPlayers = 2;
			break;
		default:
			for (int i = 25; i <= 43; i++) {
				goTo(55, i);
				cout << "                                                     ";
			}
			return(false);
			break;
		}
	} while (maxColors == 0);
	system("cls");
	return true;
}

void gameOver(struct player &jugador1, struct player &jugador2) {
	goTo(121, 15);
	cout << "FIN DEL JUEGO.";
	updatePlayerScore(jugador1);
	updatePlayerScore(jugador2);
	goTo(101, 18);
	if (jugador1.score > jugador2.score) {
		cout << "1. ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), jugador1.invalidColor);
		cout << FILL_2 << FILL_2 << " " << FILL_2 << FILL_2;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
		cout << " --> " << jugador1.score;
		goTo(121, 20);
		cout << "2. ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), jugador2.invalidColor);
		cout << FILL_2 << FILL_2 << " " << FILL_2 << FILL_2;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
		cout << " --> " << jugador2.score;
	}
	else {
		cout << "1. ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), jugador2.invalidColor);
		cout << FILL_2 << FILL_2 << " " << FILL_2 << FILL_2;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
		cout << " --> " << jugador2.score;
		goTo(121, 20);
		cout << "2. ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), jugador1.invalidColor);
		cout << FILL_2 << FILL_2 << " " << FILL_2 << FILL_2;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
		cout << " --> " << jugador1.score;
	}
}

void gameOver(struct player players[], bool combine) {
	goTo(121, 15);
	cout << "FIN DEL JUEGO.";
	for (int i = 0; i < 4; i++) {
		updatePlayerScore(players[i]);
	}
	if (combine) {
		if (players[0].score + players[2].score > players[1].score + players[3].score) {
			goTo(121, 18);
			cout << "1. ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[0].invalidColor);
			cout << FILL_2 << FILL_2 << "  ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[2].invalidColor);
			cout << FILL_2 << FILL_2;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			cout << " --> " << players[0].score + players[2].score;
			goTo(121, 20);
			cout << "2. ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[1].invalidColor);
			cout << FILL_2 << FILL_2 << " ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[3].invalidColor);
			cout << FILL_2 << FILL_2;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			cout << " --> " << players[1].score + players[3].score;
		}
		else {
			goTo(121, 18);
			cout << "1. ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[1].invalidColor);
			cout << FILL_2 << FILL_2 << " ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[3].invalidColor);
			cout << FILL_2 << FILL_2;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			cout << " --> " << players[1].score + players[3].score;
			goTo(121, 20);
			cout << "2. ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[0].invalidColor);
			cout << FILL_2 << FILL_2 << " ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[2].invalidColor);
			cout << FILL_2 << FILL_2;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			cout << " --> " << players[0].score + players[2].score;
		}
	}
	else {
		struct player aux;
		bool sorted = true;
		do {
			sorted = true;
			for (int i = 0; i < 3; i++) {
				if (players[i].score < players[i + 1].score) {
					aux = players[i];
					sorted = false;
					players[i] = players[i + 1];
					players[i + 1] = aux;
				}
			}
		} while (!sorted);
		for (int i = 0; i < 4; i++) {
			goTo(121, 18 + 2 * i);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			cout << i + 1 << ". ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), players[i].invalidColor);
			cout << FILL_2 << FILL_2 << " " << FILL_2 << FILL_2;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
			cout << " --> " << players[i].score;
		}
	}
}

void resetPlayers(struct player players[], bool models[][5][5]) {
	players[0].color = AZUL;
	players[0].invalidColor = AZUL_3;
	players[1].color = AMARILLO;
	players[1].invalidColor = AMARILLO_3;
	players[2].color = ROJO;
	players[2].invalidColor = ROJO_3;
	players[3].color = VERDE;
	players[3].invalidColor = VERDE_3;
	for (int i = 0; i < 4; i++) {
		players[i].first = true;
		players[i].surrender = false;
		players[i].lastPlayed1x1 = false;
		players[i].score = 0;
		players[i].piecesPlaced = 0;
		for (int j = 0; j < 22; j++) {
			assignShape(players[i].pieces[j], models[j]);
		}
	}
}

int main() {
	int tablero[20][20];
	bool fake = true;
	bool wait = true;
	bool models[22][5][5] = { { { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, false, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, true, true, true },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, true, true, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, true, false, false, false },
															{ false, true, true, true, true },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, true },
															{ false, false, true, false, false },
															{ false, false, true, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ true, true, true, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, true, true, true, false },
															{ false, true, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ true, true, true, true, true },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, false, true, true, false },
															{ false, true, true, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, false, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, true, false, false },
															{ false, false, false, false, false } },

															{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ true, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } } };
	struct player jugadores[4];
	resetPlayers(jugadores, models);
	bool back = false;
	int maxPlayers, maxColors;
	do {
		resetPlayers(jugadores, models);
		if (mainMenu(fake, wait)) {
			fake = false; wait = false;
			maxPlayers = 0;
			maxColors = 0;
			if (secondMenu(maxPlayers, maxColors)) {
				for (int i = 0; i < 20; i++) {
					for (int j = 0; j < 20; j++) {
						tablero[i][j] = BLANCO_2;
					}
				}
				showBoard(tablero);
				int lost = 0;
				do {
					for (int i = 0; i < maxColors; i++) {
						bool finished = false;
						while (!finished && !jugadores[i].surrender) {
							int choice = showUserInterface(jugadores[i]);
							if (choice < 21) {
								finished = previewBoard(tablero, jugadores[i], choice);
							}
							else if (choice == 22) {
								finished = true;
							}
							else {
								jugadores[i].surrender = true;
								lost++;
								finished = true;
							}
						}
						if (jugadores[i].piecesPlaced >= 21) {
							jugadores[i].surrender = true;
							lost++;
						}
					}
				} while (lost < maxColors);
				system("cls");
				showBoard(tablero);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
				if (maxColors == 2) {
					gameOver(jugadores[0], jugadores[1]);
				}
				else {
					if (maxPlayers == 2) {
						gameOver(jugadores, true);
					}
					else {
						gameOver(jugadores, false);
					}
				}
				goTo(92, 42);
				system("pause");
				system("cls");
				back = true;
			}
			else back = true;
		}
		else back = false;
	} while (back);
	return 0;
}