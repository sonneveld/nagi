#!/bin/sh
gcc -M -MG $* | sed -e 's@ /[^ ]*@@g' -e 's@^\(.*\)\.o:@\1.d \1.o:@'
 
