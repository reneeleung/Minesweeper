#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct posn {
  int x;
  int y;
};

struct ms_board {
  int width;
  int height;
  char *board;
  int num_mines;
  struct posn *mines;
};


const char UNREVEALED = ' ';
const char REVEALED[9] = "012345678";
const char FLAG = 'F';
const char MINE = '*';

bool flag(struct ms_board *b, int x, int y){
  assert(b);
  assert(x >= 1 && x <= b->width);
  assert(y >= 1 && y <= b->height);
  if (b->board[(y-1) * b->width + x - 1] == FLAG){
    b->board[(y-1) * b->width + x - 1] = UNREVEALED;
    return true;
  } else if (b->board[(y-1) * b->width + x - 1] == UNREVEALED){
    b->board[(y-1) * b->width + x - 1] = FLAG;
    return true;
  }
  return false;
}

static bool check_mine(int x, int y, int len, struct posn *mines){
  for (int j = 0; j < len; j++){
    if (mines[j].x == x && mines[j].y == y){
      return true;
    }
  }
  return false;
}

static bool check_valid(int x, int y, int height, int width){
  return x >= 1 && x <= width && y >= 1 && y <= height;
}

bool reveal(struct ms_board *b, int x, int y){
  assert(b);
  assert(x >= 1 && x <= b->width);
  assert(y >= 1 && y <= b->height);
  if (b->board[(y-1) * b->width + x - 1] != UNREVEALED){
    return false;
  }
  if (check_mine(x, y, b->num_mines, b->mines)){
    b->board[(y-1) * b->width + x - 1] = MINE;
  } else {
    int mine_count = 0;
    for (int i = x - 1; i <= x + 1; i++){
      for (int j = y - 1; j <= y + 1; j++){
        if (check_valid(i, j, b->height, b->width)){
          if (check_mine(i, j, b->num_mines, b->mines)){
          mine_count++;
          }
        }
      }
    }
    b->board[(y-1) * b->width + x - 1] = REVEALED[mine_count];
    if (mine_count == 0){
      for (int m = x - 1; m <= x + 1; m++){
        for (int n = y - 1; n <= y + 1; n++){
          if (check_valid(m, n, b->height, b->width) && (m != x || n != y)){
            if (b->board[(n-1) * b->width + m - 1] == UNREVEALED){
              reveal(b, m, n);
            }
          }
        }
      }
    }
  }
  return true;
}

bool game_won (const struct ms_board *b){
  assert(b);
  int reveal_count = 0;
  int mine_count = 0;
  int all_reveal = b->height * b->width - b->num_mines;
  for (int i = 0; i < b->height * b->width; i++){
    if (b->board[i] != UNREVEALED && b->board[i] != FLAG && b->board[i] != MINE){
      reveal_count++;
    } else if (b->board[i] == MINE){
      mine_count++;
    }
  }
  return mine_count == 0 && reveal_count == all_reveal;
}

bool game_lost(const struct ms_board *b){
  assert(b);
  for (int i = 0; i < b->height * b->width; i++){
    if (b->board[i] == MINE){
      return true;
    }
  }
  return false;
}

void print_board(const struct ms_board *b){
  printf("__|");
  for (int i = 0; i < b->width; i++){
    printf("_%c_", i + 'A');
  }
  printf("\n");
  for (int y = 1; y <= b->height; y++){
    if (y <= 9){
      printf(" ");
    }
    printf("%d|", y);
    for (int x = 1; x <= b->width; x++){
      printf(" %c ", b->board[(y-1) * b->width + x - 1]);
    }
    printf("\n");
  }
}

struct ms_board *create_board(int width, int height, int num_mines){
  struct ms_board *b = malloc(sizeof(struct ms_board));
  b->width = width;
  b->height = height;
  b->board = malloc(width * height * sizeof(char));
  for (int i = 0; i < width * height; i++){
    b->board[i] = UNREVEALED;
  }
  b->num_mines = num_mines;
  b->mines = malloc(num_mines * sizeof(struct posn));
  struct posn *all_posn = malloc(height * width * sizeof(struct posn));
  int mine_exist[num_mines];
  int existSoFar = 0;
  bool found = false;
  for (int j = 1; j <= height; j++){
    for (int i = 1; i <= width; i++){
      all_posn[(j-1) * b->width + i - 1].x = i;
      all_posn[(j-1) * b->width + i - 1].y = j;
    }
  }
  for (int i = 0; i < num_mines; i++){
    found = false;
    int random = rand()%(width * height);
    for (int j = 0; j < existSoFar; j++){
      if (random == mine_exist[j]){
        i--;
        found = true;
        break;
      }
    }
    if (found == false){
      b->mines[i] = all_posn[random];
      existSoFar++;
      mine_exist[existSoFar - 1] = random;
    }
  }
  free(all_posn);
  return b;
}

void destroy_board(struct ms_board *b){
  free(b->board);
  free(b->mines);
  free(b);
}

int main(void){
  printf("Welcome to Minesweeper!\n");
  while (1){
    int width = 0;
    int height = 0;
    int num_mines = 0;
    while (width < 4 || width > 16){
      printf("Enter width (4-16)\n");
      scanf("%d", &width);
    }
    while (height < 4 || height > 16){
      printf("Enter height (4-16)\n");
      scanf("%d", &height);
    }
    while (num_mines < 1 || num_mines > height * width){
      printf("Enter mine count (1-%d)\n", height * width);
      scanf("%d", &num_mines);
    }
    struct ms_board *b = create_board(width, height, num_mines);
    bool win = false;
    bool lose = false;
    while (win == false && lose == false){
      print_board(b);
      char command;
      printf("Enter 'f' to place/remove a flag or 'r' to reveal a tile\n");
      scanf("%c", &command);
      if (command == 'f'){
        printf("Enter coordinate (column then row, e.g. A1)\n");
        char coordinate[4];
        int co_x = 0;
        int co_y = 0;
        scanf("%s", coordinate);
        co_x = coordinate[0] - 'A' + 1;
        if (strlen(coordinate) == 3){
          co_y = (coordinate[1] - '1' + 1) * 10 + coordinate[2] - '1' + 1;
        } else if (strlen(coordinate) == 2){
          co_y = coordinate[1] - '1' + 1;
        }
        printf("Coordinate %s (%d,%d)\n", coordinate, co_x, co_y);
        if (flag(b, co_x, co_y) == false){
          printf("flag %s failed\n", coordinate);
        }
      } else if (command == 'r'){
        printf("Enter coordinate (column then row, e.g. A1)\n");
        char coordinate[4];
        int co_x = 0;
        int co_y = 0;
        scanf("%s", coordinate);
        co_x = coordinate[0] - 'A' + 1;
        if (strlen(coordinate) == 3){
          co_y = (coordinate[1] - '1' + 1) * 10 + coordinate[2] - '1' + 1;
        } else if (strlen(coordinate) == 2){
          co_y = coordinate[1] - '1' + 1;
        }
        printf("Coordinate %s (%d,%d)\n", coordinate, co_x, co_y);
        if (reveal(b, co_x, co_y) == false){
          printf("reveal %s failed\n", coordinate);
        }
      } else {
        printf("Invalid command\n");
      }
      if (game_won(b)){
        win = true;
      } else if (game_lost(b)){
        lose = true;
      }
    }
    print_board(b);
    if (win){
      printf("Congratulations, you won!\n");
      printf("_________\n");
      printf("/         \\\n");
      printf("|  /\\ /\\  |\n");
      printf("|    -    |\n");
      printf("|  \\___/  |\n");
      printf("\\_________/\n");
    } else if (lose){
      printf("Game over, you lose!\n");
      printf("_________\n");
      printf("/         \\\n");
      printf("|  X   X  |\n");
      printf("|    -    |\n");
      printf("|  _____  |\n");
      printf("\\_________/\n");
    }
    destroy_board(b);
    char playAgain = 0;
    while (playAgain != 'y' && playAgain != 'n'){
      printf("Play again? (y/n)\n");
      scanf("%c", &playAgain);
    }
    if (playAgain == 'n'){
      printf("See you later\n");
      break;
    }
  }
}
