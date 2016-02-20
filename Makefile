SIM=simulator
ASM=assembler

#CFLAGS = -m32

all: clean
	gcc $(CFLAGS) $(SIM).c -o $(SIM)
	gcc $(CFLAGS) $(ASM).c -o $(ASM)

clean:	
	rm -f $(SIM) $(ASM)
