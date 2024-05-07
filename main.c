#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
#include "images/start.h"
#include "images/game.h"
#include "images/garbage.h"
#include "images/smile.h"
#include "images/castle.h"
#include "images/smile2.h"
#include "images/background.h"
#include "images/broccoli.h"
#include "images/donut.h"
#include "images/lose.h"
#include "images/win.h"

int eatGood(struct player player, struct donut d);
int eatBad(struct player player, struct broccoli b);
void initialize(struct player *player, struct donut *d, struct broccoli *b, struct sprite *s);
void drawTimer(int time);
int randomChoice(int num1, int num2);

volatile int time = 31;
char timerString[10];
char winString[10];
int donuts = 0;
int f = 0;

enum gba_state {
  START_TRANSITION,
  START,
  PLAY,
  WIN,
  LOSE,
};

void initialize(struct player *player, struct donut *d, struct broccoli *b, struct sprite *s) {
  player->row = 15;
  player->col = 20;
  player->width = 25;
  player->height = 25;

  d->width = 15;
  d->height = 15;
  d->row = randint(42, (HEIGHT - d->height - 20));
  d->col = randint(47, (WIDTH - d->width - 20));
  d->velR = 1;
  d->velC = 1;

  b->width = 15;
  b->height = 15;
  b->row = randint(42, (HEIGHT - b->height - 20));
  b->col = randint(47, (WIDTH - b->width - 20));

  s->width = 15;
  s->height = 15;
  s->row = randint(110, (HEIGHT - s->height));
  s->col = randint(0, (WIDTH - s->width));
  s->velR = 2;
  s->velC = 2;

  vBlankCounter = 0;
  time = 31;
  donuts = 0;
  f = 0;
}

int main(void) {
  /* TODO: */
  REG_DISPCNT = MODE3 | BG2_ENABLE;

  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  struct player player;
  struct donut d;
  struct broccoli b;
  struct sprite s;

  initialize(&player, &d, &b, &s);
  
  enum gba_state state = START;

  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons
    
    if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
      state = START_TRANSITION;
    }

    switch (state) {
      case START_TRANSITION:
        state = START;
        break;
      case START:
        waitForVBlank();
        undrawImageDMA(s.row, s.col, s.width, s.height, start);
        if ((s.row + s.velR) < 110) {
          s.velR *= -1;
        }

        if ((s.row + s.velR + s.height) > HEIGHT) {
          s.velR *= -1;
        }
        
        if ((s.col + s.velC) < 0) {
          s.velC *= -1;
        }

        if ((s.col + s.velC + s.width) > WIDTH) {
          s.velC *= -1;
        }

        s.row += s.velR;
        s.col += s.velC;

        if (KEY_DOWN(BUTTON_START, currentButtons)) {
          state = PLAY;
          initialize(&player, &d, &b, &s);
          drawFullScreenImageDMA(game);
          break;
        }

        drawFullScreenImageDMA(start);
        drawImageDMA(s.row, s.col, s.width, s.height, smile2);
        break;
      case PLAY:
        f++;
        if (f == 60) {
          time--;
          sprintf(timerString, "Time: %d", time);
          waitForVBlank();
          drawRectDMA(147, 8, 53, 40, PEACH);
          drawString(150, 10, timerString, BLACK);
          f = 0;
          if (time == 0) {
            state = WIN;
            drawFullScreenImageDMA(win);
            break;
          }
        }
        
        waitForVBlank();
        //erasing player
        undrawImageDMA(player.row, player.col, player.width, player.height, game);

        //erasing donut
        undrawImageDMA(d.row, d.col, d.width, d.height, game);

        //erasing broccoli
        undrawImageDMA(b.row, b.col, b.width, b.height, game);

        if (KEY_DOWN(BUTTON_RIGHT, currentButtons)) {
          if ((player.col + player.width + 1) <= WIDTH) {
            player.col += 1;
          } else {
            player.col = WIDTH - player.width;
          }
        }
        if (KEY_DOWN(BUTTON_LEFT, currentButtons)) {
          if ((player.col - 1) >= 0) {
            player.col -= 1;
          } else {
            player.col = 0;
          }
        }
        if (KEY_DOWN(BUTTON_DOWN, currentButtons)) {
          if ((player.row + player.height + 1) <= (HEIGHT - 15)) {
            player.row += 1;
          } else {
            player.row = HEIGHT - player.height;
          }
        }
        if (KEY_DOWN(BUTTON_UP, currentButtons)) {
          if ((player.row - 1) >= 0) {
            player.row -= 1;
          } else {
            player.row = 0;
          }
        }

        if ((d.row + d.velR) < 20) {
          d.velR *= -1;
        }

        if ((d.row + d.velR + d.height) > (HEIGHT - 15)) {
          d.velR *= -1;
        }
        
        if ((d.col + d.velC) < 20) {
          d.velC *= -1;
        }

        if ((d.col + d.velC + d.width) > (WIDTH - 20)) {
          d.velC *= -1;
        }

        d.row += d.velR;
        d.col += d.velC;

        //regenerate moving donut if current one eaten
        if (eatGood(player, d)) {
          donuts++;
          undrawImageDMA(d.row, d.col, d.width, d.height, game);
          undrawImageDMA(b.row, b.col, b.width, b.height, game);

           do {
            int possibledR1 = randint(20, player.row - d.height - 2);
            int possibledR2 = randint(player.row + player.height + 2, HEIGHT - 20 - d.height);

            if ((player.row - d.height - 2) - 20 >= d.height && ((HEIGHT - 20 - d.height) - (player.row + player.height + 2)) >= d.height) {
              int choice = randomChoice(possibledR1, possibledR2);
              if (choice == possibledR2) {
                d.row = randint(player.row + player.height + 2, HEIGHT - 20 - d.height);
                d.col = randint(20, (WIDTH - d.width - 20));
              } else {
                d.row = randint(20, player.row - d.height - 2);
                d.col = randint(20, (WIDTH - d.width - 20));
              }
            } else if ((player.row - d.height - 2) - 20 < d.height) {
              d.row = randint(player.row + player.height + 2, HEIGHT - 20 - d.height);
              d.col = randint(20, (WIDTH - d.width - 20));
            } else {
              d.row = randint(20, player.row - d.height - 2);
              d.col = randint(20, (WIDTH - d.width - 20));
            }

            int possibleR1 = randint(20, player.row - b.height - 2);
            int possibleR2 = randint(player.row + player.height + 2, HEIGHT - 20 - b.height);
            
            if ((player.row - b.height - 2) - 20 >= b.height && ((HEIGHT - 20 - b.height) - (player.row + player.height + 2)) >= b.height) {
              int choice = randomChoice(possibleR1, possibleR2);
              if (choice == possibleR2) {
                b.row = randint(player.row + player.height + 2, HEIGHT - 20 - b.height);
                b.col = randint(20, (WIDTH - b.width - 20));
              } else {
                b.row = randint(20, player.row - b.height - 2);
                b.col = randint(20, (WIDTH - b.width - 20));
              }
            } else if ((player.row - b.height - 2) - 20 < b.height) {
              b.row = randint(player.row + player.height + 2, HEIGHT - 20 - b.height);
              b.col = randint(20, (WIDTH - b.width - 20));
            } else {
              b.row = randint(20, player.row - b.height - 2);
              b.col = randint(20, (WIDTH - b.width - 20));
            }
            
        } while (eatGood(player, d));
        }

        if (eatBad(player, b)) {
          state = LOSE;
          drawFullScreenImageDMA(lose);
          break;
        }

        //redrawing players
        drawImageDMA(player.row, player.col, player.width, player.height, smile);

        //redrawing donut
        drawImageDMA(d.row, d.col, d.width, d.height, donut);

        //redrawing broccoli
        drawImageDMA(b.row, b.col, b.width, b.height, broccoli);
        break;
      case LOSE:
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
          state = START_TRANSITION;
         }
        break;
      case WIN:
        sprintf(winString, "%d", donuts);
        drawString(94, 168, winString, PINK);
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
          state = START_TRANSITION;
         }
        break;
    }
    previousButtons = currentButtons; // Store the current state of the buttons
  }
  UNUSED(previousButtons);
  return 0;
}

int eatGood(struct player player, struct donut d) {
   return (player.col < (d.col + d.width) &&
          ((player.col + player.width) > d.col) &&
          (player.row < (d.row + d.height)) &&
          ((player.row + player.height) > d.row));
}

int eatBad(struct player player, struct broccoli b) {
   return (player.col < (b.col + b.width) &&
          ((player.col + player.width) > b.col) &&
          (player.row < (b.row + b.height)) &&
          ((player.row + player.height) > b.row));
}

int randomChoice(int num1, int num2) {
    // Generate a random number (0 or 1) to decide between possible broccoli positions
    int choice = rand() % 2;
    return (choice == 0) ? num1 : num2;
}