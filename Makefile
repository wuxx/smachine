SIM=simulator
ASM=assembler

#CFLAGS = -m32

.PHONY: all tags test
all:
	gcc $(CFLAGS) $(SIM).c -o $(SIM)
	gcc $(CFLAGS) $(ASM).c -o $(ASM)

test:
	./$(ASM) test.s bar.bin

tags:
	ctags -R .

clean:	
	rm -f $(SIM) $(ASM) tags bar.bin
