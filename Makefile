#-------------------------------------------------------#
task1:
	gcc -pthread Task_1/task1.c -o Task_1/bin/main
#-------------------------------------------------------#
task2:
	gcc Task_2/task2.c -o Task_2/bin/main
task2-gen:
	gcc Task_2/matrix_generator.c -o Task_2/bin/gen
#-------------------------------------------------------#
task3:
	gcc -lpapi Task_3/task3.c -o Task_3/bin/main
task3-gen:
	gcc Task_3/matrix_generator.c -o Task_3/bin/gen
#-------------------------------------------------------#
task4:
	gcc -pthread Task_4/task4.c -o Task_4/bin/main
task4-qsort:
	gcc Task_4/qsort.c -o Task_4/bin/qsort
task4-gen:
	gcc Task_4/array_generator.c -o Task_4/bin/gen
#-------------------------------------------------------#
task5:
	mpicc Task_5/task5.c -o Task_5/bin/main
task5-gen:
	gcc Task_5/matrix_vector_generator.c -o Task_5/bin/gen
#-------------------------------------------------------#
task6:
	mpicc Task_6/task6.c -o Task_6/bin/main
task6-gen:
	gcc Task_6/matrix_generator.c -o Task_6/bin/gen

# compile papi on polus
# gcc -std=c99 -lpapi test.c -o out

# compile mpi on polus
# module load SpectrumMPI
