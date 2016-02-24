SIM=simulator
ASM=assembler

#CFLAGS = -m32

.PHONY: all tags
all:
	gcc $(CFLAGS) $(SIM).c -o $(SIM)
	gcc $(CFLAGS) $(ASM).c -o $(ASM)

tags:
	ctags -R .

clean:	
	rm -f $(SIM) $(ASM) tags
