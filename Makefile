SMSIM=simulator
SMAS=assembler
SMCC=ccompiler

#CFLAGS = -m32 -g
CFLAGS = -g

.PHONY: all tags test

all: $(SMSIM) $(SMAS) $(SMCC)

$(SMSIM):$(SMSIM).c
	gcc $(CFLAGS) $(SMSIM).c -o $(SMSIM)

$(SMAS):$(SMAS).c
	gcc $(CFLAGS) $(SMAS).c  -o $(SMAS)

$(SMCC):$(SMCC).c
	gcc $(CFLAGS) $(SMCC).c  -o $(SMCC)

stest:
	./$(SMAS) test/test.s   test/test.bin
	./$(SMAS) test/sum.s    test/sum.bin
	./$(SMAS) test/prime.s  test/prime.bin

ctest:
	./$(SMCC) test/ctest1.c > test/ctest1.s
	./$(SMAS) test/ctest1.s test/ctest1.bin

tags:
	ctags -R .

clean:	
	rm -f $(SMSIM) $(SMAS) $(SMCC) tags test/*.bin
