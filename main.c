#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

Value run_lambda(Value *ctx, Value *lbd, Value *args);

Value global = nil;

int parse(Value *v, FILE *f)
{
	double n;
	int ch, i;

	for (;;) {
		while (isspace(ch = fgetc(f)))
			;
		if (ch == ';') {
			while ((ch = fgetc(f)) != '\n')
				;
		} else if (ch < 0)
		 	return ch;
		else break;
	}
	if (ch == '\"') {
		set(v, make(TString));
		for (i = 0; (ch = fgetc(f)) != '\"' && ch > 0; i++)
			string(v, i) = ch;
	} else if (ch == '(') {
		Value elem = nil;
		set(v, make(TList));
		for (i = 0; parse(&elem, f) >= 0 && elem.type != TNil; i++)
			set(&list(v, i), elem);
		delete(&elem);
	} else if (ch == ')') {
		set(v, make(TNil));
	} else {
		ungetc(ch, f);
		set(v, make(TSymbol));
		for (i = 0; !isspace(ch = fgetc(f)) && !strchr("();", ch); i++)
			string(v, i) = ch;
		ungetc(ch, f);
		nullterm(v->symbol);
		if (sscanf(v->symbol->d, "%lf", &n) > 0) {
			set(v, make(TNumber));
			v->number = n;
		}
	}
	return v->type;
}

Value eval_read(Value *ctx, Value *args)
{
	Value v = nil;

	parse(&v, stdin);
	unmark(&v);
	return v;
}

Value eval_import(Value *ctx, Value *args)
{
	FILE *f;
	Value s = nil, e = nil, r = nil;

	set(&s, eval(ctx, &list(args, 1)));
	if (s.type != TString) {
		delete(&s);
		return r;
	}

	nullterm(s.string);
	if (!(f = fopen(s.string->d, "r"))) {
		fprintf(stderr, "error: could not open: %s\n", s.string->d);
		exit(1);
	}
	delete(&s);

	while (parse(&e, f) >= 0)
		set(&r, eval(ctx, &e));
	delete(&e);
	unmark(&r);
	return r;
}

Value eval_print(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	for (i = 1; i < args->list->len; i++) {
		set(&v, eval(ctx, &list(args, i)));
		switch (v.type) {
		case TNumber:
			printf("%lg", v.number);
			break;
		case TString: case TSymbol:
			fwrite(v.string->d, sizeof(char), v.string->len, stdout);
			break;
		case TNil:
			printf("nil");
			break;
		default:
			printf("(type %d)", v.type);
			break;
		}
		putchar(' ');
	}
	printf("\n");
	unmark(&v);
	return v;
}

Value eval_set(Value *ctx, Value *args)
{
	Value d = nil;

	set(&d, eval_weak(ctx, &list(args, 1)));
	if (d.type == TWeak)
		set(d.weak, eval(ctx, &list(args, 2)));
	delete(&d);
	return nil;
}

Value eval_def(Value *ctx, Value *args)
{
	set(mapget(ctx, &list(args, 1)), eval(ctx, &list(args, 2)));
	return nil;
}

Value eval_lambda(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	set(&v, make(TList));
	for (i = 1; i < args->list->len; i++)
		set(&list(&v, i-1), list(args, i));
	unmark(&v);
	return v;
}

Value eval_quote(Value *ctx, Value *args)
{
	Value v = nil;

	set(&v, list(args, 1));
	unmark(&v);
	return v;
}

Value eval_eval(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	if (args->list->len > 1)
		set(&v, eval(ctx, &list(args, 1)));
	set(&v, eval(ctx, &v));
	unmark(&v);
	return v;
}

Value eval_weak(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	if (args->type == TSymbol) {
		set(&v, make(TWeak));
		v.weak = mapget(ctx, args);
		if (v.weak->type == TNil)
			v.weak = mapget(&global, args);
		unmark(&v);
		return v;
	}
	if (args->type != TList)
		return *args;
	set(&v, eval(ctx, &list(args, 0)));
	if (v.type == TFunc)
		set(&v, v.func(ctx, args));
	else if (v.type == TList)
		set(&v, run_lambda(ctx, &v, args));
	unmark(&v);
	return v;
}

Value eval(Value *ctx, Value *args)
{
	Value v = nil;

	set(&v, eval_weak(ctx, args));
	if (v.type == TWeak)
		set(&v, *v.weak);
	unmark(&v);
	return v;
}

Value run_lambda(Value *ctx, Value *lbd, Value *args)
{
	Value lclctx = make(TList);
	Value *vars = &list(lbd, 0);
	Value v = nil;
	int i;

	for (i = 0; i < vars->list->len; i++)
		set(mapget(&lclctx, &list(vars, i)), eval(ctx, &list(args, i+1)));
	for (i = 1; i < lbd->list->len; i++)
		set(&v, eval(&lclctx, &list(lbd, i)));
	delete(&lclctx);
	unmark(&v);
	return v;
}

void init(Value *ctx)
{
	set(ctx, make(TList));

	/* base system */
	setvar(ctx, "eval", cfunc(eval_eval));
	setvar(ctx, "read", cfunc(eval_read));
	setvar(ctx, "import", cfunc(eval_import));
	setvar(ctx, "print", cfunc(eval_print));
	setvar(ctx, "quote", cfunc(eval_quote));
	setvar(ctx, "set", cfunc(eval_set));
	setvar(ctx, "def", cfunc(eval_def));
	setvar(ctx, "fn", cfunc(eval_lambda));

	/* debugging system */
	setvar(ctx, "debug", cfunc(eval_debug));
	setvar(ctx, "write", cfunc(eval_write));

	/* maps and lists system */
	setvar(ctx, "map", cfunc(eval_map_literal));
	setvar(ctx, "map-get", cfunc(eval_map_get));
	setvar(ctx, "list", cfunc(eval_list_literal));
	setvar(ctx, "list-get", cfunc(eval_list_get));

	/* expression system */
	setvar(ctx, "len", cfunc(eval_len));
	setvar(ctx, "while", cfunc(eval_while));
	setvar(ctx, "do", cfunc(eval_do));
	setvar(ctx, "if", cfunc(eval_if));
	setvar(ctx, "+", cfunc(eval_add));
	setvar(ctx, "-", cfunc(eval_sub));
	setvar(ctx, "*", cfunc(eval_mul));
	setvar(ctx, "/", cfunc(eval_div));
	setvar(ctx, "%", cfunc(eval_mod));
	setvar(ctx, ">", cfunc(eval_gt));
	setvar(ctx, "<", cfunc(eval_lt));
	setvar(ctx, ">=", cfunc(eval_ge));
	setvar(ctx, "<=", cfunc(eval_le));
	setvar(ctx, "=", cfunc(eval_eq));
	setvar(ctx, "!=", cfunc(eval_ne));
	setvar(ctx, "or", cfunc(eval_or));
	setvar(ctx, "and", cfunc(eval_and));
	setvar(ctx, "not", cfunc(eval_not));
}

int main(int argc, char *argv[])
{
	int i;
	FILE *f;
	Value e = nil, r = nil;

	if (argc > 1) {
		if (!(f = fopen(argv[1], "r"))) {
			fprintf(stderr, "error: could not open: %s\n", argv[1]);
			exit(1);
		}
	} else {
		f = stdin;
	}

	init(&global);
	for (;;) {
		if (f == stdin) {
			printf("> ");
			fflush(stdout);
		}
		if (parse(&e, f) < 0)
			break;
		set(&r, eval(&global, &e));
	}
	exit(0);
}
