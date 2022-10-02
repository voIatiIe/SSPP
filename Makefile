build-task1-cpp:
	g++ -Wall -Werror -pthread Task_1/task1.cpp -o Task_1/bin/out
build-task1-c:
	gcc -Wall -Werror -pthread Task_1/task1.c -o Task_1/bin/out
build-task2:
	gcc -Wall -Werror Task_2/task2.c -o Task_2/bin/out
build-task2-gen-mrx:
	gcc -Wall -Werror Task_2/matrix_generator.c -o Task_2/bin/mrx_out