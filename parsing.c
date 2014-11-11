#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"

/* lval's represent result values */
typedef struct {
    int type;
    long num;
    int err;
} lval;

/* Possible lval types */
enum { LVAL_NUM, LVAL_ERR };

/* Possible Error conditions */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch(v.type) {

    case LVAL_NUM:
        printf("%li", v.num);
        break;

    case LVAL_ERR:
        if (v.err == LERR_DIV_ZERO) {
            printf("Error: Division By Zero!");
        } else if (v.err == LERR_BAD_OP) {
            printf("Error: Invalid Operator!");
        } else if (v.err == LERR_BAD_NUM) {
            printf("Error: Invalid Number!");
        } else {
            printf("Error: Unknown error");
        }
        break;

    }
}

void lval_println(lval v) {
    lval_print(v);
    putchar('\n');
}

int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0) {
    return 1;
  }
  else {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
}

lval eval_op(lval x, char* op, lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return lval_num(x.num + y.num);
    if (strcmp(op, "-") == 0) return lval_num(x.num - y.num);
    if (strcmp(op, "*") == 0) return lval_num(x.num * y.num);
    if (strcmp(op, "/") == 0) {
        if (y.num == 0) {
            return lval_err(LERR_DIV_ZERO);
        } else {
            return lval_num(x.num / y.num);
        }
    }

    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        if (errno == ERANGE) {
            return lval_err(LERR_BAD_NUM);
        } else {
            return lval_num(x);
        }
    }

    // Get the operator from the second position
    char* op = t->children[1]->contents;
    lval x = eval(t->children[2]);

    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int main(int argc, char** argv) {
  /* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);

  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctr+c to Exit\n");

  while (1) {
  	char* input = readline("lispy> ");
  	add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    }
    else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

  	free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}
