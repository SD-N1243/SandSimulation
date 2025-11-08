# Compiler and linker
Compiler = gcc
FLAGS = -lglfw3 -lfreeglut -lopengl32 -lglu32

# Files
Target = Main.c
Output = Main.exe

all:
	${Compiler} ${Target} -o ${Output} ${FLAGS}
	if [ -f "$(Output)" ]; then ./$(Output); else echo "$(Output) not found."; fi

compile:
	${Compiler} ${Target} -o ${Output} ${FLAGS}

run:
	if [ -f "$(Output)" ]; then ./$(Output); else echo "$(Output) not found."; fi