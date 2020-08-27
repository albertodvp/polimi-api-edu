#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_C_CHAR 10 // TODO this will likely fail
#define MAX_LINE 10000

int NEXT_LINE;
enum CommandType {change, delete, print, undo, redo, quit};

enum CommandType char2ct(char c) {
  enum CommandType ct = quit;
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
} ;

// Row utils
struct Row {
  char * data;
  struct Row* next;
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
}


struct Row *find_in_doc(int index) {
  return find(DOC_HEAD, index);
}

char * insert_row(struct Row *row, int index, char *data) {
  if(index == 0) {
    char * old = NULL;
    if(row->data == NULL){
      NEXT_LINE++;
      row->data = (char *) malloc(sizeof(char) * (strlen(data) + 1));
    } else {
      old = row->data;
      row->data = (char *) malloc(sizeof(char) * (strlen(data) + 1));
    }
    strcpy(row->data, data);
    return old;
  }
  if (index > 0 && row->next == NULL) {
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
  return c;
}

struct Command * read_command(FILE *fp) {
  char str[MAX_C_CHAR];
  char * _ = fgets(str, MAX_C_CHAR, fp);
  struct Command * c = parse_command(str);
  return c;
    
}
void clean_history(struct History * h){
  if (h == NULL) return;
  clean_history(h->next);
  if(h->c != NULL){
    if (h->c->data != NULL){  
      free(h->c->data);
    }
    free(h->c);
  }
  free(h);
}

void drop_last_hist_command() {
  HISTORY = HISTORY->prev;
  clean_history(HISTORY->next);
  HISTORY->next = NULL;
}

void process_print(struct Command * c){
  if (c->arg1 == 0) return;
  struct Row *current = find_in_doc(c->arg1);
  for (int i = c->arg1; i <= c->arg2; i++){
    if (current != NULL && i != 0) {
      fputs(current->data, stdout);
      current = current->next;
    } else {
      fputs(".", stdout);
    }
    fputs("\n", stdout);
  }
}

void process_delete(struct Command * c){
  struct Row *head = find_in_doc(c->arg1-1);
  if (head == NULL || head->next == NULL){
    c->data = (char *)malloc(sizeof(char));
    *(c->data) = '\0';
    //    drop_last_hist_command();
    return;
  }
  struct Row *current = head->next;
  for (int i = c->arg1; i <= c->arg2 && current != NULL; i++){
    head->next = current->next;
    if (c->data == NULL){
      c->data = current->data;
    } else {
      int l1 = strlen(c->data);
      int l2 = strlen(current->data);
      c->data = realloc(c->data, (l1 + l2 + 2) * sizeof(char));
      strcat(c->data, current->data);
    }
    c->data = realloc(c->data, (strlen(c->data) + 2) * sizeof(char));
    strcat(c->data, "\n");
    free(current);
    NEXT_LINE--;
    current = head->next;
  }
}

// TODO refactor with above function
void clean_doc(struct Row * r){
  if (r == NULL) return;
  clean_doc(r->next);
  if (r->data != NULL){
    free(r->data);
  }
  free(r);
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
  char * old;

  c->data = (char *) malloc(sizeof(char));
  *(c->data) = '\0';

  if (c->arg1 > NEXT_LINE || c->arg1 == 0) {
    drop_last_hist_command();
    return;
  }
  
  for (int i = c->arg1; i <= c->arg2; i++){
    char * _ = fgets(str, MAX_LINE, fp_in);
    if(strlen(str) == 0) {
      // Drop some rows
      old = clean_doc_from(i);
      int l1 = strlen(c->data);
      int l2 = strlen(old);
      c->data = realloc(c->data, (l1 + l2 + 1) * sizeof(char));
      strcat(c->data, old);
      break;
    } else {
      // Remove \n
      str[strlen(str)-1] = '\0';
      old = insert_row(DOC_HEAD, i, str);
      if(old != NULL){
	// Changed lines
	int l1 = strlen(c->data);
	int l2 = strlen(old);
	c->data = realloc(c->data, (l1 + l2 + 2) * sizeof(char));
	strcat(c->data, old);
	strcat(c->data, "\n");
      }
    }
  }
  
  // I assume the commands are correct, I drop the point
  if(fp_in == stdin) {
    char * _ = fgets(str, MAX_LINE, fp_in);
  }
}
void process_insert(struct Command * c){
  struct Row * p = DOC_HEAD;
  struct Row * supp = NULL;
  int j = 1;
  for(; j<c->arg1; j++, p=p->next) { }
  supp = p->next;
  char * data = c->data;
  char str[MAX_LINE];
  j = 0;
  while(*data != '\0'){
    if (*data == '\n'){
      str[j] = '\0';
      NEXT_LINE++;
      p->next  = (struct Row *) malloc(sizeof(struct Row));
      p->next->next = NULL;
      p->next->data = (char *)malloc(sizeof(char) * (strlen(str)+ 1));// TODO this pattern in different places
      strcpy(p->next->data, str);
      j = 0;
      p = p->next;
    } else {
      str[j] = *data;
      j++;
    }
    data++;
  }
  strcpy(c->data, "");
  p->next = supp; 
}

void single_hist_mov(struct Command * c) {
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
      FILE * in = fmemopen(data, sizeof(char) * (strlen(data)+1), "r+");
      process_change(c, in);
      free(data);
      fclose(in);
    }
    break;
  case delete:
    if(strlen(c->data) == 0) {
      // Redoing
      process_delete(c);
    } else {
      // Undoing
      process_insert(c);
    }
    break;
  default:
    break;
  }
}

void process_undo(struct Command * c){
  for(int i=0; i<c->arg1 && HISTORY->prev != NULL; i++){
    single_hist_mov(HISTORY->c);
    HISTORY = HISTORY->prev;
  }
}

void process_redo(struct Command * c){
    for(int i=0; i<c->arg1; i++){
      if(HISTORY->next != NULL){
	HISTORY = HISTORY->next;
	single_hist_mov(HISTORY->c);
      }
  }
}

void cleanup(){
  clean_history(HISTORY_HEAD);
  clean_doc(DOC_HEAD);
}


void append_history(struct Command * c) {
  if (HISTORY->next != NULL){
    clean_history(HISTORY->next);
  }
  struct History * next = (struct History *) malloc(sizeof(struct History));
  next->next = NULL;
  next->prev = HISTORY;
  next->c = NULL; // TODO remove?
  HISTORY->next = next;
  HISTORY = next;
  HISTORY->c = c; // TODO  366
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
  HISTORY = (struct History *) malloc(sizeof(struct History));
  HISTORY->next = NULL;
  HISTORY->prev = NULL;
  HISTORY->c = NULL;
  HISTORY_HEAD = HISTORY;
}
void initialize_doc() {
  DOC_HEAD = (struct Row *) malloc(sizeof(struct Row));
  DOC_HEAD->next = NULL;
  DOC_HEAD->data = NULL;
}

int main() {
  struct Command * c;
  NEXT_LINE = 1;
  initialize_doc();
  inizialize_hist();
  do {
    c = read_command(stdin);
    process_command(c, stdin);
  } while(c->type != quit);
  cleanup();
}

