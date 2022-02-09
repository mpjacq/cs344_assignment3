setup:
	gcc -std=gnu99 -g -Wall -o smallsh main.c

debug:
	valgrind -s --leak-check=yes --track-origins=yes --show-reachable=yes ./smallsh