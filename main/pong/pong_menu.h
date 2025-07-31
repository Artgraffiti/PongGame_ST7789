#ifndef PONG_MENU_H
#define PONG_MENU_H

typedef struct MenuItem MenuItem;
typedef struct Menu Menu;
typedef void (*MenuItemAction)(void* pvParameters);

struct MenuItem {
  char* title;
  MenuItemAction action;
};

struct Menu {
  MenuItem* items;
  int count;
  int selected;
};

Menu* create_menu(const char* titles[], MenuItemAction actions[], int count);
void menu_up(Menu* menu);
void menu_down(Menu* menu);
void menu_select(Menu* menu, void* pvParameters);
void free_menu(Menu* menu);

#endif  // PONG_MENU_H