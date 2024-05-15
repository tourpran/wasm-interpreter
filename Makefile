CC=g++
Files=\
src/main.cc\
include/essentials.cc

wapp: $(Files)
	$(CC) $(Files) -o wapp