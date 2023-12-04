#include <stdio.h>
#include <stdlib.h>
static int DEBUG_PID = 1;
int main(int argc, char **argv) {
  if (argc > 1) {
    char **end;
    DEBUG_PID = strtol(*argv, end, 10);
  }
  FILE *file = fopen("/proc/lab2", "r+t");
  if (file != NULL) {
    fprintf(file, "%u\n", DEBUG_PID);
    char *info = malloc(400);
    fread(info, 1, 400, file);
    printf("/proc/lab2:\n%s", info);
    free(info);
  } else
    printf("%s\n", "Can't open the file");
  return 0;
}