#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <new>
#include <stdexcept>
#include <iostream>


template <typename T>
class DataSource {
public:
    virtual ~DataSource() = default;

    virtual T operator()() = 0;
    virtual DataSource& operator>>(T& element) = 0;
    virtual operator bool() const = 0;

    virtual DataSource* clone() const = 0;

    virtual T extract() = 0;
    virtual T* extractBulk(size_t count) = 0;

    virtual bool hasNext() const = 0;
    virtual bool reset() = 0;
};


template <typename T>
class DefaultDataSource: public DataSource<T> {
public:
    DefaultDataSource() = default;
    ~DefaultDataSource() _NOEXCEPT = default;

    DataSource<T>* clone() const override;

    T operator()() override;
    DataSource<T>& operator>>(T& element) override;
    operator bool() const override;

    T extract() override;
    T* extractBulk(size_t count) override;

    bool hasNext() const override;
    bool reset() override;
};

template <typename T>
T DefaultDataSource<T>::operator()() {
    return extract();
}

template <typename T>
DataSource<T>& DefaultDataSource<T>::operator>>(T &element) {
    element = extract();
    return *this;
}

template <typename T>
DefaultDataSource<T>::operator bool() const {
    return hasNext();
}

template <typename T>
DataSource<T>* DefaultDataSource<T>::clone() const {
    return new DefaultDataSource(*this);
}

template <typename T>
T DefaultDataSource<T>::extract() {
    return T();
}

template <typename T>
T* DefaultDataSource<T>::extractBulk(size_t count) {
    T* batch = new T[count];
    for (size_t i = 0; i < count; i++) {
        batch[i] = T();
    }
    return batch;
}

template <typename T>
bool DefaultDataSource<T>::hasNext() const {
    return true;
}

template <typename T>
bool DefaultDataSource<T>::reset() {
    return true;
}

template <typename T>
class FileDataSource: public DataSource<T> {
public:
    explicit FileDataSource(const char* fileName);
    FileDataSource(const FileDataSource<T>& other);
    ~FileDataSource() _NOEXCEPT override;

    FileDataSource& operator=(const FileDataSource<T>& other);
    
    T operator()() override;
    DataSource<T>& operator>>(T& element) override;
    operator bool() const override;

    DataSource<T>* clone() const override;

    T extract() override;
    T* extractBulk(size_t count) override;

    bool hasNext() const override;
    bool reset() override;

private:
    void openFile(const char* fileName);
    void setFileName(const char* fileName);
    void copy(const FileDataSource<T>& other);
    void free();

private:
    char* fileName;
    std::ifstream file;
};

template <typename T>
FileDataSource<T>::FileDataSource(const char* fileName) 
    :fileName(nullptr) {
    try {
        setFileName(fileName);
        openFile(fileName);

    } catch (const std::runtime_error& e) {
        free();
        throw;
    } catch (const std::invalid_argument& e) {
        free();
        throw;
    } catch (const std::bad_alloc& e) {
        free();
        throw;
    }
}


template <typename T>
FileDataSource<T>::FileDataSource(const FileDataSource<T>& other)
    :fileName(nullptr) {
    copy(other);
}

template <typename T>
FileDataSource<T>::~FileDataSource<T>() _NOEXCEPT {
    file.close();
    free();
}

template <typename T>
FileDataSource<T>& FileDataSource<T>::operator=(const FileDataSource<T> &other) {
    if (this != &other) {
        free();
        copy(other);
    }
    return *this;
}

template <typename T>
T FileDataSource<T>::operator()() {
    return extract();
}

template <typename T>
DataSource<T>& FileDataSource<T>::operator>>(T &element) {
    element = extract();
    return *this;
}

template <typename T>
FileDataSource<T>::operator bool() const {
    return hasNext();
}

template <typename T>
DataSource<T>* FileDataSource<T>::clone() const {
    return new FileDataSource(*this);
}

// template <typename T>
// T FileDataSource<T>::extract() {
//     if (!hasNext()) {
//         throw std::runtime_error("No more data in file data source");
//     }
//     T element;
//     file >> element;
//     // if (!file) {
//     //     // throw std::runtime_error("No more data ");
//     //     throw std::runtime_error("(!file) threw this exception");
//     // }
//     if (file.fail() && !file.eof()) {
//         throw std::runtime_error("Error reading from file");
//     }
//     return element;
// }

template <typename T>
T FileDataSource<T>::extract() {
    if (!hasNext()) {
        throw std::runtime_error("No more data in file data source");
    }
    T element;
    if (!(file >> element)) {
        throw std::runtime_error("Error reading from file or end of file reached");
    }
    return element;
}

template <typename T>
T* FileDataSource<T>::extractBulk(size_t count) {
    T* batch = new T[count];
    for (size_t i = 0; i < count && hasNext(); i++) {
        batch[i] = extract();
    }
    return batch;
}

template <typename T>
bool FileDataSource<T>::hasNext() const {
    // return file.good() && !file.eof();
    return file.good() && !file.eof();
    //&& file.peek() != EOF;
}

template <typename T>
bool FileDataSource<T>::reset() {
    file.clear();
    file.seekg(0, std::ios::beg);
    return file.good();
}

template <typename T>
void FileDataSource<T>::openFile(const char* fileName) {
    if (!fileName) {
        throw std::invalid_argument("File name cannot be nullptr");
    }
    file.open(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open file");
    }
}

template <typename T>
void FileDataSource<T>::setFileName(const char* fileName) {
    if (!fileName) {
        throw std::invalid_argument("File name cannot be nullptr");
    }
    this->fileName = new char[strlen(fileName) + 1];
    if (!this->fileName) {
        throw std::bad_alloc();
    }
    strcpy(this->fileName, fileName);
}

template <typename T>
void FileDataSource<T>::copy(const FileDataSource<T>& other) {
    setFileName(other.fileName);
    openFile(other.fileName);
}

template <typename T>
void FileDataSource<T>::free() {
    delete [] fileName;
    fileName = nullptr;
}

template <typename T>
class ArrayDataSource: public DataSource<T> {
public:
    explicit ArrayDataSource(T* array, size_t arrSize);
    ArrayDataSource(const ArrayDataSource<T>& other);
    ~ArrayDataSource() _NOEXCEPT override;

    ArrayDataSource& operator=(const ArrayDataSource<T>& other);
    ArrayDataSource& operator+=(const T& element);
    ArrayDataSource operator+(const T& element);

    T operator()() override;
    DataSource<T>& operator>>(T& element) override;
    operator bool() const override;
    
    DataSource<T>* clone() const override;

    T extract() override;
    T* extractBulk(size_t cout) override;

    bool hasNext() const override;
    bool reset() override;

private:
    void copy(const ArrayDataSource<T>& other);
    void free();
    void resize(size_t step = INCREMENT_STEP);
    void reserve(size_t capacity);

private:
    static const size_t STARTING_POSITION = 0;
    static const size_t INCREMENT_STEP = 2;
private:
    size_t size;
    size_t capacity;
    size_t currentPos;
    T* data;
};

template <typename T>
ArrayDataSource<T>::ArrayDataSource(T* array, size_t arrSize)
    :size(arrSize), capacity(arrSize * INCREMENT_STEP), currentPos(STARTING_POSITION), data(nullptr) {
    try {
        if (!array) {
            throw std::invalid_argument("Array cannot be nullptr");
        }
        reserve(capacity);
        for (size_t i = 0; i < arrSize; i++) {
            data[i] = array[i];
        }

    } catch (const std::invalid_argument& e) {
        free();
        throw;
    } catch (const std::bad_alloc& e) {
        free();
        throw;
    }
} 
template <typename T>
ArrayDataSource<T>::ArrayDataSource(const ArrayDataSource<T>& other)
    :data(nullptr) {
    
    copy(other);
}

template <typename T>
ArrayDataSource<T>::~ArrayDataSource<T>() _NOEXCEPT {
    free();
}

template <typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator=(const ArrayDataSource<T> &other) {
    if (this != &other) {
        free();
        copy(other);
    }
    return *this;
}

template <typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator+=(const T &element) {
    if (size == capacity) {
        resize();
    }
    data[size++] = element;
    return *this;
}

template <typename T>
ArrayDataSource<T> ArrayDataSource<T>::operator+(const T &element) {
    ArrayDataSource temp(*this);
    temp += element;
    return temp;
}

template <typename T>
T ArrayDataSource<T>::operator()() {
    return extract();
}

template <typename T>
DataSource<T>& ArrayDataSource<T>::operator>>(T &element) {
    element = extract();
    return *this;
}

template <typename T>
ArrayDataSource<T>::operator bool() const {
    return hasNext();
}

template <typename T>
DataSource<T>* ArrayDataSource<T>::clone() const {
    return new ArrayDataSource(*this);
}

// template <typename T>
// T ArrayDataSource<T>::extract() {
//     if (!hasNext()) {
//         throw std::out_of_range("No more elements to extract in array data source");
//     }
//     return data[currentPos++];
// }

template <typename T>
T ArrayDataSource<T>::extract() {
    if (!hasNext()) {
        throw std::runtime_error("No more elements in array data source");
    }
    return data[currentPos++];
}

template <typename T>
T* ArrayDataSource<T>::extractBulk(size_t count) {
    if (currentPos + count > size) {
        count = size - currentPos;
    }
    T* batch = new T[count];
    for (size_t i = 0; i < count && hasNext(); i++) {
        batch[i] = data[currentPos++];
    }
    return batch;
}

template <typename T>
bool ArrayDataSource<T>::hasNext() const {
    return currentPos < size;
}

template <typename T>
bool ArrayDataSource<T>::reset() {
    currentPos = STARTING_POSITION;
    return true;
}

template <typename T>
void ArrayDataSource<T>::copy(const ArrayDataSource<T>& other) {
    this->size = other.size;
    this->capacity = other.capacity;
    this->currentPos = other.currentPos;
    reserve(other.capacity);
    for (size_t i = 0; i < other.size; i++) {
        this->data[i] = other.data[i];
    }
}

template <typename T>
void ArrayDataSource<T>::free() {
    delete [] data;
    data = nullptr;
}

template <typename T>
void ArrayDataSource<T>::resize(size_t step) {
    size_t newCapacity = capacity * step;
    T* newData = new T[newCapacity];

    for (size_t i = 0; i < size; i++) {
        newData[i] = (data[i]);
    }
    free();
    capacity = newCapacity;
    data = newData;
}

template <typename T>
void ArrayDataSource<T>::reserve(size_t capacity) {
    data = new T[capacity];
    if (!data) {
        throw std::bad_alloc();
    }
}

template <typename T>
class AlternateDataSource: public DataSource<T> {
public:
    explicit AlternateDataSource(DataSource<T>** sources, size_t sourcesCount);
    AlternateDataSource(const AlternateDataSource<T>& other);
    ~AlternateDataSource() _NOEXCEPT override;

    AlternateDataSource& operator=(const AlternateDataSource<T>& other);

    T operator()() override;
    DataSource<T>& operator>>(T& element) override;
    operator bool() const override;

    DataSource<T>* clone() const override;

    T extract() override;
    T* extractBulk(size_t count) override;

    bool hasNext() const override;
    bool reset() override;

private:
    void copy(const AlternateDataSource<T>& other);
    void free();
    void moveToAvailableSource();
    void reserve(size_t capacity);

private:
    static const size_t STARTING_POSITION = 0;
private:
    size_t size;
    size_t currentPos;
    DataSource<T>** sources;
};

template <typename T>
AlternateDataSource<T>::AlternateDataSource(DataSource<T>** sources, size_t sourcesCount)
    :size(sourcesCount), currentPos(STARTING_POSITION), sources(nullptr) {
    try {
        if (!sources) {
            throw std::invalid_argument("Sources cannot be nullptr");
        }
        reserve(sourcesCount);
        for (size_t i = 0; i < sourcesCount; i++) {
            this->sources[i] = sources[i]->clone();
        }

    } catch (const std::invalid_argument& e) {
        free();
        throw;
    } catch (const std::bad_alloc()) {
        free();
        throw;
    }
}

template <typename T>
AlternateDataSource<T>::AlternateDataSource(const AlternateDataSource<T>& other)
    :sources(nullptr) {
    
    copy(other);
}

template <typename T>
AlternateDataSource<T>::~AlternateDataSource<T>() _NOEXCEPT {
    free();
}

template <typename T>
AlternateDataSource<T>& AlternateDataSource<T>::operator=(const AlternateDataSource<T>& other) {
    if (this != &other) {
        free();
        copy(other);
    }
    return *this;
}

template <typename T>
T AlternateDataSource<T>::operator()() {
    return extract();
}

template <typename T>
DataSource<T>& AlternateDataSource<T>::operator>>(T &element) {
    element = extract();
    return *this;
}

template <typename T>
AlternateDataSource<T>::operator bool() const {
    return hasNext();
}

template <typename T>
DataSource<T>* AlternateDataSource<T>::clone() const {
    return new AlternateDataSource(*this);
}

//_____DOESN'T WORK_________-
// template <typename T>
// T AlternateDataSource<T>::extract() {
//     // if (!sources || currentPos >= size) {
//     //     throw std::runtime_error("Invalid state in AlternateDataSource");
//     // }
//     // std::cout << "Extracting from source" << currentPos << '\n';
//     // T element = sources[currentPos]->extract();
//     // moveToAvailableSource();
//     // return element;
//     if (!hasNext()) {
//         throw std::runtime_error("No more data in any source");
//     }
//     for (size_t i = 0; i < size; i++) {
//         size_t sourceIndex = (currentPos + i) % size;
//         if (sources[sourceIndex]->hasNext()) {
//             std::cout << "Extracting from source" << sourceIndex << '\n';
//             T element = sources[sourceIndex]->extract();
//             currentPos = (sourceIndex + 1) % size;
//             return element;
//         }
//     }

//     //should'nt be reached
//     throw std::runtime_error("Unexpected end of data");
// }


//_____DOESEN'T WORK__________
// template <typename T>
// T AlternateDataSource<T>::extract() {
//     if (!hasNext()) {
//         throw std::runtime_error("No more data in any source");
//     }
    
//     size_t initialPos = currentPos;
//     do {
//         if (sources[currentPos]->hasNext()) {
//             std::cout << "Extracting from source" << currentPos << '\n';
//             T element = sources[currentPos]->extract();
//             moveToAvailableSource();
//             return element;
//         }
//         currentPos = (currentPos + 1) % size;
//     } while (currentPos != initialPos);
    
//     // This line should not be reached if hasNext() works correctly
//     throw std::runtime_error("Unexpected end of data");
// }


//____THIS WORKED TOO____
// template <typename T>
// T AlternateDataSource<T>::extract() {
//     if (!hasNext()) {
//         throw std::runtime_error("No more data in any source");
//     }
    
//     size_t initialPos = currentPos;
//     do {
//         try {
//             if (sources[currentPos]->hasNext()) {
//                 std::cout << "Extracting from source" << currentPos << '\n';
//                 T element = sources[currentPos]->extract();
//                 moveToAvailableSource();
//                 return element;
//             }
//         } catch (const std::runtime_error&) {
//             // Ако този източник хвърли грешка, преминаваме към следващия
//         }
//         currentPos = (currentPos + 1) % size;
//     } while (currentPos != initialPos);
    
//     throw std::runtime_error("Unexpected end of data");
// }

//_________THIS WORKED________
template <typename T>
T AlternateDataSource<T>::extract() {
    if (!hasNext()) {
        throw std::runtime_error("No more data in any source");
    }
    
    size_t initialPos = currentPos;
    do {
        try {
            if (sources[currentPos]->hasNext()) {
                // std::cout << "Extracting from source" << currentPos << '\n';
                T element = sources[currentPos]->extract();
                moveToAvailableSource();
                return element;
            }
        } catch (const std::runtime_error& e) {
            //Ако този източник хвърли грешка, преминаваме към следващия
            std::cout << "Source " << currentPos << " exhausted: " << e.what() << '\n';
        }
        currentPos = (currentPos + 1) % size;
    } while (currentPos != initialPos);
    
    throw std::runtime_error("Unexpected end of data");
}


template <typename T>
T* AlternateDataSource<T>::extractBulk(size_t count) {
    T* batch = new T[count];
    size_t extracted = 0;
    while (extracted < count && hasNext()) {
        batch[extracted++] = extract();
    }
    return batch;
}

template <typename T>
bool AlternateDataSource<T>::hasNext() const {
    for (size_t i = 0; i < size ; i++) {
        if (sources[i]->hasNext()) {
            return true;
        }
    }
    return false;
}

template <typename T>
bool AlternateDataSource<T>::reset() {
    bool allReset = true;
    for (size_t i = 0; i < size; i++) {
        if (sources[i]->reset()) {
            allReset = true;
        }
    }
    currentPos = STARTING_POSITION;
    return allReset;
}

template <typename T>
void AlternateDataSource<T>::copy(const AlternateDataSource<T>& other) {
    this->size = other.size;
    this->currentPos = other.currentPos;
    reserve(other.size);

    for (size_t i = 0; i < other.size; i++) {
        this->sources[i] = other.sources[i]->clone();
    }
}

template <typename T>
void AlternateDataSource<T>::free() {
    for (size_t i = 0; i < size; i++) {
        delete sources[i];
    }
    delete [] sources;
    
    sources = nullptr;
}

template <typename T>
void AlternateDataSource<T>::moveToAvailableSource() {
    // do {
    //     currentPos = (currentPos + 1) % size;
    // }while (!sources[currentPos]->hasNext() && currentPos != STARTING_POSITION);
    size_t initialPos = currentPos;
    do {
        currentPos = (currentPos + 1) % size;
        if (sources[currentPos]->hasNext()) {
            return;
        }
    }while (currentPos != initialPos);

    // If we've checked all sources and none have data, reset currentPos
    // currentPos = STARTING_POSITION;
}

template <typename T>
void AlternateDataSource<T>::reserve(size_t capacity) {
    sources = new DataSource<T>* [capacity];
    if (!sources) {
        throw std::bad_alloc();
    }
}

template <typename T>
class GeneratorDataSource: public DataSource<T> {
public:
    explicit GeneratorDataSource(T (*generatorFunc)());
    ~GeneratorDataSource() _NOEXCEPT  = default;

    T operator()() override;
    DataSource<T>& operator>>(T& element) override;
    operator bool() const override;

    DataSource<T>* clone() const override;

    T extract() override;
    T* extractBulk(size_t count) override;

    bool hasNext() const override;
    bool reset() override;

private:    
    T (*generatorFunc)();
};

template <typename T>
GeneratorDataSource<T>::GeneratorDataSource(T (*generatorFunc)())
    :generatorFunc(generatorFunc) {}

template <typename T>
T GeneratorDataSource<T>::operator()() {
    return extract();
}

template <typename T>
DataSource<T>& GeneratorDataSource<T>::operator>>(T &element) {
    element = extract();
    return *this;
}

template <typename T>
GeneratorDataSource<T>::operator bool() const {
    return hasNext();
}

template <typename T>
DataSource<T>* GeneratorDataSource<T>::clone() const {
    return new GeneratorDataSource(*this);
}

template <typename T>
T GeneratorDataSource<T>::extract() {
    return generatorFunc();
}

template <typename T>
T* GeneratorDataSource<T>::extractBulk(size_t count) {
    T* batch = new T[count];
    for (size_t i = 0; i < count; i++) {
        batch[i] = generatorFunc();
    }
    return batch;
}

template <typename T>
bool GeneratorDataSource<T>::hasNext() const {
    return true;
}

template <typename T>
bool GeneratorDataSource<T>::reset() {
    return true;
}



