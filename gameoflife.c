#define _DEFAULT_SOURCE

#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>

#define DELAY 50000
#define CELL_ON '#'
#define CELL_OFF ' '


void init();
void quit();
void setup_signals();
void allocate_buffers();
void swap_buffers();
void free_buffers();
void next_gen();

uint16_t rows, columns;
char **current_buffer, **next_buffer;

int main() {
  setup_signals();
  init();
  getmaxyx(stdscr, columns, rows);
  allocate_buffers();

  while (1) {
    clear();
    next_gen();
    swap_buffers();
    refresh();
    usleep(DELAY);
  }
}

void setup_signals() {
  uint8_t exit_signals[3] = { SIGINT, SIGTERM, SIGTSTP };
  for (size_t i = 0; i < 3; i++) {
    signal(exit_signals[i], quit);
  }
}

void init() {
  initscr();
  noecho();
  curs_set(0);
  use_default_colors();
  srand(time(NULL));
}

void quit() {
  endwin();
  free_buffers();
  exit(0);
}

void allocate_buffers() {
  current_buffer = (char **) malloc(rows * sizeof(char *));
  next_buffer = (char **) malloc(rows * sizeof(char *));
  for (uint16_t r = 0; r < rows; r++) {
    current_buffer[r] = (char *) malloc(columns * sizeof(char));
    next_buffer[r] = (char *) malloc(columns * sizeof(char));
    for (uint16_t c = 0; c < columns; c++)
      current_buffer[r][c] = rand()%3==0 ? CELL_ON : CELL_OFF;
  }
}

void swap_buffers() {
  for (uint16_t r = 0; r < rows; r++) {
    for (uint16_t c = 0; c < columns; c++)
      current_buffer[r][c] = next_buffer[r][c];
  }
}

void free_buffers() {
  for (uint16_t r = 0; r < rows; r++) {
    free(current_buffer[r]);
    free(next_buffer[r]);
  }
  free(current_buffer);
  free(next_buffer);
}

void next_gen() {
  for (uint16_t r = 0; r < rows; r++) {
    for (uint16_t c = 0; c < columns; c++) {
      uint8_t neighbors = 0;
      for (int8_t offset_r = -1; offset_r < 2; offset_r++) {
        for (int8_t offset_c = -1; offset_c < 2; offset_c++) {
          if (offset_r != 0 || offset_c != 0) {
            int16_t x = r+offset_r;
            int16_t y = c+offset_c;
            neighbors += (0 <= x && x < rows && 0 <= y && y < columns && current_buffer[x][y] == CELL_ON) ? 1 : 0;
          }
        }
      }
      char next_cell;
      if (current_buffer[r][c] == CELL_ON) {
        next_cell = (neighbors < 2 || neighbors > 3) ? CELL_OFF : CELL_ON;
      } else {
        next_cell = neighbors == 3 ? CELL_ON : CELL_OFF;
      }
      mvprintw(c, r, "%c", next_cell);
      next_buffer[r][c] = next_cell;
    }
  }
}

