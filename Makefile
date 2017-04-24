all: criadorFifo scheduler interpretador prog1 prog2 prog3 prog4 prog5 prog6 prog7 prog8 prog9 stols
clean: 
	@rm -f criadorFifo scheduler interpretador prog1 prog2 prog3 prog4 prog5 prog6 prog7 prog8 prog9 stols

criadorFifo: criadorFifo.c
	gcc -Wall -o criadorFifo criadorFifo.c
scheduler: scheduler.c Lista.c Fila.c
	gcc -Wall -o scheduler scheduler.c Lista.c Fila.c
interpretador: interpretador.c
	gcc -Wall -o interpretador interpretador.c
prog1: prog1.c
	gcc -Wall -o prog1 prog1.c
prog2: prog2.c
	gcc -Wall -o prog2 prog2.c
prog3: prog3.c
	gcc -Wall -o prog3 prog3.c
prog4: prog4.c
	gcc -Wall -o prog4 prog4.c
prog5: prog5.c
	gcc -Wall -o prog5 prog5.c
prog6: prog6.c
	gcc -Wall -o prog6 prog6.c
prog7: prog7.c
	gcc -Wall -o prog7 prog7.c
prog8: prog8.c
	gcc -Wall -o prog8 prog8.c
prog9: prog9.c
	gcc -Wall -o prog9 prog9.c 
stols: stols.c
	gcc -Wall -o stols stols.c