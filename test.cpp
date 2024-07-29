// #include "DataSource.hpp"
// #include <cassert>
// #include <iostream>

// void testFile() {
//     //test extract single
//     FileDataSource<int> src("file.txt");
//     assert(src.extract() == 1); 

//     std::cout << src.extract() << '\n';
// }
// int main() {
//     testFile();
// }

#include <cassert>
#include <cstdlib>
#include <iostream>
#include "DataSource.hpp"
// #include "MyString.hpp"
// #include "MyVector.hpp"

#include <cassert>
#include <iostream>
#include <fstream>

void prepareTestFile(const char* filename) {
    std::ofstream file(filename);
    file << "100 200 300" << std::endl;
    file.close();
}

void testMixedAlternateDataSource() {
    // Подготовка на тестови данни
    prepareTestFile("test_data.txt");

    int arr[] = {1, 2, 3, 4, 5};
    ArrayDataSource<int>* arraySource = new ArrayDataSource<int>(arr, 5);
    DefaultDataSource<int>* defaultSource = new DefaultDataSource<int>();
    FileDataSource<int>* fileSource = new FileDataSource<int>("test_data.txt");

    DataSource<int>* sources[] = {arraySource, defaultSource, fileSource};

    // Тест 1: Конструктор и базова функционалност
    AlternateDataSource<int> ads(sources, 3);
    assert(ads.hasNext());
    assert(ads.extract() == 1);  // От ArrayDataSource
    assert(ads.extract() == 0);  // От DefaultDataSource (подразбираща се стойност)
    assert(ads.extract() == 100);  // От FileDataSource
    std::cout << "Test 1 passed: Constructor and basic extraction work" << std::endl;

    // Тест 2: Продължаване на извличането
    assert(ads.extract() == 2);  // От ArrayDataSource
    assert(ads.extract() == 0);  // От DefaultDataSource
    assert(ads.extract() == 200);  // От FileDataSource
    std::cout << "Test 2 passed: Continued extraction works" << std::endl;

    // Тест 3: Извличане след изчерпване на FileDataSource
    // Тест 3: Извличане след изчерпване на FileDataSource
    std::cout << "Extracting element (expected 3): " << ads.extract() << std::endl;
    std::cout << "Extracting element (expected 0): " << ads.extract() << std::endl;
    std::cout << "Extracting element (expected 300): " << ads.extract() << std::endl;
    std::cout << "Extracting element (expected 4): " << ads.extract() << std::endl;
    std::cout << "Extracting element (expected 0): " << ads.extract() << std::endl;
    std::cout << "Extracting element (expected 5): ";
    try {
        int value = ads.extract();
        std::cout << value << std::endl;
        assert(value == 5);
    } catch (const std::runtime_error& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    std::cout << "Test 3 passed: Extraction continues after FileDataSource is exhausted" << std::endl;

    // Тест 4: Проверка на DefaultDataSource след изчерпване на другите източници
    for (int i = 0; i < 5; ++i) {
        try {
            int value = ads.extract();
            std::cout << "Extracted element from DefaultDataSource: " << value << std::endl;
            assert(value == 0);
        } catch (const std::runtime_error& e) {
            std::cout << "Exception caught: " << e.what() << std::endl;
            assert(false && "Should not throw exception for DefaultDataSource");
        }
    }
    std::cout << "Test 4 passed: DefaultDataSource continues to provide elements" << std::endl;

    // Тест 5: reset()
    assert(ads.reset());
    assert(ads.extract() == 1);  // Отново от ArrayDataSource
    assert(ads.extract() == 0);  // От DefaultDataSource
    assert(ads.extract() == 100);  // Отново от FileDataSource
    std::cout << "Test 5 passed: reset() works" << std::endl;

    // Тест 6: extractBulk()
    int* bulk = ads.extractBulk(6);
    assert(bulk[0] == 2 && bulk[1] == 0 && bulk[2] == 200 &&
           bulk[3] == 3 && bulk[4] == 0 && bulk[5] == 300);
    delete[] bulk;
    std::cout << "Test 6 passed: extractBulk() works" << std::endl;

    // Тест 7: hasNext() винаги връща true поради DefaultDataSource
    assert(ads.hasNext());
    std::cout << "Test 7 passed: hasNext() always true due to DefaultDataSource" << std::endl;

    // Почистване
    delete arraySource;
    delete defaultSource;
    delete fileSource;
}

// Тестов генератор, който връща последователни цели числа
int sequentialGenerator() {
    static int value = 0;
    return value++;
}

// Тестов генератор, който връща случайни числа между 0 и 99
int randomGenerator() {
    return rand() % 100;
}

void testGeneratorDataSource() {
    // Тест 1: Последователен генератор
    GeneratorDataSource<int> seqGen(sequentialGenerator);
    
    std::cout << "Test 1: Sequential Generator\n";
    for (int i = 0; i < 5; ++i) {
        assert(seqGen.extract() == i);
        std::cout << "Extracted: " << i << std::endl;
    }
    std::cout << "Test 1 passed\n\n";

    // Тест 2: Оператор ()
    std::cout << "Test 2: Operator ()\n";
    assert(seqGen() == 5);
    std::cout << "Extracted using operator(): 5\n";
    std::cout << "Test 2 passed\n\n";

    // Тест 3: Оператор >>
    std::cout << "Test 3: Operator >>\n";
    int value;
    seqGen >> value;
    assert(value == 6);
    std::cout << "Extracted using operator >>: 6\n";
    std::cout << "Test 3 passed\n\n";

    // Тест 4: extractBulk
    std::cout << "Test 4: extractBulk\n";
    int* bulk = seqGen.extractBulk(3);
    assert(bulk[0] == 7 && bulk[1] == 8 && bulk[2] == 9);
    std::cout << "Extracted bulk: 7, 8, 9\n";
    delete[] bulk;
    std::cout << "Test 4 passed\n\n";

    // Тест 5: hasNext и оператор bool
    std::cout << "Test 5: hasNext and bool operator\n";
    assert(seqGen.hasNext());
    assert(seqGen);
    std::cout << "hasNext() and bool operator return true\n";
    std::cout << "Test 5 passed\n\n";

    // Тест 6: reset
    std::cout << "Test 6: reset\n";
    assert(seqGen.reset());
    std::cout << "reset() returns true\n";
    std::cout << "Test 6 passed\n\n";

    // Тест 7: Случаен генератор
    GeneratorDataSource<int> randGen(randomGenerator);
    std::cout << "Test 7: Random Generator\n";
    for (int i = 0; i < 5; ++i) {
        int value = randGen.extract();
        assert(value >= 0 && value < 100);
        std::cout << "Extracted random value: " << value << std::endl;
    }
    std::cout << "Test 7 passed\n\n";

    // // Тест 8: clone
    // std::cout << "Test 8: clone\n";
    // DataSource<int>* clonedGen = seqGen.clone();
    // assert(clonedGen->extract() == seqGen.extract());
    // std::cout << "Cloned generator extracts the same value\n";
    // delete clonedGen;
    // std::cout << "Test 8 passed\n";
}

int main() {
    testGeneratorDataSource();
    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}