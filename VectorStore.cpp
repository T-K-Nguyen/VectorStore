#include "VectorStore.h"

// ----------------- ArrayList Implementation -----------------

template <class T>
ArrayList<T>::ArrayList(int initCapacity = 10) {
    capacity = (initCapacity > 0) ? initCapacity : 10;
    data = new T[capacity];
    count = 0;
}

template <class T>
ArrayList<T>::ArrayList(const ArrayList<T>& other) {
    capacity = other.capacity;
    count = other.count;
    data = new T[capacity];
    for (int i = 0; i < count; ++i) {
        data[i] = other.data[i];
    }
}   

template <class T>
ArrayList<T>::~ArrayList() {
    delete[] data;
}

// TODO: implement other methods of ArrayList



// ----------------- Iterator of ArrayList Implementation -----------------
template <class T>
ArrayList<T>::Iterator::Iterator(ArrayList<T>* pList, int index) {
    this->pList = pList;
    cursor = (index >= 0 && index <= pList->count) ? index : 0;
}

// TODO: implement other methods of ArrayList::Iterator



// ----------------- SinglyLinkedList Implementation -----------------
template <class T>
SinglyLinkedList<T>::SinglyLinkedList() {
    head = nullptr;
    tail = nullptr;
    count = 0;
}

template <class T>
SinglyLinkedList<T>::~SinglyLinkedList() {
    clear();
}   

// TODO: implement other methods of SinglyLinkedList



// ----------------- Iterator of SinglyLinkedList Implementation -----------------
template <class T>
SinglyLinkedList<T>::Iterator::Iterator(Node* node) {
    current = node;
}

// TODO: implement other methods of SinglyLinkedList::Iterator



// ----------------- VectorStore Implementation -----------------

VectorStore::VectorStore(int dimension = 512, EmbedFn embeddingFunction = nullptr) {
    this->dimension = (dimension > 0) ? dimension : 512;
    this->embeddingFunction = embeddingFunction;
    count = 0;
}

VectorStore::~VectorStore() {
    clear();
}

// TODO: implement other methods of VectorStore
void VectorStore::clear() {
    for (int i = 0; i < records.size(); ++i) {
        delete records.get(i)->vector;
        delete records.get(i);
    }
    records.clear();
}

// Explicit template instantiation for char, string, int, double, float, and Point

template class ArrayList<char>;
template class ArrayList<string>;
template class ArrayList<int>;
template class ArrayList<double>;
template class ArrayList<float>;
template class ArrayList<Point>;

template class SinglyLinkedList<char>;
template class SinglyLinkedList<string>;
template class SinglyLinkedList<int>;
template class SinglyLinkedList<double>;
template class SinglyLinkedList<float>;
template class SinglyLinkedList<Point>;
