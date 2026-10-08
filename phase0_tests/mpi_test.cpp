#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    int rank;
    int world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (rank == 0) {
        std::cout << "========================================\n";
        std::cout << "       CYBERSENTINEL MPI TEST\n";
        std::cout << "========================================\n\n";

        std::cout << "MPI processes: "
                  << world_size
                  << "\n\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    std::cout
        << "Rank "
        << rank
        << " / "
        << world_size
        << " is running\n";

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "\nMPI environment working correctly.\n";
    }

    MPI_Finalize();

    return 0;
}
