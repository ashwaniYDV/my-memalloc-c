#include <iostream>
#include "memalloc.h"

int main() {
    // Allocate memory using custom malloc
    int* arr = (int*) malloc(5 * sizeof(int));
    if (arr == nullptr) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }

    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }

    std::cout << "Array elements: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    // Reallocate memory using custom realloc
    arr = (int*) realloc(arr, 10 * sizeof(int));
    if (arr == nullptr) {
        std::cerr << "Memory reallocation failed" << std::endl;
        return 1;
    }

    for (int i = 5; i < 10; ++i) {
        arr[i] = i * 10;
    }

    std::cout << "Array elements after reallocation: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    // Free the allocated memory using custom free
    free(arr);

    // Print the memory list for debugging purposes
    print_mem_list();

    return 0;
}
