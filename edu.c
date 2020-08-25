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
    printf("(%d,%d t:%d, data: %s)--> ", c->arg1, c->arg2, c->type, c->data);
  }
  print_hist_sup(h->next);
  
}

void print_hist() {
  printf("-------\n");
  printf("History:\n");
  print_hist_sup(HISTORY_HEAD);
  printf("-------\n");
}

char * insert_row(struct Row *row, int index, char *data) {
  if(index == 0) {
    log("Allocating space for data\n", NULL);
    char * old = NULL;
    if(row->data == NULL){
      log("Changing new node.\n", NULL);
      row->data = (char *) malloc(strlen(data) + 1);
    } else {
      log("Changing old node.\n", NULL);      
      old = row->data;
      row->data = (char *) malloc(strlen(data) + 1);
    }
    strcpy(row->data, data);
    return old;
  }
  if (index > 0 && row->next == NULL) {
    log("Creating a new node\n", NULL);
    row->next = (struct Row *) malloc(sizeof(struct Row));
    row->next->data = NULL;
    row->next->next = NULL;
    return insert_row(row, index, data);
  } else {
    return insert_row(row->next, index-1, data);
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
  c->data = NULL;
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
    head->next = current->next;
    if (c->data == NULL){
      c->data = current->data;
    } else {
      strcat(c->data, current->data);
    }
    strcat(c->data, "\n");
    free(current);
    current = head->next;
  }
}
void insert(int i, char *data){
  char row[MAX_LINE] = "";
  int j = 0;
  while (*data){
    if(*data == '\n'){
      row[j] = '\0';
      j = 0;
      // insert node
      insert(i, row);
    } else {
      row[j++] = *data;
    }
    data++;
  }
  
}

char * clean_doc_from(int i){
  struct Row * p = DOC_HEAD;
  struct Row * supp = NULL;
  int j = 1;
  char * dropped_string = (char *)malloc(MAX_LINE);
  strcpy(dropped_string, "");
  while(p != NULL) {
    supp = p->next;
    if (i==j){
      // Update tail
      p->next = NULL;
    } else if (j >= i){
      strcat(dropped_string, p->data);
      strcat(dropped_string, "\n");
      free(p->data);
      free(p);
    } 
    p = supp;      
    j++;
  }
  return dropped_string;
}
void process_change(struct Command *c, FILE *fp_in){
  char str[MAX_LINE];
  struct Row* doc_head_c;
  char * old;

  c->data = (char *) malloc(sizeof(char));
  *(c->data) = '\0';
  for (int i = c->arg1; i <= c->arg2; i++){
    log("Changing line: %d\n", i);
    fgets(str, MAX_LINE, fp_in);
    if(strlen(str) == 0) {
      old = clean_doc_from(i);
      strcat(c->data, old);
      break;
    }
    str[strlen(str)-1] = '\0';
    old = insert_row(DOC_HEAD, i, str);
    if(old != NULL){
      // Changed lines
      strcat(c->data, old);
      strcat(c->data, "\n");
    }
  }
  
  // I assume the commands are correct, I drop the point
  if(fp_in == stdin) {
    fgets(str, MAX_LINE, fp_in);
  }
}

void single_hist_mov(struct Command * c) {
  log("(%d,%d t:%d, data: %s) \n", c->arg1, c->arg2, c->type, c->data);
  switch(c->type) {
  case change:
    if(strlen(c->data) == 0) {
      // Delete added lines
      struct Command dc;
      dc.arg1 = c->arg1;
      dc.arg2 = c->arg2;
      dc.data = NULL;
      dc.type = delete;
      process_delete(&dc);
      c->data = dc.data;
    } else {
      // Change lines
      char * data = c->data;
      FILE * in = fmemopen(data, strlen(data)+1, "r+");
      process_change(c, in);
      free(data);
      fclose(in);
    }
    break;
  case delete:
    // TODO
    exit(1);
    break;
  }
}

void process_undo(struct Command * c){
  for(int i=0; i<c->arg1 && HISTORY->prev != NULL; i++){
    log("Undoing...", NULL);
    single_hist_mov(HISTORY->c);
    HISTORY = HISTORY->prev;
  }
}

void process_redo(struct Command * c){
    for(int i=0; i<c->arg1; i++){
      if(HISTORY->next != NULL){
	log("Redoing...", NULL);
	HISTORY = HISTORY->next;
	single_hist_mov(HISTORY->c);
      }
  }
}

void append_history(struct Command * c) {
  if (HISTORY->next != NULL){
      // TODO clear from next
    log("Clear unreachable history\n", NULL);
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
  // TODO remove
  print_doc();

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
  // TODO remove
  print_doc();

}

void inizialize_hist(){
  fputs("Initializing History\n", stdout);
  HISTORY = (struct History *) malloc(sizeof(struct History));
  HISTORY->next = NULL;
  HISTORY->prev = NULL;
  HISTORY->c = NULL;
  HISTORY_HEAD = HISTORY;
}
void initialize_doc() {
  fputs("Initializing the document\n", stdout);
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



    
