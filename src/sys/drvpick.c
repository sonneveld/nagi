/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "drvpick.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */



// driver search for later
// list of strings and their init function
// find the one we need, load that one
// if fail.. set to start and go through all of them, ignoring the one we just tried.



// return 0 on good drv
// return -1 on fail
int drvpick_first(DRVINITSTATE *dstate, DRVINIT *init_list, int size, u8 *def, void *ptr_list)
{
	int i = 0;

	assert(init_list);
	assert(def);
	assert(ptr_list);
	assert(size > 0);
	
	dstate->init_list = init_list;
	dstate->size = size;
	dstate->ptr_list = ptr_list;
	
	while ((i<size) && (!strcasecmp(init_list[i].name, def)) )
		i++;
	
	if (i != size)	// we've found the default
	{
		dstate->cur = &init_list[i];
		dstate->def = dstate->cur;
	}
	else	// use the first one
	{
		dstate->cur = init_list;
		dstate->def = 0;
	}
	
	// fill ptr list
	(dstate->cur)->expose_ptrs(ptr_list);
	
	return 0;
}

int drvpick_next(DRVINITSTATE *dstate)
{
	assert(dstate);
	assert(dstate->init_list);
	assert(dstate->size > 0);
	assert(dstate->ptr_list);
	assert(dstate->cur);
	assert(dstate->def);
	
	// if default the first time.. then go to start of list
	if (dstate->cur == dstate->def)
		dstate->cur = dstate->init_list;
	else
		dstate->cur ++;
	
	// if default after going through list.. skip it.
	if (dstate->cur == dstate->def)
		dstate->cur ++;
	
	// check limits
	if (dstate->cur >= (dstate->init_list + dstate->size) )
		return -1;
	
	// fill ptr list
	(dstate->cur)->expose_ptrs(dstate->ptr_list);
	
	return 0;
}
