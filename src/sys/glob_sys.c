#include "../agi.h"
#include "glob_sys.h"

// 0 = pc
// 1 = pcjr
// 2 = tandy
// 8 = 's' ps/2 model 30 or 25?
s16 computer_type = -1;

// 0 = cga
// 1 = 'r'
// 2 = HGC
// 3 = EGA
// 4 = 'v' mono vga on ps/2 model 30
s16 display_type = -1;

u16 drives_found = 0;


