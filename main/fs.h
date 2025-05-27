#ifndef FS_H
#define FS_H

#include "esp_err.h"

void listSPIFFS(char *path);
esp_err_t mountSPIFFS(char *path, char *label, int max_files);

#endif  // FS_H