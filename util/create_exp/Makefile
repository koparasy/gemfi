all: final_create_exp.c mtwist.c randistrs.c
	gcc -I. -Wall -g mtwist.c -o mtwist.o -c -lm
	gcc -I. -Wall -g randistrs.c -o randistrs.o -c -lm
	gcc -I. -Wall -g final_create_exp.c -o final_create_exp.o -c -lm
	gcc final_create_exp.o randistrs.o  mtwist.o -o generator -lm
clean:
	rm -f mtwist.o final_create_exp.o generator

new: new_create_exp.c mtwist.c randistrs.c
	gcc -I. -Wall -g mtwist.c -c  -lm
	gcc -I. -Wall -g randistrs.c  -c -lm
	gcc -I. -Wall -g new_create_exp.c  -c -lm
	gcc new_create_exp.o randistrs.o  mtwist.o -o new_generator -lm

