#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_C_CHAR 20 // TODO this will likely fail
#define MAX_LINE 1040

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

void print_row(struct Row *r){  
  if(r != NULL){
    printf(" %s --> \n", r->data);
    print_row(r->next);
  } else {
    printf(" (null) \n\n\n");
  }
}
void print_hist(struct History *h) {
  printf("Hist address: %p ||", h);
  if(h){
    if(h->c){
      printf("type: %d - data: %s --> \n", h->c->type, h->c->data);
    } else {
     printf(" ( ) --> \n");
    }
    print_hist(h->next);
  } else {
    printf(" (null) \n\n\n");
  }
}

void print_doc(){
  printf("DOCUMENT:\n");
  print_row(DOC_HEAD);
}

void print_history() {
  printf("HISTORY:\n");
  print_hist(HISTORY_HEAD);
  printf("History (current) address: %p\n", HISTORY);
}



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
      //      printf("New text\n");
      row->data = (char *) malloc(sizeof(char) * (strlen(data) + 1));
    } else {
      old = row->data;
      //      printf("Changin text\n");
      row->data = (char *) malloc(sizeof(char) * (strlen(data) + 1));
    }
    strcpy(row->data, data);
    return old;
  }
  if (index > 0 && row->next == NULL) {
    //    printf("New row\n");
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
  if(fgets(str, MAX_C_CHAR, fp) == NULL){
    exit(1);
  }
  struct Command * c = parse_command(str);
  return c;
}
void clean_command(struct Command *c) {
  if(c == NULL)
    return;
  //  printf("Cleaning command: %d,%d - type: %d - data %s \n",c->arg1, c->arg2, c->type, c->data);

  if (c->data != NULL){  
      free(c->data);
    }
  free(c);
}

void clean_history(struct History * h){
  if (h == NULL) return;
  clean_history(h->next);
  clean_command(h->c);
  free(h);
}

void drop_last_hist_command() {
  HISTORY = HISTORY->prev;
  clean_history(HISTORY->next);
  HISTORY->next = NULL;
}

void process_print(struct Command * c){
  if (c->arg1 == 0 && c->arg2 != 0 ){
      return;
  }

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
    if(c->data == NULL) {
      c->data = (char *)malloc(sizeof(char));
    *(c->data) = '\0';
    //        drop_last_hist_command();
    }
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
      free(current->data);
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
  char * dropped_string = (char *)malloc(sizeof(char)*MAX_LINE);
  strcpy(dropped_string, "");
  while(p != NULL) {
    supp = p->next;
    if (i==j){
      // Update tail
      p->next = NULL;
    } else if (j >= i){
      int l1 = strlen(p->data);
      int l2 = strlen(dropped_string);
      dropped_string = realloc(dropped_string, (l1 + l2 + 2) * sizeof(char));
      strcat(dropped_string, p->data);
      strcat(dropped_string, "\n");
      free(p->data);
      free(p);
      NEXT_LINE--;
    } 
    p = supp;      
    j++;
  }
  return dropped_string;
}

void process_change(struct Command *c, FILE *fp_in){
  char str[MAX_LINE];
  char * old;
  if(c->data == NULL){
    c->data = (char *) malloc(sizeof(char));
  }
  *(c->data) = '\0';
  if (c->arg1 > NEXT_LINE || c->arg1 == 0) {
    drop_last_hist_command(); // TODO
    return;
  }

  for (int i = c->arg1; i <= c->arg2; i++){
    if(!fgets(str, MAX_LINE, fp_in)) {
      // Drop some rows
      old = clean_doc_from(i);
      int l1 = strlen(c->data);
      int l2 = strlen(old);
      c->data = realloc(c->data, (l1 + l2 + 2) * sizeof(char));
      strcat(c->data, old);
      free(old);
      break;
    } else {
      //    printf("------------------------------------------------------------>\n");
      // Remove \n
      if(strlen(str) > 0) 
	str[strlen(str)-1] = '\0';
      old = insert_row(DOC_HEAD, i, str);
      if(old != NULL){
	// Changed lines
	int l1 = strlen(c->data);
	int l2 = strlen(old);
	c->data = realloc(c->data, (l1 + l2 + 2) * sizeof(char));
	strcat(c->data, old);
	strcat(c->data, "\n");
	free(old);
      }
    }
  }
  
  // I assume the commands are correct, I drop the point
  if(fp_in == stdin) {
    if(fgets(str, MAX_LINE, fp_in) == NULL){
      exit(1);
    }
  }
}
void process_insert(struct Command * c){
  struct Row * p = DOC_HEAD;
  struct Row * supp = NULL;
  int j = 1;
  for(; j<c->arg1; j++, p=p->next) { }
  supp = p->next;
  char * data = c->data;
  char str[MAX_LINE] = "";
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
  //  printf("Redoing/undoing command: %d,%d - type: %d - data %s \n",c->arg1, c->arg2, c->type, c->data);
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
      free(c->data);
      c->data = dc.data;
    } else {
      // Change lines
      //      printf("COMMAND DATA: %s\n", c->data);
      FILE * in = fmemopen(NULL, sizeof(char) * strlen(c->data), "r+");
      fputs(c->data, in);
      rewind(in);
      process_change(c, in);
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
    //    printf("Undoing\n");
    single_hist_mov(HISTORY->c);
    HISTORY = HISTORY->prev;
  }
}

void process_redo(struct Command * c){
    for(int i=0; i<c->arg1; i++){
      if(HISTORY->next != NULL){
	//	printf("Redoing\n");
	HISTORY = HISTORY->next;
	single_hist_mov(HISTORY->c);
      }
  }
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

int process_command(struct Command * c, FILE *fp_in){
  int q = 0;
  switch(c->type) {
  case quit:
    clean_command(c);
    q = 1;
    break;
  case print:
    process_print(c);
    clean_command(c);
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
    clean_command(c);
    break;
  case undo:
    process_undo(c);
    clean_command(c);
    break;
  }
  return q;
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

void clean_history_doc(){
  //  printf("History: %p, History head: %p", HISTORY, HISTORY_HEAD);
  clean_history(HISTORY_HEAD);
  clean_doc(DOC_HEAD);
}

int main() {
  struct Command * c;
  NEXT_LINE = 1;
  initialize_doc();
  inizialize_hist();
  int q = 0;
  do {
    /* print_doc(); */
    /* printf("\n\n\n"); */
    /* print_history(); */
    /* printf("Next line: %d\n", NEXT_LINE); */
    c = read_command(stdin);
    //    print_history();
    q = process_command(c, stdin);
  } while(!q);
  clean_history_doc();  
}

