#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Constantes do jogo
#define MAX_WIDTH 40
#define MAX_HEIGHT 40
#define MAX_LIVES 3

// Estrutura para controle do jogo
typedef struct {
    int active;
    int level;
    int lives;
    int has_key;
} GameState;

// Variáveis de estado do jogo
int player_x, player_y;
int monster_x_1, monster_y_1;
int monster_x_2, monster_y_2;
int teleport_1x, teleport_1y;
int teleport_2x, teleport_2y;
char monster1_tile = ' ';
char monster2_tile = ' ';

// Protótipos de funções
void init_game(GameState *game);
void show_title();
void render_map(char map[MAX_HEIGHT][MAX_WIDTH], int width, int height, GameState *game);
void handle_movement(char map[MAX_HEIGHT][MAX_WIDTH], char input, int width, int height, GameState *game);
void npc_interaction(char map[MAX_HEIGHT][MAX_WIDTH], int width, int height, GameState *game);
void move_monsters(char map[MAX_HEIGHT][MAX_WIDTH], int width, int height, GameState *game);
void player_die(GameState *game);
void player_wins(GameState *game);
void load_level(char map[MAX_HEIGHT][MAX_WIDTH], int *width, int *height, GameState *game);
void game_loop(GameState *game);

// Funções do terminal
void initTermios(int echo);
void resetTermios();
char getch_(int echo);
char getch();
char getche();
void sleep_ms(int milliseconds);

void init_game(GameState *game) {
    game->active = 1;
    game->lives = MAX_LIVES;
    game->has_key = 0;
    game->level = 0;
}

void player_wins(GameState *game) {
    
    system("clear");
    printf("\n\n");
    printf("  +-----------------------------------------------------+\n");
    printf("  |                                                     |\n");
    printf("  |                   VOCÊ VENCEU!                      |\n");
    printf("  |                                                     |\n");
    printf("  |  Parabéns! Você completou todas as dungeons e       |\n");
    printf("  |  recuperou a chave perdida!                         |\n");
    printf("  |                                                     |\n");
    printf("  |  Agora você pode finalmente entregar seu projeto    |\n");
    printf("  |  no CESUPA e descansar em paz!                      |\n");
    printf("  |                                                     |\n");
    printf("  +-----------------------------------------------------+\n");
    printf("\n\nPressione qualquer tecla para voltar ao menu...\n");
    getch();
    game->active = 0;
    return;
}

void player_die(GameState *game) {
    game->lives--;
    if (game->lives <= 0) {
        printf("\n\nSE LASCOU! Você perdeu TODAS as suas vidas. Voltando ao menu...\n");
        sleep_ms(3000);
        game->active = 0;
    } else {
        printf("\n\nVocê morreu! Vidas restantes: %d\n", game->lives);
        sleep_ms(1500);
        game->has_key = 0;
        game_loop(game); // Reinicia o nível atual
    }
}

void show_title() {
    system("clear");
    printf("\033[1m\n");
    printf("██████╗ ██╗  ██╗ █████╗ ███╗   ██╗████████╗ ██████╗ ███╗   ███╗\n");
    printf("██╔══██╗██║  ██║██╔══██╗████╗  ██║╚══██╔══╝██╔═══██╗████╗ ████║\n");
    printf("██████╔╝███████║███████║██╔██╗ ██║   ██║   ██║   ██║██╔████╔██║\n");
    printf("██╔═══╝ ██╔══██║██╔══██║██║╚██╗██║   ██║   ██║   ██║██║╚██╔╝██║\n");
    printf("██║     ██║  ██║██║  ██║██║ ╚████║   ██║   ╚██████╔╝██║ ╚═╝ ██║\n");
    printf("╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝\n");
    printf("\033[0m\n");
}

void render_map(char map[MAX_HEIGHT][MAX_WIDTH], int width, int height, GameState *game) {
    printf("Vidas: %d\n", game->lives);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
}

void handle_movement(char map[MAX_HEIGHT][MAX_WIDTH], char input, int width, int height, GameState *game) {
    int new_x = player_x, new_y = player_y;

    switch (input) {
        case 'w': case 'W': new_y--; break;
        case 's': case 'S': new_y++; break;
        case 'a': case 'A': new_x--; break;
        case 'd': case 'D': new_x++; break;
        case 'i': case 'I': 
            npc_interaction(map, width, height, game);
            return;
        default: return;
    }

    // Verifica limites do mapa
    if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height) return;

    char tile = map[new_y][new_x];

    // Verifica colisões
    if (tile == '*' || tile == 'P' || (tile == 'D' && !game->has_key) || (tile == '=' && !game->has_key)) {
        return;
    }

    // Verifica interações especiais
    if (tile == '@') {
        game->has_key = 1;
        // Abre todas as portas
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (map[i][j] == 'D') map[i][j] = '=';
            }
        }
    }
    else if (tile == '=' && game->has_key) {
        if (game->level == 3) {  // Último nível - vitória!
            player_wins(game);
        }
        
        else{
        system("clear");
        printf("\nvocê usou a chave e entrou no proximo level!\n");
        sleep_ms(1500);
        game->has_key = 0;
        game->level++;
        game_loop(game);
        return;
        }
    }
    else if (tile == 'O') {
        printf("\nvocê apertou o botão!\n");
        sleep_ms(1000);
        map[5][5] = ' ';
    }
    else if (tile == '#') {
        printf("\nvocê pisou nos espinho da masmorra e morreu! Reiniciando fase...\n");
        player_die(game);
        return;
    }

    // Atualiza posição do jogador
    player_x = new_x;
    player_y = new_y;

    // Verifica teleportes
    if (map[player_y][player_x] == '>') {
        if (player_x == teleport_1x && player_y == teleport_1y) {
            player_x = teleport_2x;
            player_y = teleport_2y;
        } else {
            player_x = teleport_1x;
            player_y = teleport_1y;
        }
        printf("eiiiiiitaaaaa voce se teleportou\n");
        sleep_ms(500);
    }
}

void move_monsters(char map[MAX_HEIGHT][MAX_WIDTH], int width, int height, GameState *game) {
    // Movimento do monstro aleatório (X)
    if (monster_x_1 != -1) {
        int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        int move = rand() % 4;
        int new_x = monster_x_1 + directions[move][0];
        int new_y = monster_y_1 + directions[move][1];

        if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) {
            char tile = map[new_y][new_x];
            if (tile == ' ' || tile == '&') {
                map[monster_y_1][monster_x_1] = monster1_tile;
                monster1_tile = tile;
                monster_x_1 = new_x;
                monster_y_1 = new_y;
                map[monster_y_1][monster_x_1] = 'X';

                if (monster_x_1 == player_x && monster_y_1 == player_y) {
                    printf("\no monstro da masmorra lhe pegou\n");
                    player_die(game);
                }
            }
        }
    }

    // Movimento do monstro inteligente (V)
    if (monster_x_2 != -1) {
        int dx = (player_x > monster_x_2) ? 1 : (player_x < monster_x_2) ? -1 : 0;
        int dy = (player_y > monster_y_2) ? 1 : (player_y < monster_y_2) ? -1 : 0;

        int new_x = monster_x_2 + dx;
        int new_y = monster_y_2 + dy;

        if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) {
            char tile = map[new_y][new_x];
            if (tile == ' ' || tile == '&') {
                map[monster_y_2][monster_x_2] = monster2_tile;
                monster2_tile = tile;
                monster_x_2 = new_x;
                monster_y_2 = new_y;
                map[monster_y_2][monster_x_2] = 'V';

                if (monster_x_2 == player_x && monster_y_2 == player_y) {
                    printf("\no monstro lendario pegou voce, talvez voce nao seja um bom aventureiro!\n");
                    player_die(game);
                }
            }
        }
    }
}

void npc_interaction(char map[MAX_HEIGHT][MAX_WIDTH], int width, int height, GameState *game) {
    int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    
    for (int i = 0; i < 4; i++) {
        int nx = player_x + directions[i][0];
        int ny = player_y + directions[i][1];
        
        if (nx >= 0 && nx < width && ny >= 0 && ny < height && map[ny][nx] == 'P') {
            system("clear");
            render_map(map, width, height, game);
            printf("\nVocê disse oi para o NPC(o aventureiro):\n\"O dia esta lindo!!, mas parece que eu esqueci a chave atras da minha casa.\"\n");
            sleep_ms(1500);
            return;
        }
    }

    system("clear");
    render_map(map, width, height, game);
    printf("\nnão há nenhum NPC por perto.\n");
    sleep_ms(1000);
}

void load_level(char map[MAX_HEIGHT][MAX_WIDTH], int *width, int *height, GameState *game) {
    // Inicializa variáveis de monstro e teleporte
    monster_x_1 = monster_y_1 = -1;
    monster_x_2 = monster_y_2 = -1;
    teleport_1x = teleport_1y = -1;
    teleport_2x = teleport_2y = -1;
    monster1_tile = ' ';
    monster2_tile = ' ';

    // Define o layout baseado no nível atual
    if (game->level == 0) { // Vila tutorial
        *width = 33;
        *height = 10;
        char layout[10][33] = {
            "**********D*********              ",
            "*&         P       *              ",
            "*   **             *              ",
            "*                  *              ",
            "*             **   *              ",
            "*  **              *              ",
            "*                  *              ",
            "*            **    *              ",
            "*    **       @    *              ",
            "********************              "
        };
        
        for (int i = 0; i < *height; i++) {
            for (int j = 0; j < *width; j++) {
                map[i][j] = layout[i][j];
                if (map[i][j] == '&') { player_x = j; player_y = i; }
            }
        }
    }
    else if (game->level == 1) { // Dungeon 1
        *width = 20;
        *height = 10;
        char layout[10][20] = {
            "********************",
            "D          ****    *",
            "*   *****          *",
            "*****      *  ******",
            "*   *  *****       *",
            "*   *      ******  *",
            "*          *      &*",
            "*   ****************",
            "*               @***",
            "********************"
        };
        
        for (int i = 0; i < *height; i++) {
            for (int j = 0; j < *width; j++) {
                map[i][j] = layout[i][j];
                if (map[i][j] == '&') { player_x = j; player_y = i; }
            }
        }
    }
    else if (game->level == 2) { // Dungeon 2
        *width = 20;
        *height = 20;
        char layout[20][20] = {
            "********************",
            "*                  *",
            "*  *************   *",
            "*              *   *",
            "*   *****      *   *",
            "*   *          * @ *",
            "*****  #*****  *****",
            "*            *     *",
            "*   *****    *******",
            "*   *##O*          *",
            "*   *   *    ###   *",
            "*       *    ###   *",
            "*   *   *         >*",
            "*   *   ************",
            "*   *              *",
            "D   *   *          *",
            "****************   *",
            "*&                 *",
            "*       X         >*",
            "********************"
        };
        
        for (int i = 0; i < *height; i++) {
            for (int j = 0; j < *width; j++) {
                map[i][j] = layout[i][j];
                if (map[i][j] == '&') { player_x = j; player_y = i; }
                else if (map[i][j] == 'X') { monster_x_1 = j; monster_y_1 = i; monster1_tile = ' '; }
                else if (map[i][j] == '>') {
                    if (teleport_1x == -1) { teleport_1x = j; teleport_1y = i; }
                    else { teleport_2x = j; teleport_2y = i; }
                }
            }
        }
    }
    else if (game->level == 3) { // Dungeon final
        *width = 40;
        *height = 40;
        char layout[40][40] = {
            "***************************************",
            "*                                     *",
            "*                      *   ********   *",
            "*   **************     *          *   *",
            "*   *            *     *          *   *",
            "*       **********     *          *   *",
            "*   *   * @    *       *          *   *",
            "*****   ****   ***** * *******    *   *",
            "*   *                *            *   *",
            "*   ******************   **********   *",
            "*            *    *               *****",
            "*   ***    ***    ******    ***********",
            "*****                                 *",
            "*   *   ****************************  *",
            "*   *                              *  *",
            "*   ********************************  *",
            "*   *    X                            *",
            "********************    ***************",
            "*                                     *",
            "******  *******************************",
            "*                                     *",
            "************************************  *",
            "*                                     *",
            "*   *******************************   *",
            "*   *                             *   *",
            "*   *   *******************   *   *   *",
            "*   *   *                     *   *   *",
            "*   *   *   ***************   *   *   *",
            "*   *   *   *             *   *   *   *",
            "*   *****                 *   *   *   *",
            "*   *   **********   ******   *   *   *",
            "*   *   *   *             *   *   *   D",
            "*   *   *                 *   *   *****",
            "*   *   *   *             *   *   *   *",
            "*       *   *             *   *       *",
            "*   *       *             *   *   *   *",
            "***********************************   *",
            "*                       &             *",
            "*                    V                *",
            "***************************************"
        };
        
        for (int i = 0; i < *height; i++) {
            for (int j = 0; j < *width; j++) {
                map[i][j] = layout[i][j];
                if (map[i][j] == '&') { player_x = j; player_y = i; }
                else if (map[i][j] == 'X') { monster_x_1 = j; monster_y_1 = i; monster1_tile = ' '; }
                else if (map[i][j] == 'V') { monster_x_2 = j; monster_y_2 = i; monster2_tile = ' '; }
            }
        }
    }
}

void game_loop(GameState *game) {
    char map[MAX_HEIGHT][MAX_WIDTH];
    int width, height;
    
    load_level(map, &width, &height, game);
    
    while (game->active) {
        system("clear");
        
        // Desenha jogador e monstros
        map[player_y][player_x] = '&';
        if (monster_x_1 != -1) map[monster_y_1][monster_x_1] = 'X';
        if (monster_x_2 != -1) map[monster_y_2][monster_x_2] = 'V';
        
        render_map(map, width, height, game);
        
        // Lê entrada do jogador
        char input = getch();
        
        // Remove jogador e monstros da posição anterior
        map[player_y][player_x] = ' ';
        if (monster_x_1 != -1) map[monster_y_1][monster_x_1] = monster1_tile;
        if (monster_x_2 != -1) map[monster_y_2][monster_x_2] = monster2_tile;
        
        // Processa movimento do jogador
        handle_movement(map, input, width, height, game);
        
        // Move monstros se o jogo ainda estiver ativo
        if (game->active) {
            move_monsters(map, width, height, game);
        }
    }
}

// compensar funcao getch da CONIO.h
/* Initialize new terminal i/o settings */
static struct termios old, current;

void initTermios(int echo) {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    current = old; /* make new settings same as old settings */
    current.c_lflag &= ~ICANON; /* disable buffered i/o */
    if (echo) {
        current.c_lflag |= ECHO; /* set echo mode */
    } else {
        current.c_lflag &= ~ECHO; /* set no echo mode */
    }
    tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios() {
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch() {
    return getch_(0);
}

/* Read 1 character with echo */
char getche() {
    return getch_(1);
}

/* Sleep function for Mac */
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

int main() {
    srand(time(NULL));
    
    int menu_loop = 1;
    GameState game;
    
    do {
        init_game(&game);
        show_title();
        
        printf("  1 - INICIAR\n");
        printf("  2 - CONTROLES\n");
        printf("  3 - CREDITOS\n");
        printf("  4 - SAIR\n");
        printf("  Escolha uma opcao: ");
        
        int selection;
        scanf("%d", &selection);
        while (getchar() != '\n'); // Limpa o buffer de entrada

        switch (selection) {
            case 1: 
                game_loop(&game);
                break;
            case 2:
                show_title();
                printf("  W se move para cima | S se move para baixo | A se move para esquerda | D se move para direita\n");
                printf("  I conversar com um NPC\n");
                printf("  Pressione qualquer tecla para continuar...");
                getch();
                break;
            case 3:
                show_title();
                printf("  {CREDITOS}\n\n");
                printf("  ||Gabriel Augusto Silva Pinho Pereira||\n");
                printf("  Pressione qualquer tecla para continuar...");
                getch();
                break;
            case 4:
                printf("\nate mais!!\n");
                menu_loop = 0;
                break;
            default:
                printf("\nopcao invalida.\n");
                printf("  Pressione qualquer tecla para continuar...");
                getch();
        }
    } while (menu_loop);

    return 0;
}
