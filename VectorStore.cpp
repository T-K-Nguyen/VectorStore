#include "VectorStore.h"

// ----------------- ArrayList Implementation -----------------

template <class T>
ArrayList<T>::ArrayList(int initCapacity = 10) {
    // TODO
}

template <class T>
ArrayList<T>::ArrayList(const ArrayList<T>& other) {
    // TODO
}   

template <class T>
ArrayList<T>::~ArrayList() {
    // TODO
}

// TODO: implement other methods of ArrayList



// ----------------- Iterator of ArrayList Implementation -----------------
template <class T>
ArrayList<T>::Iterator::Iterator(ArrayList<T>* pList, int index) {
    // TODO
}

// TODO: implement other methods of ArrayList::Iterator



// ----------------- SinglyLinkedList Implementation -----------------
template <class T>
SinglyLinkedList<T>::SinglyLinkedList() {
    // TODO
}   

template <class T>
SinglyLinkedList<T>::~SinglyLinkedList() {
    // TODO
}   

// TODO: implement other methods of SinglyLinkedList



// ----------------- Iterator of SinglyLinkedList Implementation -----------------
template <class T>
SinglyLinkedList<T>::Iterator::Iterator(Node* node) {
    // TODO
}   

// TODO: implement other methods of SinglyLinkedList::Iterator



// ----------------- VectorStore Implementation -----------------

VectorStore::VectorStore(int dimension = 512, EmbedFn embeddingFunction = nullptr) {
    // TODO
}

VectorStore::~VectorStore() {
    // TODO
}

// TODO: implement other methods of VectorStore


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
