#include <stdio.h>
#include <stdlib.h>

#define MAX_C_CHAR 10 // TODO this will likely fail
#define ENABLE_LOG 1
// TODO add time on log
#define log(msg, ...) do { if (ENABLE_LOG) fprintf(stderr, msg, __VA_ARGS__); } while (0)



enum CommandType {change, delete, print, undu, redo, quit};

// TODO:
// 1) what is static, inline
// 2) I miss Haskell :(
enum CommandType char2ct(char c) {
  enum CommandType ct;
  switch(c) {
  case 'c':
    ct = change;
    break;
  case 'd':
    ct = delete;
    break;
  case 'p':
    ct = print;
    break;
  case 'u':
    ct = undu;
    break;
  case 'r':
    ct = redo;
    break;
  case 'q':
    ct = quit;
    break;
  }
  return ct;
}


struct Command {
  int arg1;
  int arg2;
  enum CommandType type;
};

// No input validity checks
struct Command parse_command(char *str) {
  log("Parsing command: %s", str);
  char arg[MAX_C_CHAR];
  int i = 0;
  struct Command c;
  while(*str != ',' && *str != 'u' && *str != 'r' && *str != 'q'){
    // reading arg1
    arg[i++] = *(str++);
  }
  arg[i] = '\0';
  c.arg1 = atoi(arg);
  if(*str == ',') {
    str++;
    i = 0;
    while(*str != 'c' && *str != 'd' && *str != 'p'){
      //reading arg2
      arg[i++] = *(str++);
    }
    arg[i] = '\0';
    c.arg2 = atoi(arg);
  }
  c.type = char2ct(*str);
  log("Command: arg1: %d, arg2: %d, type: %d\n", c.arg1, c.arg2, c.type);
  return c;
};

struct Command read_command(FILE *fp) {
  char str[MAX_C_CHAR];
  fgets(str, MAX_C_CHAR, fp);
  struct Command c = parse_command(str);
  return c;
    
}

void process_command(struct Command c){
  if(c.type == quit){
    return;
  }
}

int main() {
  struct Command c;
  do {
    c = read_command(stdin);
    process_command(c);
  } while(c.type != quit);
};



    
