all: ./build/main

run: ./build/main
	./build/main

./build/main: src/main.c | ./build/
	cc -o $@ $^

./build/:
	mkdir -p ./build/
