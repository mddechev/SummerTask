#include <cstddef>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include "../v2/DataSource.hpp"

char* generateRandomString() {
    const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    static char randomString[11];

    for (size_t i = 0; i < 10; i++) {
        randomString[i] = alphabet[rand() % 26];
    }
    randomString[10] = '\0';
    return randomString; 
}

void demonstrateStringSource() {
    srand(time(nullptr));
    GeneratorDataSource<char*> stringSource(generateRandomString);

    std::cout << "25 random strings of 10 lowercase letters:\n";
    // char** batch = stringSource.extractBulk(25);

    for (int i = 0; i < 25; ++i) {
        // char* str = stringSource.extract();
        std::cout << "Random string" << i + 1 << ": " << stringSource.extract() << std::endl;
        // Не освобождаваме паметта тук, тъй като GeneratorDataSource използва същия буфер
    }
    

    // for (size_t i = 0; i < 25; i++) {
    //     delete batch[i];
    // }
    // delete [] batch;
    // // Освобождаваме паметта след приключване на използването
    // char* lastString = stringSource.extract();
    // delete[] lastString;
}

int main() {
    demonstrateStringSource();
    return 0;
}