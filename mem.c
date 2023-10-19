#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lisp.h"

int _objects = 0;
int _max_objects = 1;

static Object *objects = 0;

static void delete(Object *o)
{
	free(o->v.d);
	free(o);
	--_objects;
}

static int mark(Object *o)
{
	Value *v;
	int i, c = 0;

	if (o->mark)
		return c;
	c++;
	o->mark = 1;
	if (o->list) {
		for (i = 0; i < o->v.len; i++) {
			v = &vector(Value, &o->v, i);
			if (isobject(*v))
				c += mark(v->object);
		}
	}
	return c;
}

static void sweep()
{
	Object *o;
	Object *next, *prev = 0;

	for (o = objects; o; o = next) {
		next = o->next;
		if (o->mark) {
			o->mark = 0;
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

static void collect()
{
	int c = 0;
	Object *o;

	for (o = objects; o; o = o->next) {
		if (o->root)
			c += mark(o);
	}
	sweep();
	assert(c == _objects);
}

static Object *alloc(void)
{
	++_objects;
	Object *o = calloc(1, sizeof(Object));
	o->root = 1;
	o->next = objects;
	objects = o;
	return o;
}

Value make(enum Type type)
{
	Value v = nil;

	v.type = type;
	if (isobject(v))
		v.object = alloc();
	if (islist(v))
		v.object->list = 1;

	if (_objects > _max_objects) {
		collect();
		if (_objects > _max_objects)
			_max_objects *= 2;
	}
	return v;
}

void track(Value *v)
{
	if (isobject(*v))
		v->object->root = 0;
}

void set(Value *d, Value s)
{
	track(d);
	*d = s;
}
