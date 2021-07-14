build: src/file.c src/file.h src/int.h src/main.c
	gcc src/*.c -o bin/bin -lncurses
run: ./bin/bin
	./bin/bin