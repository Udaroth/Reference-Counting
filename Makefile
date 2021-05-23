
output: rc.o rc_test.o
	clang rc_test.o rc.o -o rc -fsanitize=address

rc.o: rc.c rc.h
	clang -c rc.c

rc_test.o: rc_test.c rc.h
	clang -c rc_test.c

clean:
	rm *.o output