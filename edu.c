#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_C_CHAR 10 // TODO this will likely fail
#define MAX_LINE 200
#define ENABLE_LOG 1
// TODO add time on log
#define log(msg, ...) do { if (ENABLE_LOG) fprintf(stderr, msg, __VA_ARGS__); } while (0)



enum CommandType {change, delete, print, undu, redo, quit};

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

// Row utils
struct Row {
  char * data;
  struct Row* next;
  struct Row* prev; // TODO prob remove
};

struct Row *find(struct Row *row, int index) { /*  */
  
  if(index == 0) {
    return row;
  }
  if (row->next != NULL) {
    return find(row->next, index-1);
  }
  return NULL;
};

void print_doc(struct Row *row) {
  if (row == NULL){
    printf("NULL\n");
    return;
  }
  printf("%s --> ", row->data);
  print_doc(row->next);
  
}

void insert_row(struct Row *row, int index, char *data) {
  if(index == 0) {
    log("Allocating space for data\n", NULL);
    row->data = (char *) realloc(row->data, strlen(data));
    strcpy(row->data, data);
    return;
  }
  if (index > 0 && row->next == NULL) {
    log("Creating a new node\n", NULL);
    row->next = (struct Row *) malloc(sizeof(struct Row));
    row->next->data = (char *) malloc(MAX_LINE);
    row->next->next = NULL;
    insert_row(row, index, data);
  } else {
    insert_row(row->next, index-1, data);
  }

}


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
void process_print(struct Command c, struct Row *doc_head){
  struct Row *current = find(doc_head, c.arg1);
  for (int i = c.arg1; i <= c.arg2 && current != NULL; i++){
    fputs(current->data, stdout);
    fputs("\n", stdout);
    current = current->next;
  }
}

void process_delete(struct Command c, struct Row *doc_head){
  struct Row *head = find(doc_head, c.arg1-1);
  struct Row *current = head->next;
  for (int i = c.arg1; i <= c.arg2 && current != NULL; i++){
    log("Deleting the node with data: %s\n", current->data);
    head->next = current->next;
    free(current->data);
    free(current);
    current = head->next;
  }

  // TODO remove
  print_doc(doc_head);

}


void process_change(struct Command c, struct Row *doc_head, FILE *fp_in){
  // TODO remove
  print_doc(doc_head);

  char str[MAX_LINE];
  struct Row* doc_head_c;
  for (int i = c.arg1; i <= c.arg2; i++){
    log("Chaning line: %d\n", i);
    fgets(str, MAX_LINE, fp_in);
    str[strlen(str)-1] = '\0';
    doc_head_c = doc_head;
    insert_row(doc_head_c, i, str);
  }
  // I assume the commands are correct, I drop the point
  fgets(str, MAX_LINE, fp_in);
  
  // TODO remove
  print_doc(doc_head);

}

void process_command(struct Command c, struct Row *doc_head, FILE *fp_in){
  switch(c.type) {
  case quit:
    break;
  case print:
    process_print(c, doc_head);
    break;
  case change:
    process_change(c, doc_head, fp_in);
    break;
  case delete:
    process_delete(c, doc_head);
    break;
  }
}


int main() {
  struct Command c;
  struct Row* doc_head = (struct Row *) malloc(sizeof(struct Row));
  doc_head->next = NULL;
  fputs("\033c", stdout); // TODO remove
  do {
    c = read_command(stdin);
    fputs("\033c", stdout); // TODO remove
    process_command(c, doc_head, stdin);
  } while(c.type != quit);
};



    
