#ifndef NAGI_LOGIC_CMD_TABLE_H
#define NAGI_LOGIC_CMD_TABLE_H

struct func_struct
{
	const char *func_name;
	void *func;
	int param_total;		// 2
	unsigned int param_flag;		// 3
};
typedef struct func_struct FUNC;

typedef u8 *(*CMD_TYPE) (u8 *);
typedef u8 (*EVAL_TYPE) (void);

/*
struct func_struct
{
	u8 *func_name;
	union
	{
		void *ptr;
		u8 *(*cmd) (u8 *);
		u8 (*eval) (void);
	} func;
	u8 param_total;		// 2
	u8 param_flag;		// 3
};
typedef struct func_struct FUNC;
*/

extern FUNC eval_table[];
extern FUNC cmd_table[CMD_MAX + 1];

extern u8 *cmd_do_nothing(u8 *code);
extern u8 cmd_ret_false(void);

#endif /* NAGI_LOGIC_CMD_TABLE_H */
