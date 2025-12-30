#include <stdio.h>
#include <stdlib.h>

#define EMPTY '.'
#define WIN 4

char **createBoard(int rows, int cols) {
    char **b = malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++)
        b[i] = malloc(cols * sizeof(char));
    return b;
}

void freeBoard(char **b, int rows) {
    for (int i = 0; i < rows; i++)
        free(b[i]);
    free(b);
}

void initBoard(char **b, int rows, int cols) {
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            b[r][c] = EMPTY;
}

void printBoard(char **b, int rows, int cols) {
    printf("\n  ");
    for (int c = 0; c < cols; c++) printf(" %d", c + 1);
    printf("\n");

    for (int r = 0; r < rows; r++) {
        printf("  ");
        for (int c = 0; c < cols; c++)
            printf(" %c", b[r][c]);
        printf("\n");
    }
    printf("\n");
}

int dropPiece(char **b, int rows, int col, char sym) {
    for (int r = rows - 1; r >= 0; r--) {
        if (b[r][col] == EMPTY) {
            b[r][col] = sym;
            return r;
        }
    }
    return -1;
}

int isBoardFull(char **b, int cols) {
    for (int c = 0; c < cols; c++)
        if (b[0][c] == EMPTY) return 0;
    return 1;
}

int countDir(char **b, int rows, int cols, int r, int c, int dr, int dc, char sym) {
    int cnt = 0;
    while (r >= 0 && r < rows && c >= 0 && c < cols && b[r][c] == sym) {
        cnt++;
        r += dr;
        c += dc;
    }
    return cnt;
}

int checkWinFrom(char **b, int rows, int cols, int r, int c, char sym) {
    int dirs[4][2] = {{0,1},{1,0},{1,1},{1,-1}};
    for (int i = 0; i < 4; i++) {
        int a = countDir(b, rows, cols, r, c, dirs[i][0], dirs[i][1], sym);
        int d = countDir(b, rows, cols, r, c, -dirs[i][0], -dirs[i][1], sym);
        if (a + d - 1 >= WIN) return 1;
    }
    return 0;
}

int main(void) {
    int ROWS, COLS;
    char **board;

    do {
        printf("Número de linhas (>=5): ");
        scanf("%d", &ROWS);
        printf("Número de colunas (>=5): ");
        scanf("%d", &COLS);
    } while (ROWS < 5 || COLS < 5);

    board = createBoard(ROWS, COLS);
    initBoard(board, ROWS, COLS);

    int player = 0;
    char symbols[2] = {'X', 'O'};

    while (1) {
        printBoard(board, ROWS, COLS);

        printf("Jogador %d (%c) - escolhe coluna (1-%d): ",
               player + 1, symbols[player], COLS);

        int colInput;
        if (scanf("%d", &colInput) != 1) {
            printf("Input inválido.\n");
            while (getchar() != '\n');
            continue;
        }

        while (getchar() != '\n');

        if (colInput < 1 || colInput > COLS) {
            printf("Coluna fora do intervalo.\n");
            continue;
        }

        int col = colInput - 1;
        int row = dropPiece(board, ROWS, col, symbols[player]);
        if (row == -1) {
            printf("Essa coluna está cheia.\n");
            continue;
        }

        if (checkWinFrom(board, ROWS, COLS, row, col, symbols[player])) {
            printBoard(board, ROWS, COLS);
            printf("🏆 Jogador %d (%c) ganhou!\n", player + 1, symbols[player]);
            break;
        }

        if (isBoardFull(board, COLS)) {
            printBoard(board, ROWS, COLS);
            printf("🤝 Empate! Tabuleiro cheio.\n");
            break;
        }

        player = 1 - player;
    }

    freeBoard(board, ROWS);
    return 0;
}