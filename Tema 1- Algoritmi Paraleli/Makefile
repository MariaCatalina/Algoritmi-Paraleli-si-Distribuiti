all: homework homework1

homework: homework.c homework.h main.c
	gcc -fopenmp -g -o homework homework.c homework.h main.c -Wall

homework1: homework1.c homework1.h main1.c
	gcc -fopenmp -g -o homework1 homework1.c homework1.h main1.c -Wall

clean:
	rm homework
	rm homework1
	rm out/*
