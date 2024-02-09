#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  // TODO: Implement this function.
  game_state_t *game_state = malloc(sizeof(game_state_t));
  if (game_state == NULL) {
      return NULL;
  }
  game_state->num_rows = 18;
  game_state->num_snakes = 1;
  game_state->snakes = malloc(sizeof(snake_t) * game_state->num_snakes);

  game_state->board = malloc(sizeof(char*) * game_state->num_rows);

  if (game_state->board == NULL) {
      free(game_state);
      return NULL;
  }

  for(unsigned int i = 0; i < game_state->num_rows; i++) {
      game_state->board[i] = malloc(21);
      if (game_state->board[i] == NULL) {
          for (unsigned int j = 0; j < i; j++) {
              free(game_state->board[j]);
          }
          free(game_state->board);
          free(game_state);
          return NULL;
      }
      memset(game_state->board[i], ' ', 20);
      game_state->board[i][20] = '\0';
    
  }


  // for the walls cuh
  for (unsigned int i = 0; i < game_state->num_rows; i++) {
      game_state->board[i][0] = '#';
      game_state->board[i][19] = '#';
  }
  for (unsigned int j = 0; j < 20; j++) {
      game_state->board[0][j] = '#';
      game_state->board[game_state->num_rows - 1][j] = '#';
  }
  strcpy(game_state->board[2], "# d>D    *         #");

  game_state->snakes[0].tail_row = 2;
  game_state->snakes[0].tail_col = 2;
  game_state->snakes[0].head_row = 2;
  game_state->snakes[0].head_col = 4;
  game_state->snakes[0].live = true;

  


  return game_state;
}

/* Task 2 */
void free_state(game_state_t *state) {
  // TODO: Implement this function.
  if (state == NULL) {
      return;
  }

  for (int i = 0; i < state->num_rows; i++) {
      free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);

  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  // TODO: Implement this function.
  if (state == NULL || fp == NULL) {
      return;
  }
  for (unsigned int i = 0; i < state->num_rows; i++) {
      fprintf(fp, "%s\n", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
      return true;
  }
  return false; 
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
      return true;
  }
  return false; 
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  if (is_head(c) || is_tail(c) || c == '^' || c == '<' || c == 'v' || c == '>') {
      return true;
  }
  return false; 
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  if (c == '^') return 'w';
  else if (c == '<') return 'a';
  else if (c == 'v') return 's';
  else if (c == '>') return 'd';
  else return '\0';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  if (c == 'W') return '^';
  else if (c == 'A') return '<';
  else if (c == 'S') return 'v';
  else if (c == 'D') return '>';
  else return '\0';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  if (c == 'v' || c == 's' || c == 'S') {
      return cur_row + 1;
  } else if (c == '^' || c == 'w' || c == 'W') {
      return cur_row - 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if (c == '>' || c == 'd' || c == 'D') {
      return cur_col + 1;
  } else if (c == '<' || c == 'a' || c == 'A') {
      return cur_col - 1;
  }
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int head_row = state->snakes[snum].head_row;
  unsigned int head_col = state->snakes[snum].head_col;
  char head_char = get_board_at(state, head_row, head_col);

  unsigned int next_row = head_row;
  unsigned int next_col = head_col;

  switch (head_char) {
      case 'W':
          next_row = get_next_row(head_row, '^');
          break;
      case 'A':
          next_col = get_next_col(head_col, '<');
          break;
      case 'S':
          next_row = get_next_row(head_row, 'v');
          break;
      case 'D': 
          next_col = get_next_col(head_col, '>');
          break;
      default:
          return ' ';
  }
  return get_board_at(state, next_row, next_col);
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cur_head_row = state->snakes[snum].head_row;
  unsigned int cur_head_col = state->snakes[snum].head_col;
  char head_char = get_board_at(state, cur_head_row, cur_head_col);

  unsigned int new_head_row = get_next_row(cur_head_row, head_char); 
  unsigned int new_head_col = get_next_col(cur_head_col, head_char);

  set_board_at(state, cur_head_row, cur_head_col, head_to_body(head_char));
  set_board_at(state, new_head_row, new_head_col, head_char);

  state->snakes[snum].head_row = new_head_row;
  state->snakes[snum].head_col = new_head_col;

  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  // this gets the tails row and tail column, so that we can blank it out
  unsigned int tail_row = state->snakes[snum].tail_row;
  unsigned int tail_col = state->snakes[snum].tail_col;
  char tail_char = get_board_at(state, tail_row, tail_col);
  set_board_at(state, tail_row, tail_col, ' ');

  unsigned int new_tail_row = get_next_row(tail_row, tail_char);
  unsigned int new_tail_col = get_next_col(tail_col, tail_char);
  char to_tell = get_board_at(state, new_tail_row, new_tail_col);
  set_board_at(state, new_tail_row, new_tail_col, body_to_tail(to_tell));
  

  state->snakes[snum].tail_row = new_tail_row;
  state->snakes[snum].tail_col = new_tail_col;


  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  for (unsigned int snum = 0; snum < state->num_snakes; ++snum) {
      snake_t *snake = &(state->snakes[snum]);

      if (!snake->live) {
          continue;
      }
      char next_square_char = next_square(state, snum);
      if (next_square_char == '#') {
          set_board_at(state, snake->head_row, snake->head_col, 'x');
          snake->live = false;
      } else if (is_snake(next_square_char)) {
          set_board_at(state, snake->head_row, snake->head_col, 'x');
          snake->live = false; 
      } else if (next_square_char == '*') {
          update_head(state, snum);
          add_food(state);
      } else {
          update_head(state, snum);
          update_tail(state, snum);
      }
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  // TODO: Implement this function.
  if (!fp) return NULL;
  
  char* buffer = malloc(1024);
  if (!buffer) return NULL;

  if (fgets(buffer, 1024, fp) == NULL) {
      free(buffer);
      return NULL;
  }
  
  size_t len = strlen(buffer);
  char *line = realloc(buffer, len + 1);
  if (line == NULL) {
      free(buffer);
      return NULL;
  }

  return line;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  if (fp == NULL) return NULL;

  game_state_t *state = malloc(sizeof(game_state_t));
  if (state == NULL) {
      return NULL;
  }
  state->num_rows = 0;
  state->board = NULL;

  char *line;
  while ((line = read_line(fp)) != NULL) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }
    state->board = realloc(state->board, sizeof(char *) * (state->num_rows + 1));
    if (!state->board) {
        return NULL;
    }
    state->board[state->num_rows] = line;
    state->num_rows++;
  }

  state-> num_snakes = 0;
  state->snakes = NULL;

  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  snake_t *snake = &(state->snakes[snum]);
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;
  char curr_char = get_board_at(state, row, col);

  while (!is_head(curr_char)) {
      row = get_next_row(row, curr_char);
      col = get_next_col(col, curr_char);
      curr_char = get_board_at(state, row, col);
  }
  snake->head_row = row;
  snake->head_col = col;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  if (state == NULL) {
      return NULL;
  }

  state->snakes = (snake_t *)malloc(sizeof(snake_t));
  state->num_snakes = 0;
  if (state->snakes == NULL) {
      return state;
  }
    

  for (unsigned int row = 0; row < state->num_rows; row++) {
      for (unsigned int col = 0; col < strlen(state->board[row]); col++) {
          char cell = state->board[row][col];
          if (is_tail(cell)) {
              snake_t *temp = (snake_t *)realloc(state->snakes, (state->num_snakes + 1) * sizeof(snake_t));
              if (temp == NULL) {
                  return state;
              }
              state->snakes = temp;
              state->snakes[state->num_snakes].tail_row = row;
              state->snakes[state->num_snakes].tail_col = col;
              state->snakes[state->num_snakes].live = true;
              find_head(state, state->num_snakes);
              state->num_snakes++;
          }
      }
  }
  return state;
}
