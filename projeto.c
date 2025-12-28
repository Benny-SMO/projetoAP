#include <stdio.h>

#define ROWS 6
#define COLS 7
#define EMPTY '.'
#define WIN 4

void initBoard(char b[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            b[r][c] = EMPTY;
}

void printBoard(char b[ROWS][COLS]) {
    printf("\n  ");
    for (int c = 0; c < COLS; c++) printf(" %d", c + 1);
    printf("\n");

    for (int r = 0; r < ROWS; r++) {
        printf("  ");
        for (int c = 0; c < COLS; c++) printf(" %c", b[r][c]);
        printf("\n");
    }
    printf("\n");
}

int dropPiece(char b[ROWS][COLS], int col, char sym) {
    // col: 0..COLS-1
    for (int r = ROWS - 1; r >= 0; r--) {
        if (b[r][col] == EMPTY) {
            b[r][col] = sym;
            return r; // devolve a linha onde caiu
        }
    }
    return -1; // coluna cheia
}

int isBoardFull(char b[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++)
        if (b[0][c] == EMPTY) return 0;
    return 1;
}

int countDir(char b[ROWS][COLS], int r, int c, int dr, int dc, char sym) {
    int cnt = 0;
    while (r >= 0 && r < ROWS && c >= 0 && c < COLS && b[r][c] == sym) {
        cnt++;
        r += dr;
        c += dc;
    }
    return cnt;
}

int checkWinFrom(char b[ROWS][COLS], int r, int c, char sym) {
    // 4 direções base: horiz, vert, diag\, diag/
    int dirs[4][2] = {{0,1},{1,0},{1,1},{1,-1}};

    for (int i = 0; i < 4; i++) {
        int dr = dirs[i][0], dc = dirs[i][1];
        int a = countDir(b, r, c, dr, dc, sym);
        int d = countDir(b, r, c, -dr, -dc, sym);
        int total = a + d - 1; // (r,c) contado 2x
        if (total >= WIN) return 1;
    }
    return 0;
}

int main(void) {
    char board[ROWS][COLS];
    initBoard(board);

    int player = 0;
    char symbols[2] = {'X', 'O'};

    while (1) {
        printBoard(board);

        printf("Jogador %d (%c) - escolhe coluna (1-%d): ",
               player + 1, symbols[player], COLS);

        int colInput;
        if (scanf("%d", &colInput) != 1) {
            // input inválido
            printf("Input inválido.\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }

        // limpar resto da linha
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}

        if (colInput < 1 || colInput > COLS) {
            printf("Coluna fora do intervalo.\n");
            continue;
        }

        int col = colInput - 1;
        int row = dropPiece(board, col, symbols[player]);
        if (row == -1) {
            printf("Essa coluna está cheia.\n");
            continue;
        }

        if (checkWinFrom(board, row, col, symbols[player])) {
            printBoard(board);
            printf("🏆 Jogador %d (%c) ganhou!\n", player + 1, symbols[player]);
            break;
        }

        if (isBoardFull(board)) {
            printBoard(board);
            printf("🤝 Empate! O tabuleiro está cheio.\n");
            break;
        }

        player = 1 - player; // alterna 0 <-> 1
    }

    return 0;
}