
all: shared_p_test

clean: 
	rm -f shared_p_test.o
	rm -f shared_p_test

shared_p_test.o:
	echo "Make shared_p.o"
	g++ -g -Igoogletest/googletest/include --std=c++11 -c shared_p_test.cpp -o shared_p_test.o

regenerate_gtest_main.a:
	$(MAKE) -C googletest/googletest/make all

shared_p_test: shared_p_test.o regenerate_gtest_main.a
	echo "Make shared_p_test"
	g++ -isystem -Igoogletest/googletest/include -g -Wall -Wextra -pthread -lpthread googletest/googletest/make/gtest_main.a shared_p_test.o -o shared_p_test
