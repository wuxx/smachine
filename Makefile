OBJ=simulator

#CFLAGS = -m32

all: clean
	gcc $(CFLAGS) $(OBJ).c -o $(OBJ)

clean:	
	rm -f $(OBJ)
