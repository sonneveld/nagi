
// 1xy, initxy, 2xy 3y
struct data_struct
{
	u8 a,b,c,d,e,f,g;
	struct data_struct *next;
};

typedef struct data_struct DATA;


DATA *fill_push(DATA *data, u8 a, u8 b, u8 c, u8 d, u8 e, u8 f, u8 g);
DATA *fill_pop(DATA *data, u8 *a, u8 *b, u8 *c, u8 *d, u8 *e, u8 *f, u8 *g);
void fill_topab(DATA *data, u8 *a, u8 *b);