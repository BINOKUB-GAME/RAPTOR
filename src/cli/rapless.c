/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rapless.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Téléscripteur et visualiseur de secours autonome (Pager).
 *                   Fournit une pagination interactive stricte (page par page,
 *                   ligne par ligne) pour l'examen de journaux ou de flux pipés.
 *                   Fonctionne sous contrainte de famine mémoire (< 2 Mo RAM),
 *                   sans aucune allocation dynamique sur le tas (0 heap).
 *                   Lit les commandes de contrôle directement sur /dev/tty afin
 *                   de préserver l'interactivité même lorsque stdin est saturé.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 17 (Fonctions 101 à 106)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   101. rapless_init_tty
 *   102. rapless_restore_tty
 *   103. rapless_get_term_size
 *   104. rapless_read_key
 *   105. rapless_render_page
 *   106. rapless_loop
 * ============================================================================
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define RAPLESS_BUFFER_SIZE  (1024 * 1024)  /* 1 Mo tampon statique maximal */
#define RAPLESS_MAX_LINES    32768          /* Indexation jusqu'à 32 768 lignes */

/* Structures et tampons statiques d'urgence (0 malloc) */
static char           g_buffer[RAPLESS_BUFFER_SIZE];
static size_t         g_line_offsets[RAPLESS_MAX_LINES];
static size_t         g_total_lines = 0;

static struct termios g_orig_termios;
static int            g_tty_fd = -1;
static int            g_termios_saved = 0;

/* Prototypes conformes au dictionnaire officiel */
void rapless_init_tty(void);
void rapless_restore_tty(void);
void rapless_get_term_size(int *out_rows, int *out_cols);
char rapless_read_key(void);
void rapless_render_page(size_t current_line, int rows, int cols);
void rapless_loop(void);

/* Gestionnaire de signal d'interruption pour restaurer le terminal */
static void rapless_signal_handler(int sig)
{
    rapless_restore_tty();
    _exit(128 + sig);
}

/* ============================================================================
 * FONCTION 102 : rapless_restore_tty
 * ----------------------------------------------------------------------------
 * Restaure les drapeaux d'origine du terminal (mode canonique et écho).
 * Enregistrée via atexit() et appelée en cas d'interruption par signal.
 * ============================================================================
 */
void rapless_restore_tty(void)
{
    if (g_termios_saved && g_tty_fd >= 0) {
        tcsetattr(g_tty_fd, TCSANOW, &g_orig_termios);
        g_termios_saved = 0;
    }
}

/* ============================================================================
 * FONCTION 101 : rapless_init_tty
 * ----------------------------------------------------------------------------
 * Ouvre /dev/tty pour isoler le flux de contrôle du clavier et bascule
 * le terminal en mode brut non-canonique et sans écho.
 * ============================================================================
 */
void rapless_init_tty(void)
{
    g_tty_fd = open("/dev/tty", O_RDWR);
    if (g_tty_fd < 0) {
        g_tty_fd = STDIN_FILENO;
    }

    if (tcgetattr(g_tty_fd, &g_orig_termios) == 0) {
        g_termios_saved = 1;
        atexit(rapless_restore_tty);

        signal(SIGINT, rapless_signal_handler);
        signal(SIGTERM, rapless_signal_handler);

        struct termios raw = g_orig_termios;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(g_tty_fd, TCSAFLUSH, &raw);
    }
}

/* ============================================================================
 * FONCTION 103 : rapless_get_term_size
 * ----------------------------------------------------------------------------
 * Interroge les dimensions réelles de l'écran via ioctl (TIOCGWINSZ).
 * En cas d'échec ou d'exécution sur port série restreint, repli sur 80x24.
 * ============================================================================
 */
void rapless_get_term_size(int *out_rows, int *out_cols)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) {
        *out_rows = (int)ws.ws_row;
        *out_cols = (int)ws.ws_col;
    } else {
        *out_rows = 24;
        *out_cols = 80;
    }
}

/* ============================================================================
 * FONCTION 104 : rapless_read_key
 * ----------------------------------------------------------------------------
 * Effectue une lecture unitaire et bloquante d'un caractère sur /dev/tty.
 * ============================================================================
 */
char rapless_read_key(void)
{
    char c = 0;
    if (g_tty_fd >= 0) {
        ssize_t n = read(g_tty_fd, &c, 1);
        if (n <= 0) {
            return 0;
        }
    }
    return c;
}

/* ============================================================================
 * FONCTION 105 : rapless_render_page
 * ----------------------------------------------------------------------------
 * Affiche la fenêtre de texte pour la ligne de départ donnée et projette
 * la barre de navigation inversée en bas d'écran.
 * ============================================================================
 */
void rapless_render_page(size_t current_line, int rows, int cols)
{
    (void)cols;
    int display_lines = (rows > 2) ? (rows - 1) : 1;

    /* Repositionnement du curseur en haut à gauche et effacement complet */
    printf("\033[H\033[2J");

    for (int i = 0; i < display_lines; i++) {
        size_t line_idx = current_line + (size_t)i;
        if (line_idx < g_total_lines) {
            printf("%s\n", &g_buffer[g_line_offsets[line_idx]]);
        } else {
            printf("~\n");
        }
    }

    /* Barre d'état en vidéo inversée sur la dernière ligne du terminal */
    printf("\033[7m-- RAPLESS [Ligne %zu/%zu] - [Espace: Page, Entrée: Ligne, b: Retour, q: Quitter] --\033[0m",
           (g_total_lines > 0) ? (current_line + 1) : 0,
           g_total_lines);

    fflush(stdout);
}

/* ============================================================================
 * FONCTION 106 : rapless_loop
 * ----------------------------------------------------------------------------
 * Boucle événementielle de consultation : capture les touches et ajuste
 * le pointeur de ligne courant.
 * ============================================================================
 */
void rapless_loop(void)
{
    int rows = 24;
    int cols = 80;
    size_t current_line = 0;

    while (1) {
        rapless_get_term_size(&rows, &cols);
        size_t page_step = (rows > 2) ? (size_t)(rows - 2) : 1;

        rapless_render_page(current_line, rows, cols);

        char key = rapless_read_key();
        if (key == 'q' || key == 'Q' || key == 27) {
            break;
        } else if (key == ' ') {
            if (current_line + page_step < g_total_lines) {
                current_line += page_step;
            } else if (g_total_lines > page_step) {
                current_line = g_total_lines - page_step;
            }
        } else if (key == '\n' || key == '\r') {
            if (current_line + 1 < g_total_lines) {
                current_line++;
            }
        } else if (key == 'b' || key == 'B') {
            if (current_line >= page_step) {
                current_line -= page_step;
            } else {
                current_line = 0;
            }
        }
    }

    /* Effacement propre de la barre d'état et saut de ligne terminal */
    printf("\033[2K\r");
    fflush(stdout);
}

/* ----------------------------------------------------------------------------
 * CHARGEMENT DU FLUX DE DONNÉES (Fichier ou stdin)
 * ----------------------------------------------------------------------------
 */
static size_t rapless_load_data(int input_fd)
{
    size_t total_bytes = 0;

    while (total_bytes < RAPLESS_BUFFER_SIZE - 1) {
        ssize_t n = read(input_fd, g_buffer + total_bytes, RAPLESS_BUFFER_SIZE - 1 - total_bytes);
        if (n <= 0) {
            break;
        }
        total_bytes += (size_t)n;
    }
    g_buffer[total_bytes] = '\0';

    g_total_lines = 0;
    if (total_bytes == 0) {
        g_line_offsets[0] = 0;
        g_total_lines = 1;
        return 0;
    }

    g_line_offsets[g_total_lines++] = 0;
    for (size_t i = 0; i < total_bytes; i++) {
        if (g_buffer[i] == '\r') {
            g_buffer[i] = '\0';
            if (i + 1 < total_bytes && g_buffer[i + 1] == '\n') {
                g_buffer[i + 1] = '\0';
                i++;
            }
            if (i + 1 < total_bytes && g_total_lines < RAPLESS_MAX_LINES) {
                g_line_offsets[g_total_lines++] = i + 1;
            }
        } else if (g_buffer[i] == '\n') {
            g_buffer[i] = '\0';
            if (i + 1 < total_bytes && g_total_lines < RAPLESS_MAX_LINES) {
                g_line_offsets[g_total_lines++] = i + 1;
            }
        }
    }

    return total_bytes;
}

/* ----------------------------------------------------------------------------
 * POINT D'ENTRÉE PRINCIPAL
 * ----------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
    int input_fd = -1;

    if (argc > 1) {
        input_fd = open(argv[1], O_RDONLY);
        if (input_fd < 0) {
            perror("rapless: ouverture fichier impossible");
            return 1;
        }
    } else {
        if (isatty(STDIN_FILENO)) {
            fprintf(stderr, "Usage : rapless [fichier]  OU  commande | rapless\n");
            return 1;
        }
        input_fd = STDIN_FILENO;
    }

    size_t bytes_read = rapless_load_data(input_fd);

    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }

    /* Si stdout est redirigé (pipe ou fichier), agir en simple relais cat */
    if (!isatty(STDOUT_FILENO)) {
        for (size_t i = 0; i < g_total_lines; i++) {
            printf("%s\n", &g_buffer[g_line_offsets[i]]);
        }
        return 0;
    }

    if (bytes_read == 0 && g_total_lines <= 1 && g_buffer[0] == '\0') {
        return 0;
    }

    rapless_init_tty();
    rapless_loop();
    rapless_restore_tty();

    return 0;
}
