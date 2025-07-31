#include "pong_menu.h"

#include <stdlib.h>
#include <string.h>

Menu* create_menu(const char* titles[], MenuItemAction actions[], int count) {
  Menu* menu = malloc(sizeof(Menu));
  menu->items = malloc(count * sizeof(MenuItem));
  menu->count = count;
  menu->selected = 0;

  for (int i = 0; i < count; i++) {
    menu->items[i].title = strdup(titles[i]);
    menu->items[i].action = actions[i];
  }

  return menu;
}

void menu_up(Menu* menu) {
  if (menu->selected > 0) {
    menu->selected--;
  }
}

void menu_down(Menu* menu) {
  if (menu->selected < menu->count - 1) {
    menu->selected++;
  }
}

void menu_select(Menu* menu, void* pvParameters) {
  if (menu->items[menu->selected].action) {
    menu->items[menu->selected].action(pvParameters);
  }
}

void free_menu(Menu* menu) {
  if (!menu) return;

  for (int i = 0; i < menu->count; i++) {
    free(menu->items[i].title);
  }

  free(menu->items);
  free(menu);
}