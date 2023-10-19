#include <stdlib.h>
#include <stdio.h>
#include "lisp.h"

int _objects = 0;

static int turn = 0;

static Object *objects = 0;

static Object *alloc(void)
{
	++_objects;
	Object *o = calloc(1, sizeof(Object));
	o->mark = turn;
	o->next = objects;
	objects = o;
	return o;
}

static void delete(Object *o)
{
	free(o->v.d);
	free(o);
	--_objects;
}

static void mark(Value *v)
{
	int i;

	if (isobject(*v)) {
		if (v->object->mark == turn)
			return;
		v->object->mark = turn;
	}
	if (islist(*v)) {
		for (i = 0; i < v->list->len; i++)
			mark(&vector(Value, v->list, i));
	}
}

static void sweep() {
	Object *o;
	Object *next, *prev = 0;

	for (o = objects; o; o = next) {
		next = o->next;
		if (o->mark == turn) {
			prev = o;
			continue;
		}
		if (prev)
			prev->next = next;
		else
			objects = next;
		delete(o);
	}
}

Value pack(void *d, void (*delete)(void*))
{
	Value v = make(TOther);

	v.other->d = d;
	v.other->delete = delete;
	return v;
}

Value make(enum Type type)
{
	Value v = nil;

	v.type = type;
	if (isobject(v))
		v.object = alloc();
	return v;
}

void collect(Value *v) {
	turn = !turn;
	mark(v);
	sweep();
}

void set(Value *d, Value s)
{
	*d = s;
}
