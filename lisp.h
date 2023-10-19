#define nil ((Value){.type = TNil})

#define vector(t, v, n) (*(t*)access((v), (n), sizeof (t)))
#define list(l, n) vector(Value, (l)->list, (n))
#define string(s, n) vector(char, (s)->string, (n))

#define isobject(v) ((v).type >= TList)
#define isother(v) ((v).type == TOther)
#define islist(v) ((v).type == TList)

#define cfunc(f) ((Value){.type = TFunc, .func = f})

#define nullterm(s) (vector(char, (s), (s)->len) = '\0', (s)->len--)

enum Type {
	TNil    = 0,
	TFunc   = 1 << 0,
	TWeak   = 1 << 1,
	TNumber = 1 << 2,
	TList   = 1 << 3,
	TSymbol = 1 << 4,
	TString = 1 << 5,
	TOther  = 1 << 6,
};

typedef struct Vector {
	void *d;
	int cap, len;
} Vector;

typedef struct Other {
	void *d;
	void (*delete)(void *);
} Other;

typedef struct Object {
	union {
		Other o;
		Vector v;
	};
	struct Object *next;
	int mark;
} Object;

typedef struct Value {
	enum Type type;
	union {
		double number;
		struct Value (*func)(struct Value *ctx, struct Value *);
		struct Value *weak;
		Other  *other;
		Vector *symbol;
		Vector *string;
		Vector *list;   /* list type, but also generic for the previous two */
		Object *object; /* generic type for the previous four */
	};
} Value;

/* main.c */
Value eval(Value *ctx, Value *v);
Value eval_weak(Value *ctx, Value *v);

/* vector.c */
void *access(Vector *v, int n, int sz);

/* mem.c */
Value make(enum Type type);
Value pack(void *d, void (*delete)(void*));
void set(Value *d, Value s);
void collect(Value *v);

/* map.c */
Value eval_map_literal(Value *ctx, Value *args);
Value eval_map_get(Value *ctx, Value *args);
Value eval_list_literal(Value *ctx, Value *args);
Value eval_list_get(Value *ctx, Value *args);
Value *mapget(Value *map, Value *key);
int cmp(Value *a, Value *b);
Value *getvar(Value *map, char *key);
void setvar(Value *map, char *key, Value v);

/* debug.c */
Value eval_write(Value *ctx, Value *args);
Value eval_debug(Value *ctx, Value *args);
void dump(Value*, int);

/* expr.c */
Value eval_while(Value *ctx, Value *args);
Value eval_do(Value *ctx, Value *args);
Value eval_if(Value *ctx, Value *args);
Value eval_len(Value *ctx, Value *args);
Value eval_add(Value *ctx, Value *args);
Value eval_sub(Value *ctx, Value *args);
Value eval_mul(Value *ctx, Value *args);
Value eval_div(Value *ctx, Value *args);
Value eval_mod(Value *ctx, Value *args);
Value eval_gt(Value *ctx, Value *args);
Value eval_lt(Value *ctx, Value *args);
Value eval_ge(Value *ctx, Value *args);
Value eval_le(Value *ctx, Value *args);
Value eval_eq(Value *ctx, Value *args);
Value eval_ne(Value *ctx, Value *args);
Value eval_or(Value *ctx, Value *args);
Value eval_and(Value *ctx, Value *args);
Value eval_not(Value *ctx, Value *args);
