#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define MAX_C_CHAR 10
#define MAX_LINE 1025
//#define MAX_LINE_PER_FILE 10000
#define DOC_BATCH 100

/*
  ------  DATA STRUCTURES ------
*/

struct Command {
  int arg1;
  int arg2;
  char type;
  char ** data;
  bool is_data_present;
} ;


struct History {
  struct Command * c;
  struct History * next;
  struct History * prev;
};

struct History * HISTORY = NULL;
struct History * HISTORY_HEAD = NULL;
char ** DOC = NULL;
int NEXT_LINE;
//FILE * in_mem_stdin = NULL;

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
  NEXT_LINE = 1;
  DOC = (char **) malloc(sizeof(char *) * DOC_BATCH);
}

/*
  ------  PRINTING UTILITIES ------
*/

void print_hist(struct History *h) {
  printf("Hist address: %p ||", h);
  if(h){
    if(h->c){
      printf("%d,%d%c, is_data_present %d\n", h->c->arg1, h->c->arg2, h->c->type, h->c->is_data_present);
      if (h->c->is_data_present) {
	printf("Data: \n");
	for(int i = 0; h->c->data[i] != NULL ; i++) {
	  printf("%p | ", h->c->data[i]);
	  for (int k=0; k<NEXT_LINE; k++){
	    if (h->c->data[i] == DOC[k]){
	      printf("\n\n-------------------------------->\n");
	      printf("%p\n", h->c->data[i]);
	      printf("\n\n-------------------------------->\n");
	      exit(42);
	    }
	}

	}
	printf("\n");
      }
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
  for(int i = 0; i < NEXT_LINE; i++) {
    if(DOC[i] == NULL)
      printf("(NULL)\n");
    else 
      printf("%s", DOC[i]);
  }
  printf("END DOCUMENT:\n");
}



void print_doc_pointers() {
  printf("DOCUMENT:\n");
  for(int i = 0; i < NEXT_LINE; i++) {
    printf("%d: %p\n", i, DOC[i]);
  }
  printf("END DOCUMENT:\n");
}

void print_history() {
  printf("HISTORY:\n");
  print_hist(HISTORY_HEAD);
  if (HISTORY->c != NULL) {
    printf("History (current) address: %p, command: %d,%d%c is_data_present %d\n", HISTORY, HISTORY->c->arg1, HISTORY->c->arg2, HISTORY->c->type, HISTORY->c->is_data_present);
    if(HISTORY->c->is_data_present) {
      printf("Data:\n");
      for(int i = 0; ; i++){
	if(HISTORY->c->data[i] == NULL)
	  break;
	printf("%p\n", HISTORY->c->data[i]);
      }
    }
  } else {
    printf("History current is HEAD");
  }
}


/*
  ------  CLEAN-UP UTILITIES ------
*/

void clean_doc(){
  for (int i=0; i<NEXT_LINE;i++){
    free(DOC[i]);
  }
  free(DOC);
}


void clean_command(struct Command *c) {
  if(c == NULL)
    return;
  if (c->is_data_present){
    for(int i=0;; i++) {
      if (c->data[i] == NULL) {
	break;
      }
      //      printf("----------> Freeing: %p\n", c->data[i]);
      free(c->data[i]);
      //      printf("----------> Freed: %p\n", c->data[i]);
    }
    c->is_data_present = false;
    free(c->data);
  }
  free(c);
  //  printf("----------> Command cleanded\n\n\n");
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
  clean_doc();
}

void process_print(struct Command * c){
  if (c->arg1 == 0) {
    if (c->arg2 != 0 ){
      return;
    }
    fputs(".\n", stdout);
      return;
  }
  for (int i = c->arg1; i <= c->arg2; i++){
    if(i >= NEXT_LINE) {
      fputs(".\n", stdout);      
    } else {
      if(DOC[i])
      fputs(DOC[i], stdout);
    }
  }
}

void process_delete(struct Command * c){
  // This function can be called in these cases:
  //   - unset some on j DOC lines with 1 < j < nlines 
  //   - set j lines on c->data
  //   - shift all rows (from the next of the dropped one to NEXT_LINE

  //   - c->is_data_present == true in the end
  //   - remove j from NEXT_LINE
  int nlines = c->arg2 - c->arg1 + 1;
  char * dropped_strs[nlines];
  int dropped_nlines = 0;

  if(c->arg1 >= NEXT_LINE) {
    // Trying to delete not existing lines -> do nothing
    c->data = (char **) malloc(sizeof(char *));
    c->data[0] = NULL;
    c->is_data_present = true;
    return;
  }
  
  // Initialize to NULL str pointers
  for(int j = 0; j < nlines; dropped_strs[j] = NULL, j++) { }

  // Delete lines from document
  int j = c->arg1;
  
  // Delete at most nlines
  for (; j <= c->arg2 && j < NEXT_LINE; j++) {
    dropped_strs[j - c->arg1] = DOC[j];
    DOC[j] = NULL;
  }
  dropped_nlines = j - c->arg1;

  // Shift lines (where necessary)
  while(DOC[j] != NULL && j < NEXT_LINE) {
    DOC[j - dropped_nlines] = DOC[j];
    j++;
  }

  //  printf("DELETE: dropped str ptrs (dropped %d)\n", dropped_nlines);
  // Adding data on command
  c->data = (char **) malloc(sizeof(char *) * (dropped_nlines+1));
  for (j = 0; j < dropped_nlines; j++) {
    //printf("%p\n", dropped_strs[j]);
    c->data[j] = dropped_strs[j];
  }
  c->data[j] = NULL;
  c->is_data_present = true;
  
  // Updating NEXT_LINE
  NEXT_LINE -= dropped_nlines;
}


void process_change(struct Command *c, FILE *fp_in){
  // Drop j lines with 0 < j < nlines
  FILE * supp = fp_in;   // TODO 1,4c cause fp_in to nil for some reason
  int nlines = c->arg2 - c->arg1 + 1;
  char * dropped_strs[nlines];
  int dropped_nlines = 0;
  // Initialize to NULL str pointers
  for(int j = 0; j < nlines; dropped_strs[j] = NULL, j++) { }

  // Invalid change
  if (c->arg1 > NEXT_LINE || c->arg1 == 0) {
    drop_last_hist_command();
    return;
  }
  
  //  printf("CHANGE: dropped str ptrs: \t\t\t");
  // Drop rows if needed
  if(c->arg1 < NEXT_LINE) {
    int j = c->arg1;
    for (; j <= c->arg2 && j < NEXT_LINE; j++) {
      //  printf("%p | ", DOC[j]);
      dropped_strs[j - c->arg1] = DOC[j];
    }
    dropped_nlines = j - c->arg1;
    //    printf ("(dropped lines: %d) \n", dropped_nlines);
  }
  
  //  printf("CHANGE: added str ptrs: \t\t\t");
  if (supp != NULL) {
    // NOT UNDO OR REDO
    // change text with fp
    // Change for the first time
    // Getting the string (it constains \n as sep)
    for(int j = c->arg1;  j <= c->arg2; j++) {
      DOC[j] = (char *)malloc(sizeof(char) * MAX_LINE);
      fgets(DOC[j], MAX_LINE, supp);
      //      printf("%p | ", DOC[j]);
    }
    // Reading the point
    char point[3]; // ".\n"
    fgets(point, 3, supp);
  } else if (c->is_data_present) {
    // UNDO OR REDO
    // change text with c->data
    int j;
    for(j = c->arg1;  j <= c->arg2 && c->data[j-c->arg1] != NULL; j++) {
      DOC[j] = c->data[j-c->arg1];
      //      printf("%p | ", DOC[j]);
    }
    nlines = j - c->arg1;
  } else {
    exit(3);
  }
  //  printf ("\n");

  //  printf("CHANGE: dropped (stored) str ptrs: \t\t\t");
  // It there is some data, I used it to save it in the document
  // If it was a first time change on a new line, this is already false buy anyway
  free(c->data);
  c->is_data_present = false;
  
  int j;
  if(c->arg1 < NEXT_LINE) {
    // Some line is dropped, i add the data to the comment
    c->data = (char **) malloc(sizeof(char *) * (dropped_nlines+1));
    for (j = 0; j < dropped_nlines; j++) {
      c->data[j] = dropped_strs[j];
      //  printf("%p | ", dropped_strs[j]);
    }
    c->data[j] = NULL;
    c->is_data_present = true;
  }
  //  printf("\n\n\n");

  NEXT_LINE += nlines - dropped_nlines;
}

      
void single_hist_mov(struct Command * c) {
  switch(c->type) {
  case 'c':
    if(c->is_data_present) {
      // Change altered some lines (stored in c->alt)
      process_change(c, NULL);
    } else {
      // Change only added lines -> revert by deleting them
      process_delete(c);
    }
    break;
  case 'd':
    if(c->is_data_present) {
      // Undoing deletion
      // Free lines
      int prev_dropped_nlines;
      for(prev_dropped_nlines = 0; c->data[prev_dropped_nlines] != NULL ; prev_dropped_nlines++) {
      }
      
      // Free j lines in c->arg1 (if needed)
      if(c->arg1 < NEXT_LINE) {
	for(int j=NEXT_LINE-1;j>=c->arg1;j--) {
	  DOC[j+prev_dropped_nlines] = DOC[j];
	}
      }

      // Insert lines saved in c->arg1
      for(int j = 0; c->data[j] != NULL; j++) {
	//printf("i: %d, j: %d", i, j);
	DOC[j+c->arg1] = c->data[j];
      }
      
      c->is_data_present = false;
      NEXT_LINE += prev_dropped_nlines;
    } else {
      // Redoing deletion or "useless" delete
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
  c->is_data_present = false;
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

/* FILE * read_file_in_memory(FILE * fp){ */
/*   int s = (MAX_LINE + 1) * MAX_LINE_PER_FILE;   */
/*   char * text = (char *)malloc(sizeof(char) * s); */
/*   fread(text, sizeof(char), s, fp); */
/*   s = strlen(text) + 1; */
/*   FILE * out = fmemopen(NULL, sizeof(char) * s, "w+"); */
/*   fputs(text, out); */
/*   rewind(out); */
/*   free(text); */
/*   return out; */
//}

int main() {
  struct Command * c;
  
  //  in_mem_stdin = read_file_in_memory(stdin);
  in_mem_stdin = stdin;

  initialize_doc();
  inizialize_hist();
  int q = 0;
  int dim = 1;
  do {
    c = read_command(in_mem_stdin);
    //    printf("---NEXT COMMAND (%d,%d%c) ------\n", c->arg1, c->arg2, c->type);
    q = process_command(c, in_mem_stdin);
    /* print_doc_pointers(); */
    /* printf("Next line: %d\n", NEXT_LINE); */
    /* print_history(); */
    /* printf("---------\n\n\n\n\n\n\n"); */
    if (NEXT_LINE > dim *DOC_BATCH - DOC_BATCH/10) {
      DOC = (char **) realloc(DOC, sizeof(char *) * DOC_BATCH);
      dim++;
    }
  } while(!q);
  clean_history_doc();  
}
