#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_C_CHAR 20 // TODO this will likely fail
#define MAX_LINE 1040
#define MAX_LINE_PER_FILE 10000

/*
  ------  DATA STRUCTURES ------
*/


struct Command {
  int arg1;
  int arg2;
  char type;
  struct Row * alt;
} ;


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
int NEXT_LINE;
FILE * in_mem_stdin = NULL;

/*
  ------  INIT UTILITIES ------
*/

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

/*
  ------  PRINTING UTILITIES ------
*/

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
      printf("type: %d\n", h->c->type);
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


/*
  ------  CLEAN-UP UTILITIES ------
*/

void clean_doc(struct Row * r){
  if (r == NULL) return;
  clean_doc(r->next);
  if (r->data != NULL){
    free(r->data);
  }
  free(r);
}


void clean_command(struct Command *c) {
  if(c == NULL)
    return;
  if (c->alt != NULL) {
    clean_doc(c->alt);    
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


void clean_history_doc(){
  clean_history(HISTORY_HEAD);
  clean_doc(DOC_HEAD);
}

/*
  ------ ROW/DOCUMENT UTILITIES ------
 */


struct Row * create_doc(char *str) {
  char s[2] = "\n";
  char *tok = strtok(str, s);
  struct Row * d = (struct Row *)malloc(sizeof(struct Row));
  d->data = tok;
  struct Row * supp = d;
  while(tok != ".") {
    d->next = (struct Row *)malloc(sizeof(struct Row));
    d->next->data = tok;
    d->next->next = NULL;
    d = d->next;
    tok = strtok(NULL, s);
  }
  return supp;
}

struct Row * change_new(struct Command *c, FILE *fp_in){
  // Change for the first time
  // Returns the dropped document (if present)
  int nlines = c->arg2 - c->arg1 + 1;
  char * str = (char *)malloc(sizeof(char) *MAX_LINE * nlines);
  *str = '\0';
  char line[MAX_LINE];
  struct Row * new_doc = NULL;
  // Getting the string (it constains \n as sep
  while(fgets(line, MAX_LINE, fp_in) && nlines >= 0) {
    strcat(str, line);
    nlines--;
  }
  // Insert new doc
  if(strlen(str) > 0) {
    return create_doc(str);
  } else {
    printf("str: %s\nline: %s\n", str, line);
    exit(2);
  }
  free(str);
}

struct Row * drop_rows_from_to(int from, int to){
  // from: 2 
  // to: 3
  // doc: (1) -> (a) -> (b) -> (2) -> (3)
  //
  // result or drop_rows_from_to(2, 3): (1) -> (2) -> (3)
  struct Row * p = DOC_HEAD;
  struct Row * supp = NULL;
  struct Row * drop = NULL;
  int i = 1;
  for(; i<from ; i++, p=p->next) { }
  drop = p->next;
  supp = p;
  for(; i<=from && p != NULL; i++, p=p->next) { }
  if(p == NULL){
    // To is over the last element (e.g. deletion of lines over the last one)
    supp->next = NULL;
  } else{
    // 
    supp->next = p->next;
    p->next = NULL;
  }
  NEXT_LINE -= to - i + 1;
  return drop;
}

void insert_row_in(struct Row * in, int index){
  // in: (a) -> (b)
  // doc: (1) -> (2) -> (3)
  //
  // result or insert_row_in(in, 2): (1) -> (a) -> (b) -> (2) -> (3)
  struct Row * p = DOC_HEAD;
  struct Row * supp = NULL;
  struct Row * supp2 = NULL;
  int i = 1;
  for(; i<index ; i++, p=p->next) { }
  supp = p->next;
  p->next = in;
  for(i=0; p; supp2 = p, p=p->next, i++) { }
  supp2->next = supp;
  NEXT_LINE += i;
}

/*
  ------  SINGLE COMMAND ROUTINES ------
*/


void process_print(struct Command * c){
  if (c->arg1 == 0 && c->arg2 != 0 ){
      return;
  }
  int i = 1;
  struct Row *current = DOC_HEAD;
  for (; current != NULL && i <= c->arg1; i++, current = current->next) { }
  for (i = c->arg1; i <= c->arg2; i++){
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
  struct Row * drop = drop_rows_from_to(c->arg1, c->arg2);  
  if(c->alt == NULL) {
    // First time deletion
    c->alt = drop;
  } else {
    exit(3);
  }
}


void process_change(struct Command *c, FILE *fp_in){
  struct Row * doc = NULL;
  struct Row * dropped_doc = NULL;
  if (c->arg1 > NEXT_LINE || c->arg1 == 0) {
    drop_last_hist_command();
    return;
  }
  // Drop rows if needed
  if(c->arg1 < NEXT_LINE) {
    dropped_doc = drop_rows_from_to(c->arg1, c->arg2);
  }

  if (fp_in != NULL) {
    // NOT UNDO OR REDO
    // change text with fp
    doc = change_new(c, fp_in);
  } else {
    // UNDO OR REDO
    // change text with c->alt
    doc = c->alt;
  }
  insert_row_in(doc, c->arg1);
  c->alt = dropped_doc;
  NEXT_LINE += c->arg2 - c->arg1 + 1;
}

      
void single_hist_mov(struct Command * c) {
  switch(c->type) {
  case 'c':
    if(c->alt == NULL) {
      // Change only added lines -> revert by deleting them
      process_delete(c);
    } else {
      // Change altered some lines (stored in c->alt)
      process_change(c, NULL);
    }
    break;
  case 'd':
    if(c->alt != NULL) {
      // Undoing deletion
      insert_row_in(c->alt, c->arg1);
      c->alt = NULL;
    } else {
      // Redoing deletion
      process_delete(c);
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
/*
p  ------  HISTORY UTILITIES ------
 */

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

/*
  ------  COMMANDS UTILITIES ------
*/

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
  c->type = *str;
  c->alt = NULL;
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


int process_command(struct Command * c, FILE *fp_in){
  int q = 0;
  switch(c->type) {
  case 'q':
    clean_command(c);
    q = 1;
    break;
  case 'p':
    process_print(c);
    clean_command(c);
    break;
  case 'c':
    append_history(c);
    process_change(c, fp_in);
    break;
  case 'd':
    append_history(c);
    process_delete(c);
    break;
  case 'r':
    process_redo(c);
    clean_command(c);
    break;
  case 'u':
    process_undo(c);
    clean_command(c);
    break;
  }
  return q;
}

/*
  ------  MAIN RELATED ------
 */

FILE * read_file_in_memory(FILE * fp){
  int s = (MAX_LINE + 1) * MAX_LINE_PER_FILE;  
  char * text = (char *)malloc(sizeof(char) * s);
  fread(text, sizeof(char), s, fp);
  s = strlen(text) + 1;
  FILE * out = fmemopen(NULL, sizeof(char) * s, "w+");
  fputs(text, out);
  rewind(out);
  free(text);
  return out;
}

int main() {
  struct Command * c;
  NEXT_LINE = 1;
  //  in_mem_stdin = read_file_in_memory(stdin);
  in_mem_stdin = stdin;
  initialize_doc();
  inizialize_hist();
  int q = 0;
  do {
    c = read_command(in_mem_stdin);
    q = process_command(c, in_mem_stdin);
  } while(!q);
  clean_history_doc();  
}

