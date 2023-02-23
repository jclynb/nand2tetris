#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<assert.h>
#include<regex.h>
#include<stdbool.h>
#include<errno.h>

enum instructiontype{A, C, L, Comment};

struct symtab {
  struct symtab *next;
  char *symbol;
  int value;
};

struct symtab *lookup(struct symtab *first, char *s) {
/*
 * Lookup a symbol in symbol table `first`,
 * Returns the entry with the matching symbol if one exists.
 */
  for (struct symtab *np = first; np != NULL; np = np->next) {
    if (strcmp(s, np->symbol) == 0)
      return np; // Symbol found
  }
  return NULL;
}

struct symtab *insert(struct symtab *symtab, char *sym, int val) {
/*
 * Insert symbol/value pair
 */
  struct symtab *np;
  if ((np = lookup(symtab, sym)) == NULL) {
    np = (struct symtab *) malloc(sizeof(*np));
    if (np == NULL || (np->symbol = strdup(sym)) == NULL) {
      return NULL;
    }
    np->next = symtab;
  }
  np->value = val; // if lookup() succeeded, this updates the entry containing symbol
  return np;
}

void printsymtab(struct symtab *symtab) {
  while (symtab != NULL) {
    printf("symbol: %s\n", symtab->symbol);
    printf("value: %d\n", symtab->value);
    symtab = symtab->next;
  }
}

struct symtab *init_symtab() {
  struct symtab *s = malloc(sizeof(struct symtab));
  s->symbol = "R0";
  s->value = 0;
  s->next = NULL;
  s = insert(s, "R1", 1);
  s = insert(s, "R2", 2);
  s = insert(s, "R3", 3);
  s = insert(s, "R4", 4);
  s = insert(s, "R5", 5);
  s = insert(s, "R6", 6);
  s = insert(s, "R7", 7);
  s = insert(s, "R8", 8);
  s = insert(s, "R9", 9);
  s = insert(s, "R10", 10);
  s = insert(s, "R11", 11);
  s = insert(s, "R12", 12);
  s = insert(s, "R13", 13);
  s = insert(s, "R14", 14);
  s = insert(s, "R15", 15);
  s = insert(s, "SCREEN", 16384);
  s = insert(s, "KBD", 24576);
  s = insert(s, "SP", 0);
  s = insert(s, "LCL", 1);
  s = insert(s, "ARG", 2);
  s = insert(s, "THIS", 3);
  s = insert(s, "THAT", 4);
  return s;
}

struct symtab *init_desttab() {
  struct symtab *d = malloc(sizeof(struct symtab));
  d->symbol = "null";
  d->value = 0;
  d->next = NULL;
  d = insert(d, "M", 1);
  d = insert(d, "D", 2);
  d = insert(d, "MD", 3);
  d = insert(d, "A", 4);
  d = insert(d, "AM", 5);
  d = insert(d, "AD", 6);
  d = insert(d, "ADM", 7);
  return d;
};

struct symtab *init_comptab() {
  struct symtab *c = malloc(sizeof(struct symtab));
  c->symbol = "0";
  c->value = 0b101010;
  c->next = NULL;
  c = insert(c, "1", 0b111111);
  c = insert(c, "-1", 0b111010);
  c = insert(c, "D", 0b001100);
  c = insert(c, "A", 0b110000);
  c = insert(c, "M", 0b110000);
  c = insert(c, "!D", 0b001101);
  c = insert(c, "!A", 0b110001);
  c = insert(c, "!M", 0b110001);
  c = insert(c, "-D", 0b001111);
  c = insert(c, "-A", 0b110011);
  c = insert(c, "-M", 0b110011);
  c = insert(c, "D+1", 0b011111);
  c = insert(c, "A+1", 0b110111);
  c = insert(c, "M+1", 0b110111);
  c = insert(c, "D-1", 0b011111);
  c = insert(c, "A-1", 0b110010);
  c = insert(c, "M-1", 0b110010);
  c = insert(c, "D+A", 0b000010);
  c = insert(c, "D+M", 0b000010);
  c = insert(c, "D-A", 0b010011);
  c = insert(c, "D-M", 0b010011);
  c = insert(c, "A-D", 0b000111);
  c = insert(c, "M-D", 0b000111);
  c = insert(c, "D&A", 0b000000);
  c = insert(c, "D&M", 0b000000);
  c = insert(c, "D|A", 0b010101);
  c = insert(c, "D|M", 0b010101);
  return c;
};

struct symtab *init_jumptab() {
  struct symtab *j = malloc(sizeof(struct symtab));
  j->symbol = "null";
  j->value = 0;
  j->next = NULL;
  j = insert(j, "JGT", 1);
  j = insert(j, "JEQ", 2);
  j = insert(j, "JGE", 3);
  j = insert(j, "JLT", 4);
  j = insert(j, "JNE", 5);
  j = insert(j, "JLE", 6);
  j = insert(j, "JMP", 7);
  return j;
};

int trim_instruction(char *s) {
/*
 * Ends string when first whitespace is encounterd
 */
  for (int i = 0; i <= strlen(s); i++) {
    int result = isspace(s[i]);
    if (result) {
      s[i] = '\0';
      return 0;
    }
  } return 0;
}

enum instructiontype get_type(char c) {
/*
 * Determines instruction type (A command @R0, L (label), Comment //, or C command)
 */
  enum instructiontype type;
  if (c == '@') {
    type = A;
  } else if (c == '(') {
    type = L;
  } else if (c == '/') {
    type = Comment;
  } else {
    type = C;
  }
  return type;
}

bool get_ctype(char *buff) {
/*
 * Determines if the C_command is type dest=comp or comp;jump
 */
  char *ctype = strstr(buff, "="); // strstr returns a pointer to the first occurrence of "M"
  if (ctype) {
    return true;
  }
  return false;
}


char *parseAL(char *buff, enum instructiontype type) {
/*
 * Parses A (enum type 0) and L (enum type 2) commands
 * Removes '@' or '(' and ')'
 */
  char *returnstr = malloc(sizeof(buff));
  // A command
  if (type == 0) {
    strcpy(returnstr, buff + 1);
    return returnstr;
  }
  // L command
  else if (type == 2) {
    strncpy(returnstr, buff + 1, strlen(buff) - 2);
    return returnstr;
  }
  printf("No valid A or L command found: %s\n", buff);
  return NULL;
}

char *to_binary(int value, int size) {
/*
 * Int to binary conversion
 */
  char *binary = malloc(sizeof(size));
  char *ptr = binary + size - 1;
  memset(binary, '\0', size);
  while (*ptr >= *binary) {
    if (*ptr == 15) { // A commands start with 0
      *ptr = '0';
      ptr--;
    }
    int m = value % 2;
    value = value / 2;

    if (m & 1) {
      *ptr = '1';
      ptr--;
    } else {
      *ptr = '0';
      ptr--;
    }
  }
  return binary;
}

char *c_to_binary(char *buff, bool ctype) {
 /*
  * Translates a C_command to binary
  * C_commands start with "111"
  * The 4th bit is determined by whether comp includes "M"
  * The final bits are determined by "comp", "dest", and "jump" which have interger values stored in symbol tables
  * Comp, dest, and jump values are translated into binary using to_binary
  */
  char *dest;
  char *comp;
  char *jump;

  if (ctype) {
  // If dest=comp type
    char *token = strtok(buff, "=");
    dest = token;
    token = strtok(0, "=");
    comp = token;
    jump = "null";
  } else {
    // Else comp;jump type
    char *token = strtok(buff, ";");
    dest = "null";
    comp = token;
    token = strtok(0, ";");
    jump = token;
  }

  char *binary = malloc(20);
  memset(binary, '1', 3);

  // Initalize symbol tables for dest/jump/comp values
  struct symtab *d = init_desttab();
  struct symtab *c = init_comptab();
  struct symtab *j = init_jumptab();
  struct symtab *dest_entry;
  struct symtab *comp_entry;
  struct symtab *jump_entry;

  // COMP
  if ((comp_entry = lookup(c, comp)) == NULL) {
    printf("Error: not a valid comp: %s\n", comp);
    return NULL;
  }

  int c_int = comp_entry->value;
  char *c_bin = to_binary(c_int, 6);
  // Set the a-bit
  char *set_aval = strstr(comp, "M"); // strstr returns a pointer to the first occurrence of "M"
  if (set_aval) {
    memset(binary + 3, '1', 1);
  } else {
    memset(binary + 3, '0', 1);
  }
  memcpy(binary + 4, c_bin, 6);

  // DEST
  if ((dest_entry = lookup(d, dest)) == NULL) {
    printf("Error: not a valid dest: %s\n", dest);
    return NULL;
  }

  int d_int = dest_entry->value;
  char *d_bin = to_binary(d_int, 3);

  memcpy(binary + 10, d_bin, 3);

  // JUMP
  if ((jump_entry = lookup(j, jump)) == NULL) {
    printf("Error: not a valid jump: %s\n", jump);
    return NULL;
  }

  int j_int = jump_entry->value;
  char *j_bin = to_binary(j_int, 3);
  memcpy(binary + 13, j_bin, 3);

  return binary;
}

struct symtab *getsymtab(char *argv[]) {
/*
 * First pass through file: constructs a symbol table of a program's predefined symbols and label symbols
 * init_symtab first initalizes the symbol table with predefined symbols (such as R0->0, R1->1, etc)
 * The .asm file is then scanned line by line to look for labels (itype 2)
 * linenum counts the number of lines (excluding comments) to be used as values in the symbol table
 * Label symbols map to corresponding line number in file
 */
  struct symtab *symtab = init_symtab();

  FILE *fp = fopen(*++argv, "r");
  if (fp == NULL) {
    printf("Input error\n");
    return 0;
  }

  char buff[500];
  int linenum = 0;

  while(fscanf(fp, "%[^\n] ", buff) == 1) {
    // read until a newline is found
    trim_instruction(buff); // Trims extra whitespace and comments
    enum instructiontype itype = get_type(buff[0]);

    if (itype == 2) {
      // If Label Instruction, parse label, add label and line number to symbol tree
      char *symbol;
      if ((symbol = parseAL(buff, itype)) == NULL) {
        return 0;
      }
      if (lookup(symtab, symbol) == NULL) {
        symtab = insert(symtab, symbol, linenum);
        linenum++;
      }
    } else if (itype != 3) {
      // If the line is not a comment, increase the line number
      linenum++;
    }
  }
  fclose(fp);
  return symtab;
}

int main(int argc, char *argv[]) {
  /*
   * Opens a .asm file and translates each line into 16 bit binary code, which is written to a .hack file
   * getsymtab initializes the symbol table (linked list struct symtab), and inserts new labels when a '(' is encountered
   * During the second pass, a_commands (itype 0) and c_commands (itype 1) are translated into binary using to_binary (for ints) and c_to_binary (for parsing dest/comp/jump instructions)
   * itypes come from "enum instructiontype", which include A, C, L, comment (A = a_command, C = c_command, L = label, and comments, which are ignored)
   */
  if (argc < 1) {
    printf("Input error\n");
    return 0;
  }

  struct symtab *symtab = getsymtab(argv);

  FILE *fp = fopen(*++argv, "r");
  if (fp == NULL) {
    printf("Input error\n");
    return 0;
  }

  FILE *out = fopen("out.hack", "w");
  char buff[500];
  int varnum = 16;
  while(fscanf(fp, "%[^\n] ", buff) == 1) {
    trim_instruction(buff); // trim whitespace and comments
    enum instructiontype itype = get_type(buff[0]);

    // If symbol (ex: @sum), a_to_binary(varnum)
    // If num (ex: @0), a_to_binary(num)
    if (itype == 0) {
    // If the line is an A_command
      struct symtab *s;
      char *variable;
      if ((variable = parseAL(buff, itype)) == NULL) {
        return 0;
      }
      if (isdigit(variable[0])) {
        int v = atoi(variable);
        char *a_binary = to_binary(v, 16);
        fputs(a_binary, out);
        fputs("\n", out);
        free(a_binary);
      }
      else if ((s = lookup(symtab, variable)) == NULL) {
        symtab = insert(symtab, variable, varnum);
        char *a_binary = to_binary(varnum, 16);
        varnum++;
        fputs(a_binary, out);
        fputs("\n", out);
        free(a_binary);
      } else {
        char *a_binary = to_binary(s->value, 16);
        fputs(a_binary, out);
        fputs("\n", out);
        free(a_binary);
      }
    } else if (itype == 1) {
    // If the line is a C_command
      bool ctype = get_ctype(buff);
      char *c_binary;
      if ((c_binary = c_to_binary(buff, ctype)) == NULL) {
        return 0;
      }
      fputs(c_binary, out);
      fputs("\n", out);
      free(c_binary);
    }
  }
  printf("code saved in out.hack :)\n");
  fclose(out);
  fclose(fp);
  return 0;
}
