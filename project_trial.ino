#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <avr/sleep.h>

#define VRx A0  // X-axis of the joystick
#define VRy A1  // Y-axis of the joystick
#define BUTTON 2

#define NO_UNDOS 3

// Thresholds for detecting movement
#define DEADZONE 100  // Adjust this value to control sensitivity

// TFT Display Pins
#define TFT_CS   8
#define TFT_DC   10
#define TFT_RST  9

// Touchscreen Pins
#define TOUCH_CS 7
#define TOUCH_IRQ 3

// Screen size
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// Raw touch ranges — replace these with your calibrated values
#define RAW_MIN_X 300
#define RAW_MAX_X 3800
#define RAW_MIN_Y 400
#define RAW_MAX_Y 3800

#define BUTTON_X 250
#define BUTTON_Y 120
#define BUTTON_W 50
#define BUTTON_H 30

#define RETRY_X 160
#define RETRY_Y 120
#define RETRY_W 65
#define RETRY_H 25

#define WIN_VALUE 2048

// Swipe variables
bool isTouching = false;
TS_Point start, end;

#define SQUARE_SIZE 45

#define GRID_LEN 4

uint32_t score = 0;
uint32_t last_score[NO_UNDOS] = {0, 0, 0};
bool undo = false;
bool has_won = false;
bool win_screen = false;

uint16_t colors[12] = {
  0, 
  ILI9341_WHITE, // 2
  ILI9341_LIGHTGREY, // 4
  ILI9341_CYAN, // 8
  ILI9341_GREENYELLOW, // 16
  ILI9341_ORANGE, // 32
  ILI9341_RED, // 64
  ILI9341_PURPLE, // 128
  ILI9341_PINK, // 256
  ILI9341_MAGENTA, // 512
  ILI9341_YELLOW, // 1024
  ILI9341_OLIVE // 2048
};

class Grid {
  public:
    uint32_t grid[GRID_LEN][GRID_LEN];
    uint32_t max_val = 0;

    Grid();

    Grid(const Grid &otherGrid) {
      for (int i = 0; i < GRID_LEN; i++)
        for (int j = 0; j < GRID_LEN; j++)
          grid[i][j] = otherGrid.grid[i][j];
    }

    void draw();
    void swipeDown();
    void swipeUp();
    void swipeLeft();
    void swipeRight();
  
  private:
    void addRandomTile();
    void drawCenteredTextInBox(int rectX, int rectY, uint32_t value);
};

Grid::Grid() {
  randomSeed(analogRead(A0));

  for (int i = 0; i < GRID_LEN; i++)
    for (int j = 0; j < GRID_LEN; j++)
      grid[i][j] = 0;

  int x = random(0, GRID_LEN);
  int y = random(0, GRID_LEN);

  int chance = random(0, 100);

  if (chance < 80)
    grid[x][y] = 2;
  else
    grid[x][y] = 4;
  max_val = grid[x][y];
}

Grid grid, last_grids[NO_UNDOS];

uint16_t getColor(uint32_t value) {
  uint32_t comp = 2;
  for (int i = 1; i < 12; i++) {
    if (value & comp)
      return colors[i];
    comp = comp << 1;
  }

  return 0;
}

void Grid::drawCenteredTextInBox(int rectX, int rectY, uint32_t value) {
  tft.setTextSize(2);
  
  uint16_t color = getColor(value);
  tft.setTextColor(color);

  int16_t x1, y1;
  uint16_t wText, hText;
  char message[10];
  sprintf(message, "%d", value);

  tft.getTextBounds(message, 0, 0, &x1, &y1, &wText, &hText);

  int textX = rectX + (SQUARE_SIZE - wText) / 2;
  int textY = rectY + (SQUARE_SIZE - hText) / 2;

  tft.setCursor(textX, textY);
  tft.print(message);
}

void Grid::draw() {

  int x_curs = 40;
  int y_curs = 40;

  for (int i = 0; i < GRID_LEN; i++) {
    for (int j = 0; j < GRID_LEN; j++) {

      if (undo or grid[i][j] != last_grids[NO_UNDOS - 1].grid[i][j]) {
        tft.fillRect(x_curs, y_curs, SQUARE_SIZE, SQUARE_SIZE, ILI9341_BLACK);
      }
      
      tft.drawRect(x_curs, y_curs, SQUARE_SIZE, SQUARE_SIZE, ILI9341_WHITE);

      if (grid[i][j])
        this->drawCenteredTextInBox(x_curs, y_curs, grid[i][j]);
      y_curs += SQUARE_SIZE + 2;
      // Serial.print(grid[i][j]);
      // Serial.print(" ");
    } 
    // Serial.println();

    x_curs += SQUARE_SIZE + 2;
    y_curs = 40;
  }

  tft.fillRect(70, 8, 350, 15, ILI9341_BLACK);
  tft.setCursor(10, 8);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Score: ");
  tft.print(score);
}

void initialize() {
  tft.fillScreen(ILI9341_BLACK);

  grid.draw();

  tft.drawRect(250, 120, 50, 30, ILI9341_WHITE);
  tft.setCursor(253, 127);
  tft.setTextSize(2);
  tft.print("Undo");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(BUTTON, INPUT_PULLUP);

  tft.begin();
  ts.begin();

  tft.setRotation(3);
  ts.setRotation(3);

  initialize();
}

void handleUndo() {
  score = last_score[NO_UNDOS - 1];
  grid = last_grids[NO_UNDOS - 1];

  for (int i = NO_UNDOS - 1; i >= 1; i--) {
    last_score[i] = last_score[i - 1];
    last_grids[i] = last_grids[i - 1];
  }  

  undo = true;
  grid.draw();
  undo = false;
}

void saveState() {
  for (int i = 0; i < NO_UNDOS - 1; i++) {
    last_grids[i] = last_grids[i + 1];
    last_score[i] = last_score[i + 1];
  }

  last_grids[NO_UNDOS - 1] = grid;
  last_score[NO_UNDOS - 1] = score;
}

void loop() {
  /*if (has_won) {
    win_screen = true;
    has_won = false;

    tft.fillScreen(ILI9341_BLACK);

    tft.setTextSize(4);
    tft.setTextColor(ILI9341_WHITE);

    int16_t x1, y1;
    uint16_t wText, hText, wText2, hText2;
    char message_win[10];
    char message_score[10];
    sprintf(message_win, "YOU WON!");
    sprintf(message_score, "Score: %u", score);

    tft.getTextBounds(message_win, 0, 0, &x1, &y1, &wText, &hText);
    tft.getTextBounds(message_score, 0, 0, &x1, &y1, &wText2, &hText2);

    int textX = (SCREEN_WIDTH - wText) / 2;
    int textY = (SCREEN_HEIGHT - hText2 - hText) / 2;

    tft.setCursor(textX, textY);
    tft.print(message_win);

    int textX2 = (SCREEN_WIDTH - wText2) / 2;
    int textY2 =  textY + hText2 + 2;

    tft.setCursor(textX2, textY2);
    tft.print(message_score);

    tft.drawRect(120, 160, 65, 25, ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(123, 164);
    tft.print("Retry");
  }*/

  int xValue = analogRead(VRx);  // Read X-axis (left-right)
  int yValue = analogRead(VRy);  // Read Y-axis (up-down)
  bool buttonPressed = digitalRead(BUTTON);

  if (buttonPressed == LOW) {
    handleUndo();
    delay(300);
    return;
  }

  if (abs(xValue - 512) > DEADZONE or abs(yValue - 512) > DEADZONE) {
    saveState();

    // Left / Right detection
    if (xValue < 512 - DEADZONE) {
      grid.swipeDown();
    } else if (xValue > 512 + DEADZONE) {
      grid.swipeUp();
    }

    // Up / Down detection
    if (yValue < 512 - DEADZONE) {
      grid.swipeLeft();
    } else if (yValue > 512 + DEADZONE) {
      grid.swipeRight();
    }

    if (grid.max_val == WIN_VALUE) {
      has_won = true;
    }

    delay(300);
    return;
  }

  if (ts.touched()) {
    if (!isTouching) {
      // Touch started
      start = ts.getPoint();
      isTouching = true;
    }
  } else {
    if (isTouching) {
      // Touch ended
      end = ts.getPoint();
      isTouching = false;

      handleSwipe(start, end);
    }
  }
  delay(10);
}

void Grid::swipeDown() {
  for (int lin = 0; lin < GRID_LEN; lin++) {
    // gravity
    int col = GRID_LEN - 1;
    for (int y = GRID_LEN - 1; y >= 0; y--) {
      if (grid[lin][y]) {
        grid[lin][col] = grid[lin][y];
        col--;
      }
    }

    // zeros
    while (col >= 0) {
      grid[lin][col] = 0;
      col--;
    }

    // // coalesce
    for (int y = GRID_LEN - 1; y > 0; y--) {
      if (grid[lin][y] and grid[lin][y] == grid[lin][y - 1]) {
        score += grid[lin][y];
        grid[lin][y] = grid[lin][y] << 1;
        max_val = max(max_val, grid[lin][y]);
        grid[lin][y - 1] = 0;
        y--;
      }
    }

    // gravity
    col = GRID_LEN - 1;
    for (int y = GRID_LEN - 1; y >= 0; y--) {
      if (grid[lin][y]) {
        grid[lin][col] = grid[lin][y];
        col--;
      }
    }

    // zeros
    while (col >= 0) {
      grid[lin][col] = 0;
      col--;
    }
  }

  this->addRandomTile();
  this->draw();
}

void Grid::swipeUp() {
  for (int lin = 0; lin < GRID_LEN; lin++) {
    // gravity
    int col = 0;
    for (int y = 0; y < GRID_LEN; y++) {
      if (grid[lin][y]) {
        grid[lin][col] = grid[lin][y];
        col++;
      }
    }

    // zeros
    while (col < GRID_LEN) {
      grid[lin][col] = 0;
      col++;
    }

    // // coalesce
    for (int y = 0; y < GRID_LEN - 1; y++) {
      if (grid[lin][y] and grid[lin][y] == grid[lin][y + 1]) {
        score += grid[lin][y];
        grid[lin][y] = grid[lin][y] << 1;
        max_val = max(max_val, grid[lin][y]);
        grid[lin][y + 1] = 0;
        y++;
      }
    }

    // gravity
    col = 0;
    for (int y = 0; y < GRID_LEN; y++) {
      if (grid[lin][y]) {
        grid[lin][col] = grid[lin][y];
        col++;
      }
    }

    // zeros
    while (col < GRID_LEN) {
      grid[lin][col] = 0;
      col++;
    }
  }

  this->addRandomTile();
  this->draw();
}

void Grid::swipeLeft() {
  for (int col = 0; col < GRID_LEN; col++) {
    // gravity
    int lin = 0;
    for (int x = 0; x < GRID_LEN; x++) {
      if (grid[x][col]) {
        grid[lin][col] = grid[x][col];
        lin++;
      }
    }

    // zeros
    while (lin < GRID_LEN) {
      grid[lin][col] = 0;
      lin++;
    }

    // // coalesce
    for (int x = 0; x < GRID_LEN - 1; x++) {
      if (grid[x][col] and grid[x][col] == grid[x + 1][col]) {
        score += grid[x][col];
        grid[x][col] = grid[x][col] << 1;
        max_val = max(max_val, grid[x][col]);
        grid[x + 1][col] = 0;
        x++;
      }
    }

    // gravity
    lin = 0;
    for (int x = 0; x < GRID_LEN; x++) {
      if (grid[x][col]) {
        grid[lin][col] = grid[x][col];
        lin++;
      }
    }

    // zeros
    while (lin < GRID_LEN) {
      grid[lin][col] = 0;
      lin++;
    }
  }

  this->addRandomTile();
  this->draw();
}

void Grid::swipeRight() {
  for (int col = 0; col < GRID_LEN; col++) {
    // gravity
    int lin = GRID_LEN - 1;
    for (int x = GRID_LEN - 1; x >= 0; x--) {
      if (grid[x][col]) {
        grid[lin][col] = grid[x][col];
        lin--;
      }
    }

    // zeros
    while (lin >= 0) {
      grid[lin][col] = 0;
      lin--;
    }

    // // coalesce
    for (int x = GRID_LEN - 1; x > 0; x--) {
      if (grid[x][col] and grid[x][col] == grid[x - 1][col]) {
        score += grid[x][col];
        grid[x][col] = grid[x][col] << 1;
        max_val = max(max_val, grid[x][col]);
        grid[x - 1][col] = 0;
        x--;
      }
    }

    // gravity
    lin = GRID_LEN - 1;
    for (int x = GRID_LEN - 1; x >= 0; x--) {
      if (grid[x][col]) {
        grid[lin][col] = grid[x][col];
        lin--;
      }
    }

    // zeros
    while (lin >= 0) {
      grid[lin][col] = 0;
      lin--;
    }
  }

  this->addRandomTile();
  this->draw();
}

void Grid::addRandomTile() {
  int pairs[16][2];
  int idx = 0;
  for (int i = 0; i < GRID_LEN; i++)
    for (int j = 0; j < GRID_LEN; j++)
      if (!grid[i][j]) {
        pairs[idx][0] = i;
        pairs[idx][1] = j;
        idx++;
      }

  int pos = random(0, idx);
  int prob = random(0, 100);
  int val = 2;
  if (prob >= 80)
    val = 4;
  grid[pairs[pos][0]][pairs[pos][1]] = val;
}

void handleSwipe(TS_Point s, TS_Point e) {
  // Map raw values to screen pixels
  int startX = map(s.x, RAW_MAX_X, RAW_MIN_X, 0, SCREEN_WIDTH);
  int startY = map(s.y, RAW_MAX_Y, RAW_MIN_Y, 0, SCREEN_HEIGHT);
  int endX   = map(e.x, RAW_MAX_X, RAW_MIN_X, 0, SCREEN_WIDTH);
  int endY   = map(e.y, RAW_MAX_Y, RAW_MIN_Y, 0, SCREEN_HEIGHT);

  int dx = endX - startX;
  int dy = endY - startY;
  const int threshold = 10;  // Minimum movement to be a swipe

  if (win_screen and startX >= RETRY_X and startX <= RETRY_X + RETRY_W and startY >= RETRY_Y
     and startY <= RETRY_Y + RETRY_H 
      and dx <= threshold and dy <= threshold) {

      score = 0;
      grid = Grid();
      for (int i = 0; i < NO_UNDOS; i++) {
        last_score[i] = 0;
        last_grids[i] = Grid();
      }
      win_screen = false;
      initialize();
      delay(300);
      return;
    }

  if (startX >= BUTTON_X and startX <= BUTTON_X + BUTTON_W and startY >= BUTTON_Y and startY <= BUTTON_Y + BUTTON_H 
    and dx <= threshold and dy <= threshold) {
      handleUndo();
      return;
    }

  if (abs(dx) > abs(dy)) {
    if (abs(dx) > threshold) {
      saveState();

      if (dx > 0) {
        grid.swipeRight();
      } else {
        grid.swipeLeft();
      }
    }
  } else {
    if (abs(dy) > threshold) {
      for (int i = 0; i < NO_UNDOS - 1; i++) {
        last_grids[i] = last_grids[i + 1];
        last_score[i] = last_score[i + 1];
      }

      last_grids[NO_UNDOS - 1] = grid;
      last_score[NO_UNDOS - 1] = score;

      if (dy > 0) {
        grid.swipeDown();
      } else {
        grid.swipeUp();
      }
    }
  }

  if (grid.max_val == WIN_VALUE)
    has_won = true;

  delay(300);  // Prevents rapid multiple detections
}
