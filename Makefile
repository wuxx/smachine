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

ctest1:
	./$(SMCC) test/ctest1.c > test/ctest1.s
	./$(SMAS) test/ctest1.s test/ctest1.bin

ctest2:
	./$(SMCC) test/ctest2.c > test/ctest2.s
	 #./$(SMAS) test/ctest2.s test/ctest2.bin

tags:
	ctags -R .

clean:	
	rm -f $(SMSIM) $(SMAS) $(SMCC) tags test/*.bin
