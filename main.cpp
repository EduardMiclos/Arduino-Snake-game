/* --- LIBRARIES --- */
#include <IRremote.h>
#include <Vector.h>
/* ----------------- */



using namespace std;

#define MAZE_ROW_SIZE 8
#define MAZE_COL_SIZE 8



/* --- GLOBAL VARIABLES --- */
int pins[17]= { 99, 5, 4, 3, 2, 14, 15, 16, 17, 13, 12, 11, 10, 9, 8, 7, 6};
int cols[8] = { pins[13], pins[10], pins[15], pins[9], pins[4], pins[16], pins[6], pins[1]};
int rows[8] = {pins[8], pins[7], pins[3], pins[14], pins[2], pins[12], pins[11], pins[5]};

/* Receiver PIN for IR Remote signal. */
int RECV_PIN = 19;

IRrecv irrecv(RECV_PIN);
decode_results results;
/* ------------------------ */



/* ENUM */
enum direction {LEFT, UP, RIGHT, DOWN, NONE};
/* ---- */



/* --- CLASSES --- */
class Position{
  int x, y;
public:
    Position(void){};
    Position(int, int);
    int getX(void);
    int setX(int);
    int getY(void);
    int setY(int);
    int positionIntersect(Position);
};
Position::Position(int x, int y){
  this->x = x;
  this->y = y;
}
int Position::getX(void){
  return this->x;
}
int Position::setX(int x){
  this->x = x;
}
int Position::getY(void){
  return this->y;
}
int Position::setY(int y){
  this->y = y;
}
int Position::positionIntersect(Position P){
  return (this->x == P.getX()) && (this->y == P.getY());
}


class Snake{
  Vector <Position> coordinates;  
  Vector <direction> orientation;
  direction forbiddenMove;
  Position position_storrage_array[MAZE_ROW_SIZE * MAZE_COL_SIZE];
  direction direction_storrage_array[MAZE_ROW_SIZE * MAZE_COL_SIZE];
  int length;
public:
    Snake(Position, Position);
    Position* getPosition(int);
    Position *getHead(void);
    int getLength(void);
    int selfIntersect(void);
    void grow(void);
    void updatePosition(direction);
    void reset(void);
};
Snake::Snake(Position head, Position tail){
  /* Setting the maximum amount of elements for coordinates. */
  this->coordinates.setStorage(position_storrage_array);

  this->coordinates.push_back(head);
  this->coordinates.push_back(tail);

  /* Setting the maximum amount of elements for orientation. */
  this->orientation.setStorage(direction_storrage_array);

  this->orientation.push_back(RIGHT);
  this->orientation.push_back(RIGHT);

  /* You cannot move in the opposite side of the current orientation. */
  this->forbiddenMove = LEFT;
  this->length = 2;
}
void Snake::grow(void){
  int lastX, lastY;
  direction lastOrientation = this->orientation.back();

  this->length++;

  /* If the snake grows, it grows in the same direction in which he was moving before. */
  this->orientation.push_back(lastOrientation);

  Position lastPosition, newPosition;
  lastPosition = this->coordinates.back();

  lastX = lastPosition.getX();
  lastY = lastPosition.getY();

  switch(lastOrientation){
    case LEFT:
        newPosition.setX(lastX);
        newPosition.setY((lastY - 1 + MAZE_COL_SIZE) % MAZE_COL_SIZE);
        break;
      case UP:
        newPosition.setX((lastX - 1 + MAZE_ROW_SIZE) % MAZE_ROW_SIZE);
        newPosition.setY(lastY);
        break;
      case RIGHT:
        newPosition.setX(lastX);
        newPosition.setY((lastY + 1) % MAZE_COL_SIZE);
        break;
      case DOWN:
        newPosition.setX((lastX + 1) % MAZE_ROW_SIZE);
        newPosition.setY(lastY);
        break;
      default:
        break;
  }

  this->coordinates.push_back(newPosition);

}
void Snake::updatePosition(direction new_direction){
  int i;

  /* You can't move along the same axis. */
  if(new_direction == forbiddenMove)
    new_direction = this->orientation.back();
  
  for(i = 0; i < this->length - 1; i++)
    this->orientation[i] = this->orientation[i + 1];
  this->orientation[i] = new_direction;

  for(i = 0; i < this->length; i++){
    int x = this->coordinates[i].getX();
    int y = this->coordinates[i].getY();  

    switch(this->orientation[i]){
      case LEFT:
        this->coordinates[i].setY((y - 1 + MAZE_COL_SIZE) % MAZE_COL_SIZE);
        this->forbiddenMove = RIGHT;
        break;
      case UP:
        this->coordinates[i].setX((x - 1 + MAZE_ROW_SIZE) % MAZE_ROW_SIZE);
        this->forbiddenMove = DOWN;
        break;
      case RIGHT:
        this->coordinates[i].setY((y + 1) % MAZE_COL_SIZE);
        this->forbiddenMove = LEFT;
        break;
      case DOWN:
        this->coordinates[i].setX((x + 1) % MAZE_ROW_SIZE);
        this->forbiddenMove = UP;
        break;
      default:
        break;
    }
  }

}
int Snake::getLength(void){
  return this->length;
}
Position* Snake::getPosition(int i){
  return &this->coordinates[i];
}
Position* Snake::getHead(){
  return this->getPosition(this->length - 1);
}
int Snake::selfIntersect(void){
  int i;
  Position *head = this->getHead();

  /* Checking if the snake self intersects itself. */
  for(i = 0; i < this->length - 1; i++)
    if(head->positionIntersect(this->coordinates[i]))
      return 1;
  return 0;
}
void Snake::reset(void){
  while(this->length > 2){
    this->coordinates.pop_back();
    this->orientation.pop_back();
    this->length--;
  }
}


class Food{
  Position coordinates;
public:
  Food(Snake);
  Position* getPosition(void);
  int intersect(Snake, int, int);
  int touchesHead(Snake);
  void updatePosition(Snake);
};
Food::Food(Snake snake){
  this->updatePosition(snake);
}
void Food::updatePosition(Snake snake){
  int xRand, yRand;

  /* Spawn food until the position doesn't intersect with any Snake segment. */
  do {
    xRand = rand() % MAZE_ROW_SIZE;
    yRand = rand() % MAZE_COL_SIZE;
  } while(this->intersect(snake, xRand, yRand));

  this->coordinates.setX(xRand);
  this->coordinates.setY(yRand);
}
int Food::intersect(Snake snake, int x, int y){
  int i;
  Position *pos;

  for(i = 0; i < snake.getLength(); i++){
    pos = snake.getPosition(i);
    if(x == pos->getX() && y == pos->getY()) return 1;
  }

  return 0;
}
int Food::touchesHead(Snake snake){
  return this->coordinates.positionIntersect(*snake.getHead());
}
Position* Food::getPosition(void){
  return &this->coordinates;
}


class Maze{
  int matrix[MAZE_ROW_SIZE][MAZE_COL_SIZE] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
  };
public:
  Maze(Snake, Food);
  int getValue(int, int);
  void resetMatrix(void);
  void updateMatrix(Snake, Food);
};
Maze::Maze(Snake snake, Food food){
  this->updateMatrix(snake, food);
}
void Maze::resetMatrix(void){
  int i, j;
  for(i = 0; i < MAZE_ROW_SIZE; i++)
    for(j = 0; j < MAZE_COL_SIZE; j++)
      this->matrix[i][j] = 0;
}
void Maze::updateMatrix(Snake snake, Food food){
  this->resetMatrix();
  int i, length = snake.getLength(), x, y;
  Position *snakePosition, *foodPosition;

  for(i = 0; i < length; i++){
    snakePosition = snake.getPosition(i);
    x = snakePosition->getX();
    y = snakePosition->getY();
    this->matrix[x][y] = 1;
  }
  
  foodPosition = food.getPosition();
  this->matrix[foodPosition->getX()][foodPosition->getY()] = 1;
}
int Maze::getValue(int x, int y){
  return this->matrix[x][y];
}
/* ------------ */



/* --- GAME GLOBAL VARIABLES --- */
/* Setting initial position. */
  Position head(0, 0), tail(0, 1);

  /* Creating the Snake. */
  Snake snake(head, tail);

  /* Creating the food. */
  Food food(snake);

  /* Creating the maze. */
  Maze maze(snake, food);

/* ---------------------------- */


void setup(){
  int i;

  /* For Monitor ouput. */
  Serial.begin(9600);

  /* Specifying the pin that is connected to receiver module output. */
  irrecv.enableIRIn();
  
  /* Settings PINS mode. */
  for(i = 1; i <= 16; i++)
    pinMode(pins[i], OUTPUT);

  /* Settings all LEDs to LOW. */
  for(i = 0; i < 8; i++)
    digitalWrite(rows[i - 1], LOW);
}


/* Light up the 8x8 matrix. */
void lightUp(Maze maze){
  int i, j;

  for(i = 1; i <= 8; i++){

    /* Only set on HIGH the COLUMNS that we don't want to light up. */
    for(j = 1; j <= 8; j++)
      digitalWrite(cols[j - 1], 1 - maze.getValue(i - 1, j - 1));

    digitalWrite(rows[i - 1], HIGH);
      
    delay(1);
  
    /* Reset all ROWS to LOW. */
    digitalWrite(rows[i - 1], LOW);

    /* Reset all COLUMNS to LOW for future animation. */
    for(j = 1; j <= 8; j++)
      digitalWrite(cols[j - 1], LOW);

   }
}

int updateGame(direction orientation){
  snake.updatePosition(orientation);

  /* If the snake intersects itself, the game should be reseted. */
  if(snake.selfIntersect())
    return 1;


  /* The snake only grows if it touches the food. */
  if(food.touchesHead(snake)){
    snake.grow();
    food.updatePosition(snake);
  }
  
  maze.updateMatrix(snake, food);

  /* If the first condition is not met, then the game should not be reseted. */
  return 0;
}

/* Decoding the IR modulated signals. */
direction decodeValue(){
  switch(results.value){
    case 0xFF10EF:   return LEFT;
    case 0xFF18E7:   return UP;
    case 0xFF5AA5:   return RIGHT;
    case 0xFF4AB5:   return DOWN;
    default: return NONE;
  }
}

void reset(){
  snake.reset();
  maze.updateMatrix(snake, food);
}

void loop(){
  direction signalValue = RIGHT;
  direction lastValidValue = RIGHT;
  int onScreenTime, resetGame = 0;
  
  while(!resetGame){

    onScreenTime = 0;
    while(onScreenTime <= 25){
      lightUp(maze);
      onScreenTime++;
    }

    /* Decoding the IR signal. */
    if(irrecv.decode(&results)){
    signalValue = decodeValue();
    irrecv.resume(); 
  }

    /* Checking if the signal corresponds to a certain move (LEFT, UPT, RIGHT, DOWN). */
    if(signalValue != NONE){
      lastValidValue = signalValue;
      resetGame = updateGame(signalValue);
    }
    else
      resetGame = updateGame(lastValidValue);
    
  };

  reset();
}