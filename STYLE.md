# NAGI Style Guide

The original code was written many years ago when I was still learning
how to write in C. I would probably choose a different style for a new project.


## Indentation

Tabs, 8 spaces wide.


## Braces

Brace is always on its own line. Braces optional for single statement blocks.

e.g.

	int example_function(int arg, int arg, int arg))
	{
		if (condition)
			return arg + arg;
		else
		{
			arg -= arg;
			return arg * arg;
		}
	}

NOTE: Prefer to always use braces for new code.
