#include <game.h>
#include <klib.h>

void init_screen();
void splash();
int read_key1();
void get_tube(); 
void delete_bird();
void draw_bird();
void draw_lose();
void draw_start();
void draw_win();
int tube[50][2];
int game_status;
int newx,newy;
int tube_num;
int w, h;

int main() {//go through
  // Operating system is a C program
  _ioe_init();
  init_screen();
  //splash();
  draw_start();
  game_status = 0;
  while (1) {
    int op = read_key1();
    if(op == 1 && game_status!=1){
      game_status = 1;
      get_tube();
      newx =0;
      newy =tube[0][0]+2;
      draw_bird();
    }
    if(game_status==1){
      if(op !=0 ){
        switch (op)
        {
          case 2:
            delete_bird();
            if(newy>=1)
              newy-=1;
            draw_bird();
            break;
          case 3:
            delete_bird();
            if((newy+2)*SIDE<=h)
              newy+=1;
            draw_bird();
            break;
          case 4:
            delete_bird();
            if(newx>=1)
              newx-=1;
            draw_bird();
            break;
          case 5:
            delete_bird();
            if((newx+2)*SIDE<=w)
              newx+=1;
            draw_bird();
            break;
          default:
            break;
        }
      }
      if(!((newx+1)%4)){
        int index=newx/4;
        if(newy<tube[index][0]||newy>tube[index][1]){
          game_status=2;
        }
      }
      if(newx>=tube_num*4){
        game_status=3;
      }
    }
    if(game_status==2){
      draw_lose();
    }
    if(game_status == 3){
      draw_win();
    }
  }
  return 0;
}

int read_key1() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  /*#define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };*/
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    //puts("Key pressed: ");
    //puts(key_names[event.keycode]);
    //puts("\n");
    if(event.keycode==_KEY_S)
      return 1;
    else if(event.keycode==_KEY_UP)
      return 2;
    else if(event.keycode==_KEY_DOWN)
      return 3;
    else if(event.keycode==_KEY_LEFT)
      return 4;
    else if(event.keycode==_KEY_RIGHT)
      return 5;
    else
      return 0;
  }
  else
    return 0;
}



void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

void draw_rect1(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  _DEV_VIDEO_FBCTL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}

/*void splash() {
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_rect1(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
}*/

void get_tube(){
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
        draw_rect1(x * SIDE, y * SIDE, SIDE, SIDE, 0x000000);
    }
  }
  tube_num = (w/SIDE)/4-1;
  for(int i=0;i < tube_num;i++){
    tube[i][0]=rand()%(h/SIDE/2)+2;
    tube[i][1]=tube[i][0]+5;
  }
  for(int x=0;x < tube_num; x++){
    for(int y = 0; y * SIDE <= h; y++){
      if(y<tube[x][0]||y>tube[x][1])
        draw_rect1((4*x+3)*SIDE, y * SIDE , SIDE, SIDE, 0x00ee00);
    }
  }
}

void delete_bird(){
  draw_rect1(newx*SIDE,newy*SIDE,SIDE,SIDE,0x000000);
}

void draw_bird(){
  draw_rect1(newx*SIDE,newy*SIDE,SIDE,SIDE,0xee0000);
}

void draw_lose(){
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_rect1(x * SIDE, y * SIDE, SIDE, SIDE, 0xeed5b7); // white
      }
    }
  }
}

void draw_start(){
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_rect1(x * SIDE, y * SIDE, SIDE, SIDE, 0x836fff); // blue
      }
    }
  }
}

void draw_win(){
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      draw_rect1(x * SIDE, y * SIDE, SIDE, SIDE, 0xffff00);
    }
  }
  int index_x = (w/SIDE/2)-4;
  int index_y = (h/SIDE/2)-3;
  draw_rect1((index_x)* SIDE, (index_y+2) * SIDE, SIDE, SIDE, 0xff3030);
  draw_rect1((index_x+1)* SIDE, (index_y+3) * SIDE, SIDE, SIDE, 0xff3030);
  draw_rect1((index_x+2)* SIDE, (index_y+4) * SIDE, SIDE, SIDE, 0xff3030);
  draw_rect1((index_x+3)* SIDE, (index_y+3) * SIDE, SIDE, SIDE, 0xff3030);
  draw_rect1((index_x+4)* SIDE, (index_y+2) * SIDE, SIDE, SIDE, 0xff3030);
  draw_rect1((index_x+5)* SIDE, (index_y+1) * SIDE, SIDE, SIDE, 0xff3030);
  draw_rect1((index_x+6)* SIDE, (index_y) * SIDE, SIDE, SIDE, 0xff3030);
}
