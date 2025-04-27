
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 

 #define MAX_NONTERMS  30
 #define MAX_PRODS     30
 #define MAX_SYMBOLS   30
 #define MAX_CHAR      100
 #define MAX_STACK     200
 #define EPSILON       '~'        /* one-char ε */
 

 typedef struct {
     char head;
     int  prodCount;
     char productions[MAX_PRODS][MAX_SYMBOLS];
 } GrammarRule;
 
 typedef struct {
     char nonTerm;
     char firstSet[MAX_SYMBOLS];
     char followSet[MAX_SYMBOLS];
 } Sets;

 GrammarRule grammar[MAX_NONTERMS];
 Sets        setsTable[MAX_NONTERMS];
 int         grammarCount = 0;
 
 char LL1Table[MAX_NONTERMS][MAX_SYMBOLS][MAX_SYMBOLS];
 
 static char terminals[MAX_SYMBOLS];
 static int  terminalCount = 0;
 

 int  findNonTermIndex(char);
 void addToSet(char *, char);
 int  containsSymbol(const char *, char);
 int  isNonTerminal(char);
 int  isTerminal(char);
 
 void readGrammarFromFile(const char *);
 void displayGrammar(const char *);
 void leftFactorGrammar(void);
 void removeLeftRecursion(void);
 void initSetsTable(void);
 void computeFirstSets(void);
 void computeFollowSets(void);
 void buildLL1Table(void);
 void displaySets(void);
 void displayLL1Table(void);
 

 static void parseInputFile(const char *);
 static int  parseLine(const char *, int);
 

 int findNonTermIndex(char head)
 {
     for (int i = 0; i < grammarCount; ++i)
         if (grammar[i].head == head) return i;
     return -1;
 }
 
 void addToSet(char *set, char symbol)
 {
     if (!containsSymbol(set, symbol)) {
         size_t len = strlen(set);
         set[len] = symbol;
         set[len + 1] = '\0';
     }
 }
 
 int containsSymbol(const char *set, char symbol)
 {
     return strchr(set, symbol) != NULL;
 }
 
 int isNonTerminal(char c) { return (c >= 'A' && c <= 'Z'); }
 int isTerminal(char c)    { return !isNonTerminal(c) && c != EPSILON && c; }
 

 void readGrammarFromFile(const char *fname)
 {
     FILE *fp = fopen(fname, "r");
     if (!fp) { perror(fname); exit(1); }
 
     char buf[MAX_CHAR];
     while (fgets(buf, sizeof buf, fp)) {
         buf[strcspn(buf, "\r\n")] = '\0';
         if (strlen(buf) < 3) continue;
 
         char *arrow = strstr(buf, "->");
         if (!arrow) continue;
 
         char head = buf[0];
         int idx   = findNonTermIndex(head);
         if (idx == -1) {
             idx = grammarCount++;
             grammar[idx].head = head;
             grammar[idx].prodCount = 0;
         }
 
         char *rhs = arrow + 2;
         char *tok = strtok(rhs, "|");
         while (tok) {
             strcpy(grammar[idx].productions[grammar[idx].prodCount++], tok);
             tok = strtok(NULL, "|");
         }
     }
     fclose(fp);
 }
 
 void displayGrammar(const char *msg)
 {
     printf("\n%s\n", msg);
     for (int i = 0; i < grammarCount; ++i) {
         printf("%c -> ", grammar[i].head);
         for (int j = 0; j < grammar[i].prodCount; ++j) {
             printf("%s", grammar[i].productions[j]);
             if (j < grammar[i].prodCount - 1) printf(" | ");
         }
         printf("\n");
     }
 }
 

 static void commonPrefix(const char *s1, const char *s2, char *out)
 {
     int i = 0;
     while (s1[i] && s2[i] && s1[i] == s2[i]) { out[i] = s1[i]; ++i; }
     out[i] = '\0';
 }
 
 void leftFactorGrammar(void)
 {
     int changed = 1;
     while (changed) {
         changed = 0;
         for (int i = 0; i < grammarCount; ++i) {
             for (int p1 = 0; p1 < grammar[i].prodCount; ++p1) {
                 for (int p2 = p1 + 1; p2 < grammar[i].prodCount; ++p2) {
 
                     char pre[MAX_SYMBOLS];
                     commonPrefix(grammar[i].productions[p1],
                                  grammar[i].productions[p2], pre);
                     if (!*pre) continue;
 
                     /* new non-terminal */
                     char newHead = 'X' + i;            /* quick but OK for small grammars */
                     if (!isNonTerminal(newHead) || findNonTermIndex(newHead) != -1)
                         continue;
 
                     int newIdx = grammarCount++;
                     grammar[newIdx].head = newHead;
                     grammar[newIdx].prodCount = 0;
 
                     /* leftovers */
                     char left1[MAX_SYMBOLS], left2[MAX_SYMBOLS];
                     strcpy(left1, grammar[i].productions[p1] + strlen(pre));
                     strcpy(left2, grammar[i].productions[p2] + strlen(pre));
                     if (!*left1) { left1[0] = EPSILON; left1[1] = '\0'; }
                     if (!*left2) { left2[0] = EPSILON; left2[1] = '\0'; }
 
                     strcpy(grammar[newIdx].productions[0], left1);
                     strcpy(grammar[newIdx].productions[1], left2);
                     grammar[newIdx].prodCount = 2;
 
                     /* rewrite original */
                     sprintf(grammar[i].productions[p1], "%s%c", pre, newHead);
 
                     /* delete p2 */
                     for (int k = p2; k < grammar[i].prodCount - 1; ++k)
                         strcpy(grammar[i].productions[k],
                                grammar[i].productions[k + 1]);
                     --grammar[i].prodCount;
 
                     changed = 1;
                     goto restart;
                 }
             }
         }
     restart: ;
     }
 }
 

 void removeLeftRecursion(void)
 {
     for (int i = 0; i < grammarCount; ++i) {
         char A = grammar[i].head;
 
         char alpha[MAX_PRODS][MAX_SYMBOLS], beta[MAX_PRODS][MAX_SYMBOLS];
         int  ac = 0, bc = 0, hasRec = 0;
 
         for (int p = 0; p < grammar[i].prodCount; ++p) {
             const char *prod = grammar[i].productions[p];
             if (prod[0] == A) { hasRec = 1; strcpy(alpha[ac++], prod + 1); }
             else              { strcpy(beta[bc++],  prod); }
         }
         if (!hasRec) continue;
 
         char A_ = 'Z' - i;                      /* again: quick unique letter */
         int idxA_ = grammarCount++;
         grammar[idxA_].head = A_;
         grammar[idxA_].prodCount = 0;
 
         grammar[i].prodCount = 0;
         for (int b = 0; b < bc; ++b) {
             sprintf(grammar[i].productions[grammar[i].prodCount++], "%s%c", beta[b], A_);
         }
         if (bc == 0) {
             grammar[i].productions[grammar[i].prodCount][0] = EPSILON;
             grammar[i].productions[grammar[i].prodCount][1] = A_;
             grammar[i].productions[grammar[i].prodCount][2] = '\0';
             ++grammar[i].prodCount;
         }
 
         for (int a = 0; a < ac; ++a) {
             sprintf(grammar[idxA_].productions[grammar[idxA_].prodCount++],
                     "%s%c", alpha[a], A_);
         }
         grammar[idxA_].productions[grammar[idxA_].prodCount][0] = EPSILON;
         grammar[idxA_].productions[grammar[idxA_].prodCount][1] = '\0';
         ++grammar[idxA_].prodCount;
     }
 }
 

 void initSetsTable(void)
 {
     for (int i = 0; i < grammarCount; ++i) {
         setsTable[i].nonTerm = grammar[i].head;
         setsTable[i].firstSet[0]  = '\0';
         setsTable[i].followSet[0] = '\0';
     }
 }
 
 static void computeFirst(char X, char *res);
 
 void computeFirstSets(void)
 {
     int changed;
     do {
         changed = 0;
         for (int i = 0; i < grammarCount; ++i) {
             char before[MAX_SYMBOLS];
             strcpy(before, setsTable[i].firstSet);
             computeFirst(grammar[i].head, setsTable[i].firstSet);
             if (strcmp(before, setsTable[i].firstSet)) changed = 1;
         }
     } while (changed);
 }
 
 static void computeFirst(char X, char *res)
 {
     if (isTerminal(X) || X == EPSILON) { addToSet(res, X); return; }
 
     int idx = findNonTermIndex(X);
     if (idx == -1) return;
 
     for (int p = 0; p < grammar[idx].prodCount; ++p) {
         const char *prod = grammar[idx].productions[p];
         if (prod[0] == EPSILON && prod[1] == '\0') { addToSet(res, EPSILON); continue; }
 
         int k = 0;
         while (prod[k]) {
             char sub[MAX_SYMBOLS] = "";
             computeFirst(prod[k], sub);
 
             for (int s = 0; sub[s]; ++s)
                 if (sub[s] != EPSILON) addToSet(res, sub[s]);
 
             if (!containsSymbol(sub, EPSILON)) break;
             ++k;
         }
         if (!prod[k]) addToSet(res, EPSILON);
     }
 }
 
 void computeFollowSets(void)
 {
     addToSet(setsTable[0].followSet, '$');
 
     int changed;
     do {
         changed = 0;
         for (int i = 0; i < grammarCount; ++i) {
             char A = grammar[i].head;
             char *followA = setsTable[i].followSet;
 
             for (int p = 0; p < grammar[i].prodCount; ++p) {
                 const char *prod = grammar[i].productions[p];
                 int len = strlen(prod);
 
                 for (int pos = 0; pos < len; ++pos) {
                     char B = prod[pos];
                     if (!isNonTerminal(B)) continue;
 
                     char *followB = setsTable[findNonTermIndex(B)].followSet;
 
                     /* FIRST(β) */
                     int q = pos + 1;
                     char firstBeta[MAX_SYMBOLS] = "";
                     int allEps = 1;
                     while (q < len) {
                         char tmp[MAX_SYMBOLS] = "";
                         computeFirst(prod[q], tmp);
                         for (int s = 0; tmp[s]; ++s)
                             if (tmp[s] != EPSILON)
                                 if (!containsSymbol(followB, tmp[s])) {
                                     addToSet(followB, tmp[s]); changed = 1;
                                 }
                         if (!containsSymbol(tmp, EPSILON)) { allEps = 0; break; }
                         ++q;
                     }
                     if (q == len && allEps) {
                         for (int s = 0; followA[s]; ++s)
                             if (!containsSymbol(followB, followA[s])) {
                                 addToSet(followB, followA[s]); changed = 1;
                             }
                     }
                 }
             }
         }
     } while (changed);
 }
 

 static void gatherTerminals(void)
 {
     terminalCount = 0; terminals[0] = '\0';
 
     for (int i = 0; i < grammarCount; ++i)
         for (int p = 0; p < grammar[i].prodCount; ++p)
             for (int k = 0; grammar[i].productions[p][k]; ++k) {
                 char c = grammar[i].productions[p][k];
                 if (isTerminal(c) && !containsSymbol(terminals, c)) {
                     int len = strlen(terminals);
                     terminals[len] = c; terminals[len + 1] = '\0'; ++terminalCount;
                 }
             }
 
     addToSet(terminals, '$');   /* ensure $ */
     terminalCount = strlen(terminals);
 }
 
 static int termIndex(char c)
 {
     char *p = strchr(terminals, c);
     return p ? (int)(p - terminals) : -1;
 }
 
 void buildLL1Table(void)
 {
     gatherTerminals();
 
     for (int r = 0; r < MAX_NONTERMS; ++r)
         for (int c = 0; c < MAX_SYMBOLS; ++c)
             LL1Table[r][c][0] = '\0';
 
     for (int i = 0; i < grammarCount; ++i) {
         char A = grammar[i].head;
         for (int p = 0; p < grammar[i].prodCount; ++p) {
             const char *prod = grammar[i].productions[p];
 
             char firstP[MAX_SYMBOLS] = "";
             if (prod[0] == EPSILON && !prod[1]) {
                 addToSet(firstP, EPSILON);
             } else {
                 int k = 0, allEps = 1;
                 while (prod[k]) {
                     char sub[MAX_SYMBOLS] = "";
                     computeFirst(prod[k], sub);
                     for (int s = 0; sub[s]; ++s)
                         if (sub[s] != EPSILON) addToSet(firstP, sub[s]);
                     if (!containsSymbol(sub, EPSILON)) { allEps = 0; break; }
                     ++k;
                 }
                 if (allEps) addToSet(firstP, EPSILON);
             }
 
             char entry[MAX_SYMBOLS];
             sprintf(entry, "%c->%s", A, prod);
 
             for (int s = 0; firstP[s]; ++s)
                 if (firstP[s] != EPSILON) {
                     int col = termIndex(firstP[s]);
                     strcpy(LL1Table[i][col], entry);
                 }
 
             if (containsSymbol(firstP, EPSILON)) {
                 const char *followA = setsTable[findNonTermIndex(A)].followSet;
                 for (int s = 0; followA[s]; ++s) {
                     int col = termIndex(followA[s]);
                     strcpy(LL1Table[i][col], entry);
                 }
             }
         }
     }
 }
 

 void displaySets(void)
 {
     printf("\nFIRST & FOLLOW Sets:\n");
     for (int i = 0; i < grammarCount; ++i) {
         printf("  %c:\n    FIRST  = { ", setsTable[i].nonTerm);
         for (int j = 0; setsTable[i].firstSet[j]; ++j)
             printf("%c ", setsTable[i].firstSet[j]);
         printf("}\n    FOLLOW = { ");
         for (int j = 0; setsTable[i].followSet[j]; ++j)
             printf("%c ", setsTable[i].followSet[j]);
         printf("}\n\n");
     }
 }
 
 void displayLL1Table(void)
 {
     printf("\nLL(1) Parsing Table:\n        ");
     for (int c = 0; c < terminalCount; ++c)
         printf("  %c   ", terminals[c]);
     printf("\n");
 
     for (int r = 0; r < grammarCount; ++r) {
         printf("  %c  | ", grammar[r].head);
         for (int c = 0; c < terminalCount; ++c) {
             if (LL1Table[r][c][0])
                 printf("%-6s", LL1Table[r][c]);
             else
                 printf("  -   ");
         }
         printf("\n");
     }
 }
 

 static void printStack(char *buf, const char st[], int top)
 {
     int pos = 0;
     for (int i = 0; i <= top; ++i) pos += sprintf(buf + pos, "%c", st[i]);
     buf[pos] = '\0';
 }
 static void printRemaining(char *buf, char tok[][MAX_SYMBOLS], int ip, int n)
 {
     int pos = 0;
     for (int i = ip; i < n; ++i) {
         pos += sprintf(buf + pos, "%s", tok[i]);
         if (i < n - 1) pos += sprintf(buf + pos, " ");
     }
     buf[pos] = '\0';
 }
 static int tokenize(const char *line, char tok[][MAX_SYMBOLS])
 {
     char copy[256]; strcpy(copy, line);
     int n = 0;
     for (char *t = strtok(copy, " \t\r\n"); t; t = strtok(NULL, " \t\r\n"))
         strcpy(tok[n++], t);
     strcpy(tok[n++], "$");
     return n;
 }
 
 static int parseLine(const char *line, int lineNo)
 {
     char tok[128][MAX_SYMBOLS];
     int nTok = tokenize(line, tok);
 
     char stack[MAX_STACK]; int top = -1;
     stack[++top] = '$'; stack[++top] = grammar[0].head;
 
     int ip = 0, step = 1, errors = 0;
     char sBuf[256], iBuf[256];
 
     printf("\nLine %d: %s\n", lineNo, line);
     printf("%-4s %-20s %-25s %-20s\n", "Step", "Stack", "Input", "Action");
 
     while (top >= 0) {
         printStack(sBuf, stack, top);
         printRemaining(iBuf, tok, ip, nTok);
 
         char X = stack[top];
         char *curToken = tok[ip];
 
         if (X == '$' && curToken[0] == '$' && !curToken[1]) {
             printf("%-4d %-20s %-25s ACCEPT\n", step++, sBuf, iBuf);
             return errors;
         }
 
         if (!isNonTerminal(X) || X == '$') {            /* terminal */
             if (X == curToken[0] && (!curToken[1] || curToken[0] == '$')) {
                 printf("%-4d %-20s %-25s match '%c'\n", step++, sBuf, iBuf, X);
                 --top; ++ip;
             } else {
                 printf("%-4d %-20s %-25s ERROR: expected '%c', discarding '%s'\n",
                        step++, sBuf, iBuf, X, curToken);
                 ++ip; ++errors;
             }
             continue;
         }
 
         int row = findNonTermIndex(X);
         int col = termIndex(curToken[0]);
         if (row >= 0 && col >= 0 && LL1Table[row][col][0]) {
             char rhs[MAX_SYMBOLS];
             strcpy(rhs, strchr(LL1Table[row][col], '>') + 1);
 
             printf("%-4d %-20s %-25s use %s\n",
                    step++, sBuf, iBuf, LL1Table[row][col]);
             --top;
             if (!(rhs[0] == EPSILON && rhs[1] == '\0'))      /* not ε */
                 for (int k = (int)strlen(rhs) - 1; k >= 0; --k)
                     stack[++top] = rhs[k];
         } else {
             printf("%-4d %-20s %-25s ERROR: no rule for %c with '%s'; skipping '%s'\n",
                    step++, sBuf, iBuf, X, curToken, curToken);
             ++ip; ++errors;
         }
 
         if (ip >= nTok) {            /* safeguard */
             printf("%-4d %-20s %-25s ERROR: unexpected end of input\n",
                    step++, sBuf, iBuf);
            ++errors; break;
         }
     }
     return errors;
 }
 
 static void parseInputFile(const char *fname)
 {
     FILE *fp = fopen(fname, "r");
     if (!fp) { perror(fname); return; }
 
     char line[256];
     int lineNo = 1, totalErr = 0;
     while (fgets(line, sizeof line, fp)) {
         if (strspn(line, " \t\r\n") == strlen(line)) { ++lineNo; continue; }
         int e = parseLine(line, lineNo);
         if (!e)  printf("Line %d: Parsed successfully.\n", lineNo);
         else     printf("Line %d: Parsed with %d error%s.\n",
                         lineNo, e, e == 1 ? "" : "s");
         totalErr += e;
         ++lineNo;
     }
     fclose(fp);
     printf("\nParsing completed with %d total error%s.\n",
            totalErr, totalErr == 1 ? "" : "s");
 }
 

 int main(void)
 {
     freopen("output.txt", "w", stdout);
 
     readGrammarFromFile("grammar.txt");
     displayGrammar("Initial Grammar:");
 
     leftFactorGrammar();
     displayGrammar("After Left Factoring:");
 
     removeLeftRecursion();
     displayGrammar("After Removing Left Recursion:");
 
     initSetsTable();
     computeFirstSets();
     computeFollowSets();
     displaySets();
 
     buildLL1Table();
     displayLL1Table();
 
     parseInputFile("input.txt");
     return 0;
 }
 