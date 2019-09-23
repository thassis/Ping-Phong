
all: $(TODOS_ARQUIVOS_OBJ)
	gcc ping.c -lglut -lSOIL -lGL -lGLEW -lGLU -lm && ./a.out

run: all
	(cd .. && exec Makefile/ping)

clean:
	rm *.o ping
