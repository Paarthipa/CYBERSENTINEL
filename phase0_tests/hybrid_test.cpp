#include <mpi.h>
#include <omp.h>
#include <iostream>

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    int rank;
    int world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();

        #pragma omp critical
        {
            std::cout
                << "MPI Rank "
                << rank
                << " / "
                << world_size
                << " | OpenMP Thread "
                << thread_id
                << " / "
                << total_threads
                << "\n";
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "\n========================================\n";
        std::cout << " Hybrid MPI + OpenMP Test Successful\n";
        std::cout << "========================================\n";
    }

    MPI_Finalize();

    return 0;
}