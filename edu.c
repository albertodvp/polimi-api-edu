#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_C_CHAR 10 // TODO this will likely fail
#define MAX_LINE 200
#define ENABLE_LOG 1
// TODO add time on log
#define log(msg, ...) do { if (ENABLE_LOG) fprintf(stderr, msg, __VA_ARGS__); } while (0)

enum CommandType {change, delete, print, undo, redo, quit};

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
    ct = undo;
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
  char * data;
};

// Row utils
struct Row {
  char * data;
  struct Row* next;
  struct Row* prev; // TODO prob remove
};


struct History {
  struct Command * c;
  struct History * next;
  struct History * prev;
};

struct History * HISTORY = NULL;
struct History * HISTORY_HEAD = NULL;
struct Row * DOC_HEAD = NULL;


struct Row *find(struct Row *row, int index) {
  if(index == 0) {
    return row;
  }
  if (row->next != NULL) {
    return find(row->next, index-1);
  }
  return NULL;
};


struct Row *find_in_doc(int index) {
  return find(DOC_HEAD, index);
}

void print_row(struct Row *row) {
  if (row == NULL){
    printf("NULL\n");
    return;
  }
  printf("%s --> ", row->data);
  print_row(row->next);
}

void print_doc(){
  print_row(DOC_HEAD);
}

void print_hist_sup(struct History *h) {
  if (h == NULL){
    printf("NULL\n");
    return;
  }
  else if (h->c == NULL){
    printf("() -->");
  } else {
    struct Command * c = h -> c;
    if (HISTORY == h) {
      printf("*** ");
    }
    printf("(%d,%d t:%d)--> ", c->arg1, c->arg2, c->type);
  }
  print_hist_sup(h->next);
  
}

void print_hist() {
  printf("-------\n");
  printf("History:\n");
  print_hist_sup(HISTORY_HEAD);
  printf("-------\n");
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
struct Command * parse_command(char *str) {
  log("Parsing command: %s", str);
  char arg[MAX_C_CHAR];
  int i = 0;
  struct Command * c = (struct Command *) malloc(sizeof(struct Command));
  while(*str != ',' && *str != 'u' && *str != 'r' && *str != 'q'){
    // reading arg1
    arg[i++] = *(str++);
  }
  arg[i] = '\0';
  c->arg1 = atoi(arg);
  if(*str == ',') {
    str++;
    i = 0;
    while(*str != 'c' && *str != 'd' && *str != 'p'){
      //reading arg2
      arg[i++] = *(str++);
    }
    arg[i] = '\0';
    c->arg2 = atoi(arg);
  }
  c->type = char2ct(*str);
  log("Command: arg1: %d, arg2: %d, type: %d\n", c->arg1, c->arg2, c->type);
  return c;
};

struct Command * read_command(FILE *fp) {
  char str[MAX_C_CHAR];
  fgets(str, MAX_C_CHAR, fp);
  struct Command * c = parse_command(str);
  return c;
    
}
void process_print(struct Command * c){
  struct Row *current = find_in_doc(c->arg1);
  for (int i = c->arg1; i <= c->arg2 && current != NULL; i++){
    fputs(current->data, stdout);
    fputs("\n", stdout);
    current = current->next;
  }
}

void process_delete(struct Command * c){
  struct Row *head = find_in_doc(c->arg1-1);
  struct Row *current = head->next;
  for (int i = c->arg1; i <= c->arg2 && current != NULL; i++){
    log("Deleting the node with data: %s\n", current->data);
    head->next = current->next;
    // sava data on a in memory file
    //current->data;
    free(current->data);
    free(current);
    current = head->next;
  }

  // TODO remove
  print_doc();

}


void process_change(struct Command *c, FILE *fp_in){
  // TODO remove
  print_doc();

  char str[MAX_LINE];
  struct Row* doc_head_c;
  for (int i = c->arg1; i <= c->arg2; i++){
    log("Chaning line: %d\n", i);
    fgets(str, MAX_LINE, fp_in);
    str[strlen(str)-1] = '\0';
    doc_head_c = DOC_HEAD;
    insert_row(doc_head_c, i, str);
  }
  // I assume the commands are correct, I drop the point
  fgets(str, MAX_LINE, fp_in);
  
  // TODO remove
  print_doc();

}
void process_undo(struct Command * c){
  // TODO
}

void process_redo(struct Command * c){
  // TODO
}

void append_history(struct Command * c) {
  if (HISTORY->next != NULL){
      // TODO clear from next
  }
  log("Creating new history node\n", NULL);
  struct History * next = (struct History *) malloc(sizeof(struct History));
  next->next = NULL;
  next->prev = HISTORY;
  next->c = NULL;
  HISTORY->next = next;
  HISTORY = next;
  HISTORY->c = c;
}

void process_command(struct Command * c, FILE *fp_in){
  switch(c->type) {
  case quit:
    break;
  case print:
    process_print(c);
    break;
  case change:
    append_history(c);
    process_change(c, fp_in);
    break;
  case delete:
    append_history(c);
    process_delete(c);
    break;
  case redo:
    process_redo(c);
    break;
  case undo:
    process_undo(c);
    break;
  }
}

void inizialize_hist(){
  printf("Initializing History\n");
  HISTORY = (struct History *) malloc(sizeof(struct History));
  HISTORY->next = NULL;
  HISTORY->prev = NULL;
  HISTORY->c = NULL;
  HISTORY_HEAD = HISTORY;
}
void initialize_doc() {
  printf("Initializing the document");
  DOC_HEAD = (struct Row *) malloc(sizeof(struct Row));
  DOC_HEAD->next = NULL;
}

int main() {
  struct Command * c;
  initialize_doc();
  inizialize_hist();
  do {
    print_hist();
    c = read_command(stdin);
    process_command(c, stdin);
  } while(c->type != quit);
}



    
