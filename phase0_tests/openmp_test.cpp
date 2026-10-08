#include <iostream>
#include <omp.h>

int main() {

    std::cout << "========================================\n";
    std::cout << "       CYBERSENTINEL OPENMP TEST\n";
    std::cout << "========================================\n\n";

    int max_threads = omp_get_max_threads();

    std::cout << "Maximum available OpenMP threads: "
              << max_threads << "\n\n";

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();

        #pragma omp critical
        {
            std::cout << "Thread "
                      << thread_id
                      << " / "
                      << total_threads
                      << " is running\n";
        }
    }

    std::cout << "\nOpenMP working correctly.\n";

    return 0;
}