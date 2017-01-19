SMACHINE=smachine
SMAS=assembler
SMCC=ccompiler

HOS = $(shell uname -m)

ifeq ("x86_64", $(HOST))
CFLAGS = -m32 -g
else
CFLAGS = -g
endif

.PHONY: all tags test

all: $(SMACHINE) $(SMAS) $(SMCC)

$(SMACHINE):$(SMACHINE).c
	gcc $(CFLAGS) $(SMACHINE).c -o $(SMACHINE)

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
	./$(SMACHINE) test/ctest1.bin || true

ctest2:
	./$(SMCC) test/ctest2.c > test/ctest2.s
	 #./$(SMAS) test/ctest2.s test/ctest2.bin

tags:
	ctags -R .

clean:	
	rm -f $(SMACHINE) $(SMAS) $(SMCC) tags test/*.bin
