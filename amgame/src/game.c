#include <game.h>
#include <klib.h>

void init_screen();
void splash();
int read_key1();
void get_tube(); 
void draw_bird();
int tube[50][2];

int main() {//flappy bird
  // Operating system is a C program
  _ioe_init();
  init_screen();
  //splash();
  get_tube();
  while (1) {
    read_key1();
    draw_bird();
  }
  return 0;
}

int read_key1() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    //puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
    if(event.keycode==_KEY_UP)
      return 1;
    else if(event.keycode==_KEY_S)
      return 2;
    else
      return 0;
  }
  else
    return 0;
}

int w, h;

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
int tube_num;
void get_tube(){
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

void draw_bird(){
  draw_rect1(0,(tube[0][0]+1)*SIDE,SIDE,SIDE,0xee0000);
}