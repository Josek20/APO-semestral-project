/*******************************************************************
  Project main function template for MicroZed based MZ_APO board
  designed by Petr Porazil at PiKRON

  change_me.c      - main file

  include your name there and license for distribution.

  Remove next text: This line should not appear in submitted
  work and project name should be change to match real application.
  If this text is there I want 10 points subtracted from final
  evaluation.

 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"



char *memdev="/dev/mem";
//Assigning 
#define SPILED_REG_BASE_PHYS 0x43c40000
#define SPILED_REG_SIZE      0x00004000
#define SPILED_REG_LED_LINE_o           0x004
#define SPILED_REG_LED_RGB1_o           0x010
#define SPILED_REG_LED_RGB2_o           0x014
#define SPILED_REG_LED_KBDWR_DIRECT_o   0x018
#define SPILED_REG_KBDRD_KNOBS_DIRECT_o 0x020
#define SPILED_REG_KNOBS_8BIT_o         0x024

#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0xEE82EE      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0x8B0000      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */
#define Pink            0xF81F

#define FRAME_H 320
#define FRAME_W 480
#define MAX_FOOD 100
#define font font_winFreeSystem14x16

uint32_t rgb_knobs_value;
uint16_t frame[FRAME_H][FRAME_W];

unsigned char *mem_base;
unsigned char *parlcd_mem_base = 0;

int speed = 10;
int snake_size = 10;
char colors[4] = {Olive,Red,Purple,LightGrey};
int food[MAX_FOOD][2];
int food_size = 5;
int mode = 0;
uint16_t snake_color;

int score_counter1 = 0;
int score_counter2 = 0;

char a,res;  

enum direction{
  UP,//0
  LEFT,//1
  DOWN,//2
  RIGHT//3
};

struct snake_head{
  int position_x;
  int position_y;
  int size;
  // char snake[50];
  // struct snake_head *body;
};

double distance(int pos_x,int pos_y,int food_position[2]){
  // double a = sqrt((pos_x-food_position[0])*(pos_x-food_position[0])+(pos_y-food_position[1])*(pos_y-food_position[1]));
  double a = (pos_x-food_position[0])*(pos_x-food_position[0]);
  double b = (pos_y-food_position[1])*(pos_y-food_position[1]);
  double c = sqrt(a+b);
  return c;  
}

enum direction search_algorithm(int start_x,int start_y,int food_position[2],enum direction initial){
  
  enum direction path;
  double best = 1000;
  //int best_move = 0;
  
  //for(int i = 0;i<4;i++){
    // printf("direction x %d:%d\n",i,start_x-2*speed*(i == 0 || i == 2? (i/2==0?-1:1)*(-1):0));
    // printf("direction y %d:%d\n",i,start_y-2*speed*(i == 1 || i==3? (i/2==0?-1:1)*(-1):0));
    if(initial == 0){
      if(best>distance(start_x,start_y-2*speed,food_position)){
	best = distance(start_x,start_y-2*speed,food_position);
	path = 1;
      }
      if(best>distance(start_x,start_y+2*speed,food_position)){
	best = distance(start_x,start_y+2*speed,food_position);
	path = 3;
      }
      if(best>distance(start_x-2*speed,start_y,food_position)){
	best = distance(start_x-2*speed,start_y,food_position);
	path = 0;
      } 
    }
    if(initial == 2){
      if(best>distance(start_x,start_y-2*speed,food_position)){
	best = distance(start_x,start_y-2*speed,food_position);
	path = 1;
      }
      if(best>distance(start_x,start_y+2*speed,food_position)){
	best = distance(start_x,start_y+2*speed,food_position);
	path = 3;
      }
      if(best>distance(start_x+2*speed,start_y,food_position)){
	best = distance(start_x+2*speed,start_y,food_position);
	path = 2;
      } 
    }
    if(initial == 3){
      if(best>distance(start_x,start_y+2*speed,food_position)){
	best = distance(start_x,start_y+2*speed,food_position);
	path = 3;
      }
      if(best>distance(start_x-2*speed,start_y,food_position)){
	best = distance(start_x-2*speed,start_y,food_position);
	path = 0;
      }
      if(best>distance(start_x+2*speed,start_y,food_position)){
	best = distance(start_x+2*speed,start_y,food_position);
	path = 2;
      } 
    }
    if(initial == 1){
      if(best>distance(start_x,start_y-2*speed,food_position)){
	best = distance(start_x,start_y-2*speed,food_position);
	path = 1;
      }
      if(best>distance(start_x-2*speed,start_y,food_position)){
	best = distance(start_x-2*speed,start_y,food_position);
	path = 0;
      }
      if(best>distance(start_x+2*speed,start_y,food_position)){
	best = distance(start_x+2*speed,start_y,food_position);
	path = 2;
      } 
    }/*
    if(best > distance(start_x-2*speed*(i == 0 || i == 2? (i/2==0?-1:1)*(-1):0), start_y-2*speed*(i == 1 || i==3? (i/2==0?-1:1)*(-1):0),food_position)){
      best = distance(start_x-2*speed*(i == 0 || i == 2? (i/2==0?-1:1)*(-1):0),start_y-2*speed*(i == 1 || i==3? (i/2==0?-1:1)*(-1):0),food_position);
      
      path = i;
      //best_move = i;
    }*/
  //}
  return path;
}

int colision_detection(int snake_x,int snake_y,int boudaries_size){
  if (snake_x+boudaries_size > FRAME_H-boudaries_size || 
      snake_x-boudaries_size < boudaries_size ||
      snake_y+boudaries_size > FRAME_W-boudaries_size ||
      snake_y-boudaries_size < boudaries_size)
  {
    return 1;
  }
  return 0;
}

int colision_with_food(int snake_x,int snake_y, int food_x,int food_y,int size){
  if(abs(snake_x-food_x)<= size && abs(snake_y-food_y)<= size){
    return 1;
  }
  return 0;
}

int colision(int food_counter,struct snake_head buddy[MAX_FOOD],struct snake_head head){
  for(int i = 0;i<food_counter;i++){
    if(abs(buddy[i].position_x-head.position_x)<2*snake_size && abs(buddy[i].position_y-head.position_y)<2*snake_size){
      return 1;
    }
  }
  return 0;
}

void draw(void){
  for(int i = 0;i<FRAME_H;i++){
    for(int j = 0;j<FRAME_W;j++){
      parlcd_write_data(parlcd_mem_base, frame[i][j]);
    }
  }
}

void paint_frame(uint16_t color, uint16_t boudaries,int boudaries_size){
  for(int i = 0;i<FRAME_H;i++){
    for(int j = 0;j<FRAME_W;j++){
      if (i<boudaries_size || i>FRAME_H-boudaries_size){
        frame[i][j] = boudaries;
      }
      else if (j<boudaries_size || j>FRAME_W-boudaries_size){
        frame[i][j] = boudaries;
      }
      else{
        frame[i][j] = color;
      }
    }
  }
}

void fill(int x_start,int y_start,int size,uint16_t color){
  for(int i = x_start-size;i<x_start+size;i++){
    for(int j = y_start-size;j<y_start+size;j++){
      frame[i][j] = color;
    }
  }
}

void fill_display(char *string, int pos_y,int pos_x,int size, uint16_t forground,uint16_t background){
  int char_ind_1,strwidth;
  uint16_t row,color;
  strwidth = 0;
	for(int g = 0; g < strlen(string);g++){
		char_ind_1 = string[g] - font.firstchar;
		if(char_ind_1<font.size) {
      int char_ind = string[g] - font.firstchar;
      
      const uint16_t *bits = font.bits + char_ind * font.height;
      unsigned char width = font.width != 0? font.width[char_ind] : font.maxwidth;

      for(int i = 0; i < font.height; i ++)
      {
        row = bits[i];
        for(int j = 0; j < width; j++)
        {
          color = ((row & 0x8000)!=0) ? forground : background;
          for(int k = 0; k < size; ++k)
          {
            for(int m = 0; m < size; ++m)
            {
              frame[i * size + pos_y + k][j * size +pos_x+strwidth*size+ m] = color;          
            }
          }
          row <<= 1;
      }
    }
  }
  unsigned char width_1 = font.width;
  
  if (width_1!=0){
    width_1 = font.width[char_ind_1];
  }
  else{
    width_1 = font.maxwidth;
  }
  
  strwidth += width_1;
  
  }
}

void fill_food(int food[2],uint16_t food_color, int size){
  for(int i = food[0]-size;i<food[0]+size;i++){
    for(int j=food[1]-size;j<food[1]+size;j++){
      frame[i][j] = food_color;
    }
  }
}

void randomize_food()
{
  int a,b;
  for(int i = 0;i<MAX_FOOD;i++)
  {
    a = rand()%(FRAME_H - 20);
    b = rand()%(FRAME_W - 20);
    food[i][0] = (a>40 && a!=160?a:a+40);
    food[i][1] = (b>40 && b!=240?b:b+40);
    // printf("(x,y):(%d,%d)\n",food[i][0],food[i][1]);
  }
}

//TODO:draw a hand 
void fill_hand(int a,int b){//90 or 190
  for(int i = 0;i<FRAME_H;i++){
    for(int j = 0;j<FRAME_W;j++){
      if ((j-b)*(j-b)+(i-a)*(i-a)<=100){
        frame[i][j] = White;
      }
      else{
        frame[i][j] = Black;      
      }
    }
  }
}

void display(){

  int s = 50,k;
  // fill(0,0,Black);
  fill_hand(s,100);
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
  int flag = 1;
  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
  
  int buff_rk[2];
  buff_rk[0] = (rgb_knobs_value>>16) & 0xFF;


  fill_display("Player with AI",40,140,2,DarkGrey,Black);  
  fill_display("AI with AI",140,140,2,DarkGrey,Black); 
  fill_display("Speed and color",240,140,2,DarkGrey,Black);
  draw();
  
  k = s;


  while(flag){
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    res = read(0,&a,1);    

    // usleep(20000);
      
    buff_rk[1] = (rgb_knobs_value>>16) & 0xFF; //red knob
    switch(a){
      case 's'://True
        // k = s+100>=290?290:290;
        k = k+100*(k==250?0:1);
        // fill_hand(k,100);
        break;
      case 'w'://False
        // k = s-100;
        k = k-100*(k==50?0:1); 
        // fill_hand(k,100);
        break;
      case ' ':
        flag = 0;
        a = 'g';
        mode = k/100 == 2?2:(k/100==1?1:0);
        // printf("%d\n",mode);
        //call another display with speed and color
        if (mode==2){
          flag = 1;
	        char speed_screen[] = "000";
          speed_screen[strlen(speed_screen)-2] = (speed/10>0?speed/10:0)+'0';		  
          speed_screen[strlen(speed_screen)-1] = speed%10+'0';
          fill_hand(s,50);
	        int b = 50;
	        int index = 0;
          while(flag){
            res = read(0,&a,1);  
            
            fill_display("Speed",40,60,2,DarkGrey,Black);
            fill_display("Color",40,360,2,DarkGrey,Black);   
            fill_display(speed_screen,70,70,2,DarkGrey,Black);
            draw(); 
            switch(a){
              case 'w':
                if(b==50){
                  speed+=1;
                  if(speed>20){
                    speed = 20;
                  }
                  speed_screen[strlen(speed_screen)-2] = (speed/10>0?speed/10:0)+'0';		  
                  speed_screen[strlen(speed_screen)-1] = speed%10+'0';
                  
                }
                else{
                  index+=1;
                  if(index == 4){
                    index = 0;
                  }
                  for(int j = 80;j<100;j++){
                    for(int i = 360;i<400;i++){
                      frame[j][i] = colors[index];
                    }
                  }
                }
                draw();
                break;
              case 's':
                if(b==50){
                  speed-=1;
                  if(speed<0){
                  speed = 0;
                  }
                  speed_screen[strlen(speed_screen)-2] = (speed/10>0?speed/10:0)+'0';		  
                  speed_screen[strlen(speed_screen)-1] = speed%10+'0';
                  // fill_display(speed_screen,70,70,2,DarkGrey,Black);
                }
                else{
                  index-=1;
                  if(index == -1){
                    index = 3;
                  }
                  // fill()
                  for(int j = 80;j<100;j++){
                    for(int i = 360;i<400;i++){
                      frame[j][i] = colors[index];
                    }
                  }
                }
                draw();
                break;
              case 'a':
                b = 50;
                fill_hand(s,b);
                break;
              case 'd':
                b = 350;
                fill_hand(s,b);
                break;
              case ' ':
                flag = 0;
                break;
              
            }

          }
          flag = 1;
	        snake_color = colors[index];
        }
        break;
      default:
        break;
    }
    buff_rk[0] = buff_rk[1];      
    fill_hand(k,100);
    fill_display("Player with AI",40,140,2,DarkGrey,Black);  
    fill_display("AI with AI",140,140,2,DarkGrey,Black); 
    fill_display("Speed and color",240,140,2,DarkGrey,Black);
    draw();
  }
}

int main(int argc, char *argv[])
{
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);  
  parlcd_hx8357_init(parlcd_mem_base);
  *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c; 
  rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);


  struct snake_head head = {160,240,10};//snake size is 20x20
  struct snake_head ai_head = {160,180,10};
  struct snake_head full_snake[MAX_FOOD];
  struct snake_head full_ai_snake[MAX_FOOD];
  
  int food_counter = 0;

  srand(time(NULL));
  randomize_food();
  
  enum direction drct = UP;
  enum direction drct_ai = search_algorithm(ai_head.position_x,ai_head.position_y,food[food_counter],UP);
  

  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = Red;
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = Red;
  display();

  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
  char scoreR[] = "Score:000";
  char scoreB[] = "Score:000";
  
  paint_frame(Purple,DarkGrey,head.size);  
  fill_display(scoreR,0,0,1,Red,DarkGrey);  
  fill_display(scoreB,0,410,1,Blue,DarkGrey);  
  
  fill(head.position_x,head.position_y,head.size,snake_color);
  fill(ai_head.position_x,ai_head.position_y,ai_head.size,Maroon);
  
  fill_food(food[food_counter],White, food_size);
  draw();

  int bk,rk;
  int buff_rk[2];
  int buff_bk[2];


  bk =  rgb_knobs_value      & 0xFF; // blue knob position
  rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
  
  buff_bk[0] = bk;
  buff_rk[0] = rk;
  
  
  
  while(1){

    if(mode == 0){
      rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
      res = read(0,&a,1);
      
    }
    if(mode == 1){
      drct = search_algorithm(head.position_x,head.position_y,food[food_counter],drct);
    }
    usleep(20000);
        
    //left goes down right goes up
    buff_bk[1] =  rgb_knobs_value      & 0xFF; // blue knob position
    buff_rk[1] = (rgb_knobs_value>>16) & 0xFF; // red knob position
    
    drct_ai = search_algorithm(ai_head.position_x,ai_head.position_y,food[food_counter],drct_ai);
    
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = Red;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = Red;

    if (a == 'a'|| buff_rk[0]>buff_rk[1]){
      drct += 1;
      if (drct>3)
      {
        drct = 0;
      } 
      
    }
    if (a == 's'|| buff_rk[0]<buff_rk[1]){
      drct -= 1;
      if (drct==-1)
      {
        drct = 3;
      } 
            
      
    }
    buff_rk[0] = buff_rk[1];      
    buff_bk[0] = buff_bk[1];
    
    //movement depending on the direction 
    if(score_counter1>0){
      for(int i = score_counter1-1; i>=0;i--){
        if(i==0){
          full_snake[i].position_x = head.position_x; 
          full_snake[i].position_y = head.position_y;
        }
        else{
          full_snake[i].position_x = full_snake[i-1].position_x; 
          full_snake[i].position_y = full_snake[i-1].position_y;
        }
      }
    }
   if(score_counter2>0){
      for(int i = score_counter2-1; i>=0;i--){
        if(i==0){
          full_ai_snake[i].position_x = ai_head.position_x; 
          full_ai_snake[i].position_y = ai_head.position_y;
        }
        else{
          full_ai_snake[i].position_x = full_ai_snake[i-1].position_x; 
          full_ai_snake[i].position_y = full_ai_snake[i-1].position_y;
        }
      }
    }

    ai_head.position_x +=speed*((drct_ai%2) == 0 && (drct_ai/2) == 1 ? 1:0);//down
    ai_head.position_x -=speed*((drct_ai%2) == 0 && (drct_ai/2) == 0 ? 1:0);//up

    ai_head.position_y +=speed*((drct_ai%2) == 1 && (drct_ai/2) == 1 ? 1:0);//right
    ai_head.position_y -=speed*((drct_ai%2) == 1 && (drct_ai/2) == 0 ? 1:0);//left
   
    head.position_x +=speed*((drct%2) == 0 && (drct/2) == 1 ? 1:0);//down
    head.position_x -=speed*((drct%2) == 0 && (drct/2) == 0 ? 1:0);//up

    head.position_y +=speed*((drct%2) == 1 && (drct/2) == 1 ? 1:0);//right
    head.position_y -=speed*((drct%2) == 1 && (drct/2) == 0 ? 1:0);//left
    
    if (colision_detection(head.position_x,head.position_y,head.size)||colision_detection(ai_head.position_x,ai_head.position_y,ai_head.size)){ 
      printf("Game over:%d\n",score_counter1);
      printf("Game over:%d\n",score_counter2);      
      
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = Magenta;
      // *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = Magenta;      
      // printf("Game over\n");
      fill_display("GAME OVER",120,80,4,Red,Black);
      draw();
      // break;
      break;
    }
    
    
    // write to decide which light to turn red 
    if(score_counter1>0){
      if(colision(score_counter1,full_snake,ai_head)){
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = Magenta;
        fill_display("GAME OVER",0,80,4,Red,Black);//120
        draw();
        break;
      }
    }
    if(score_counter2>0){
      if(colision(score_counter2,full_ai_snake,head)){
        fill_display("GAME OVER",0,80,4,Red,Blue);
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = Magenta;
        draw();
        break;
      }
    }
    if(colision_with_food(ai_head.position_x,ai_head.position_y,head.position_x,head.position_y,head.size)){
      // write to decide which light to turn red 
      if(head.position_x-ai_head.position_x>=20){
        if(drct == 0&&drct_ai==1){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
        }
        if(drct == 1&&drct_ai==2){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB2_o) = Magenta;
        }
        if(drct == 3&&drct_ai==2){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB2_o) = Magenta;
        }
        if(drct == 0&&drct_ai==3){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
        }
      }
      else if(ai_head.position_x-head.position_x>=20){
        if(drct == 1&&drct_ai==0){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB2_o) = Magenta;
        }
        if(drct == 2&&drct_ai==1){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
        }
        if(drct == 2&&drct_ai==3){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
        }
        if(drct == 3&&drct_ai==0){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB2_o) = Magenta;
        }

      }
      else if(ai_head.position_y - head.position_y>=20){
        if(drct == 0){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB2_o) = Magenta;
        }
        else{
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
        }
      }
      else if(head.position_y - ai_head.position_y>=20){
        if(drct_ai == 0){
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
        }
        else{
          *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB2_o) = Magenta;
        }
      }
      else{
        *(volatile uint32_t*)(mem_base+SPILED_REG_LED_RGB1_o) = Magenta;
      }
      fill_display("GAME OVER",0,80,4,Red,White);
      // *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = Magenta;
      draw();
      break;
    }


    if (colision_with_food(head.position_x,head.position_y,food[food_counter][0],food[food_counter][1],head.size)){
      food_counter+=1;
     
      full_snake[score_counter1].position_x = ((drct%2 == 0 && drct/2 == 1) ? head.position_x-20:((drct%2 == 0 && drct/2 == 0) ? head.position_x+20:head.position_x));
      // full_snake[score_counter1].position_x = ((drct%2 == 0 && drct/2 == 0) ? head.position_x+20:head.position_x);
      
      full_snake[score_counter1].position_y = ((drct%2) == 1 && (drct/2) == 1 ? head.position_y-20:((drct%2) == 1 && (drct/2) == 0 ? head.position_y+20:head.position_y));
      // full_snake[score_counter1].position_y = ((drct%2) == 1 && (drct/2) == 0 ? head.position_y+20:head.position_y);
     
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = Green;
    
      full_snake[score_counter1].size = 10;//head.size;
      score_counter1+=1;
      
    }
    if (colision_with_food(ai_head.position_x,ai_head.position_y,food[food_counter][0],food[food_counter][1],ai_head.size)){
      food_counter+=1;
      
      full_ai_snake[score_counter2].position_x = ((drct_ai%2 == 0 && drct_ai/2 == 1) ? ai_head.position_x-20:((drct_ai%2 == 0 && drct_ai/2 == 0) ? ai_head.position_x+20:ai_head.position_x));
      // full_ai_snake[score_counter2].position_x = ((drct_ai%2 == 0 && drct_ai/2 == 0) ? ai_head.position_x+20:ai_head.position_x);
      
      full_ai_snake[score_counter2].position_y = ((drct_ai%2) == 1 && (drct_ai/2) == 1 ? head.position_y-20:((drct_ai%2) == 1 && (drct_ai/2) == 0 ? head.position_y+20:ai_head.position_y));
      // full_ai_snake[score_counter2].position_y = ((drct_ai%2) == 1 && (drct_ai/2) == 0 ? head.position_y+20:ai_head.position_y);
      
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = Green;
      
      full_ai_snake[score_counter2].size = 10;//head.size;
      score_counter2+=1;
      
    }
    
    if(food_counter == MAX_FOOD){
      printf("Game over\n");
      fill_display("GAME OVER",120,80,4,Red,Black);      
      break;
    }
    
    scoreR[strlen(scoreR)-3] = score_counter1/100+'0';		   
    scoreR[strlen(scoreR)-2] = score_counter1/10+'0';		    
    scoreR[strlen(scoreR)-1] = score_counter1%10+'0';		   
    
    scoreB[strlen(scoreB)-3] = score_counter2/100+'0';		   
    scoreB[strlen(scoreB)-2] = score_counter2/10+'0';		    
    scoreB[strlen(scoreB)-1] = score_counter2%10+'0';

    
    
    paint_frame(Purple,DarkGrey,head.size);  
    fill_display(scoreR,0,0,1,Red,DarkGrey);  
    fill_display(scoreB,0,410,1,Blue,DarkGrey);   

    fill_food(food[food_counter],White, food_size);
    
    fill(head.position_x,head.position_y,head.size,snake_color);
    fill(ai_head.position_x,ai_head.position_y,ai_head.size,Blue);
    
    if(score_counter1>0){
      for(int i = score_counter1-1; i>=0;i--){
        fill(full_snake[i].position_x,full_snake[i].position_y,full_snake[i].size,snake_color);
      }
    }
    if(score_counter2>0){
      for(int i = score_counter2-1; i>=0;i--){
        fill(full_ai_snake[i].position_x,full_ai_snake[i].position_y,full_ai_snake[i].size,Blue);
      }
    }
    
    draw();

  }
  // printf("Game over\n");
  printf("Goodbye\n");

  return 0;
}
