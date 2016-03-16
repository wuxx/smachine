SMSIM=simulator
SMAS=assembler
SMCC=ccompiler

#CFLAGS = -m32 -g
CFLAGS = -g

.PHONY: all tags test

all: $(SMSIM) $(SMAS) $(SMCC)
	gcc $(CFLAGS) $(SMSIM).c -o $(SMSIM)
	gcc $(CFLAGS) $(SMAS).c  -o $(SMAS)
	gcc $(CFLAGS) $(SMCC).c  -o $(SMCC)

test:
	./$(SMAS) test.s bar.bin

tags:
	ctags -R .

clean:	
	rm -f $(SMSIM) $(SMAS) $(SMCC) tags bar.bin
