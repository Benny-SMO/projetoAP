#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* ================= CONFIG ================= */

#define EMPTY '.'
#define MAX_PLAYERS 12
#define MAX_LINE 256

static const char SYMBOLS[] = "XOABCDEFGHIJKL";

/* ================= STRUCTS ================= */

typedef struct {
    int r, c;
    char sym;
} Move;

typedef struct {
    int rows, cols;
    int winLen;
    int nPlayers;
    int turn;

    char **board;
    char symbols[MAX_PLAYERS];

    Move *hist;
    int histSize, histCap;
} Game;

/* ================= INPUT ROBUSTO ================= */

static int read_line(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 1;
}

static int parse_int(const char *s, int *out) {
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') return 0;

    errno = 0;
    char *end;
    long v = strtol(s, &end, 10);
    if (errno != 0) return 0;

    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return 0;

    *out = (int)v;
    return 1;
}

static int read_int_range(const char *msg, int lo, int hi) {
    char line[MAX_LINE];
    for (;;) {
        printf("%s", msg);
        fflush(stdout);

        if (!read_line(line, sizeof(line))) exit(0);

        int v;
        if (!parse_int(line, &v)) {
            printf("-> Número inválido.\n");
            continue;
        }
        if (v < lo || v > hi) {
            printf("-> Intervalo [%d..%d].\n", lo, hi);
            continue;
        }
        return v;
    }
}

/* ================= MEMÓRIA ================= */

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Erro: memória insuficiente.\n");
        exit(1);
    }
    return p;
}

/* ================= TABULEIRO ================= */

static char **board_create(int rows, int cols) {
    char **b = xmalloc(rows * sizeof(char *));
    for (int r = 0; r < rows; r++)
        b[r] = xmalloc(cols * sizeof(char));
    return b;
}

static void board_free(char **b, int rows) {
    for (int r = 0; r < rows; r++) free(b[r]);
    free(b);
}

static void board_init(char **b, int rows, int cols) {
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            b[r][c] = EMPTY;
}

static void board_print(char **b, int rows, int cols) {
    printf("\n   ");
    for (int c = 0; c < cols; c++) printf("%2d", c + 1);
    printf("\n");

    for (int r = 0; r < rows; r++) {
        printf("%2d ", r + 1);
        for (int c = 0; c < cols; c++)
            printf(" %c", b[r][c]);
        printf("\n");
    }
    printf("\n");
}

static int board_drop(char **b, int rows, int col, char sym) {
    for (int r = rows - 1; r >= 0; r--) {
        if (b[r][col] == EMPTY) {
            b[r][col] = sym;
            return r;
        }
    }
    return -1;
}

static int board_full(char **b, int cols) {
    for (int c = 0; c < cols; c++)
        if (b[0][c] == EMPTY) return 0;
    return 1;
}

/* ================= VITÓRIA ================= */

static int count_dir(char **b, int rows, int cols,
                     int r, int c, int dr, int dc, char sym) {
    int cnt = 0;
    while (r >= 0 && r < rows && c >= 0 && c < cols && b[r][c] == sym) {
        cnt++;
        r += dr;
        c += dc;
    }
    return cnt;
}

static int check_win(char **b, int rows, int cols,
                     int winLen, int r, int c, char sym) {
    int dirs[4][2] = {{0,1},{1,0},{1,1},{1,-1}};
    for (int i = 0; i < 4; i++) {
        int a = count_dir(b, rows, cols, r, c, dirs[i][0], dirs[i][1], sym);
        int d = count_dir(b, rows, cols, r, c, -dirs[i][0], -dirs[i][1], sym);
        if (a + d - 1 >= winLen) return 1;
    }
    return 0;
}

/* ================= HISTÓRICO / UNDO ================= */

static void hist_init(Game *g) {
    g->histCap = 256;
    g->histSize = 0;
    g->hist = xmalloc(g->histCap * sizeof(Move));
}

static void hist_push(Game *g, int r, int c, char sym) {
    if (g->histSize >= g->histCap) {
        g->histCap *= 2;
        g->hist = realloc(g->hist, g->histCap * sizeof(Move));
        if (!g->hist) exit(1);
    }
    g->hist[g->histSize++] = (Move){r, c, sym};
}

static int undo(Game *g) {
    if (g->histSize == 0) return 0;
    Move m = g->hist[--g->histSize];
    g->board[m.r][m.c] = EMPTY;
    g->turn = (g->turn - 1 + g->nPlayers) % g->nPlayers;
    return 1;
}

/* ================= JOGO ================= */

static void print_help(void) {
    printf("Comandos:\n");
    printf("  <n>    jogar na coluna n\n");
    printf("  u      undo\n");
    printf("  help   ajuda\n");
    printf("  q      sair\n");
}

static void game_play(Game *g) {
    char line[MAX_LINE];

    for (;;) {
        clear_screen();
        board_print(g->board, g->rows, g->cols);

        printf("Liga-%d | Jogador %d (%c) > ",
               g->winLen, g->turn + 1, g->symbols[g->turn]);
        fflush(stdout);

        if (!read_line(line, sizeof(line))) return;

        if (strcmp(line, "q") == 0) return;
        if (strcmp(line, "help") == 0) { print_help(); continue; }

        if (strcmp(line, "u") == 0) {
            if (!undo(g)) printf("-> Nada para desfazer.\n");
            continue;
        }

        int colInput;
        if (!parse_int(line, &colInput) ||
            colInput < 1 || colInput > g->cols) {
            printf("-> Coluna inválida.\n");
            continue;
        }

        int row = board_drop(g->board, g->rows,
                             colInput - 1, g->symbols[g->turn]);
        if (row < 0) {
            printf("-> Coluna cheia.\n");
            continue;
        }

        hist_push(g, row, colInput - 1, g->symbols[g->turn]);

        if (check_win(g->board, g->rows, g->cols,
                      g->winLen, row, colInput - 1,
                      g->symbols[g->turn])) {
            board_print(g->board, g->rows, g->cols);
            printf("🏆 Jogador %d ganhou!\n", g->turn + 1);
            return;
        }

        if (board_full(g->board, g->cols)) {
            board_print(g->board, g->rows, g->cols);
            printf("🤝 Empate!\n");
            return;
        }

        g->turn = (g->turn + 1) % g->nPlayers;
    }
}

/* ================= MENU ================= */

int main(void) {
    for (;;) {
        printf("\n=== 4 EM LINHA ===\n");
        printf("1) Novo jogo\n");
        printf("2) Regras / Ajuda\n");
        printf("0) Sair\n");

        int op = read_int_range("Opção: ", 0, 2);
        if (op == 0) break;

        if (op == 2) {
            print_help();
            continue;
        }

        Game g;
        g.rows = read_int_range("Linhas (>=5): ", 5, 30);
        g.cols = read_int_range("Colunas (>=5): ", 5, 30);
        g.nPlayers = read_int_range("Jogadores (2..12): ", 2, MAX_PLAYERS);

        int maxWin = g.rows < g.cols ? g.rows : g.cols;
        g.winLen = read_int_range("Liga-N (>=4): ", 4, maxWin);

        for (int i = 0; i < g.nPlayers; i++)
            g.symbols[i] = SYMBOLS[i];

        g.turn = 0;
        g.board = board_create(g.rows, g.cols);
        board_init(g.board, g.rows, g.cols);
        hist_init(&g);

        game_play(&g);

        board_free(g.board, g.rows);
        free(g.hist);
    }

    printf("Adeus!\n");
    return 0;
}