#
CC=gcc
COPTS= -O3 -Wall


cache:	cache.o main_cache.o
	$(CC) $(COPTS) -o cache cache.o  main_cache.o


cache.o :	cache.c cache.h 
	$(CC) -c $(COPTS)  -I. cache.c

main_cache.o :	main_cache.c cache.h
	$(CC) -c $(COPTS)  -I. main_cache.c  


clean :
	-rm cache cache.o main_cache.o 
