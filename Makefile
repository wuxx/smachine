SIM=simulator
AS=assembler
CC=ccompiler

#CFLAGS = -m32

.PHONY: all tags test
all:
	gcc $(CFLAGS) $(SIM).c -o $(SIM)
	gcc $(CFLAGS) $(AS).c -o $(AS)
	gcc $(CFLAGS) $(CC).c -o $(CC)

test:
	./$(AS) test.s bar.bin

tags:
	ctags -R .

clean:	
	rm -f $(SIM) $(AS) $(CC) tags bar.bin
