#include "VectorStore.h"

// ----------------- ArrayList Implementation -----------------

template <class T>
ArrayList<T>::ArrayList(int initCapacity) {
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

template <class T>
void ArrayList<T>::ensureCapacity(int cap) { // thêm overflow check do fail test 19
    if (cap <= capacity) return;

    // use hard-coded 32-bit signed max để tránh overflow khi nhân chia
    const long long MAX_INT32 = 2147483647LL;
    long long proposed = static_cast<long long>(capacity);
    // Grow roughly by 1.5x until reaching requirement (single step acceptable here)
    proposed = proposed + (proposed >> 1); // *1.5
    if (proposed <cap) proposed = cap; // ensure at least required
    if (proposed > MAX_INT32) throw std::overflow_error("Requested capacity too large");

    int newCapacity = static_cast<int>(proposed);
    T* newData = new T[newCapacity];
    for (int i = 0; i < count; ++i) newData[i] = data[i];
    delete[] data;
    data = newData;
    capacity = newCapacity;
}

template <class T>
ArrayList<T>& ArrayList<T>::operator=(const ArrayList<T>& other) { // thêm exception safety do fail test 31
    if (this == &other) return *this;

    // thử Allocate trước khi xóa
    T* newData = new T[other.capacity];
    for (int i = 0; i < other.count; ++i) {
        newData[i] = other.data[i];
    }
    // Now commit.
    delete[] data;
    data = newData;
    capacity = other.capacity;
    count = other.count;
    return *this;
}

template <class T>
void ArrayList<T>::add(T e) {
    ensureCapacity(count + 1);
    data[count++] = e;
}

template <class T>
void ArrayList<T>::add(int index, T e) {
    if (index < 0 || index > count) throw std::out_of_range("Index is out of range!");
    
    ensureCapacity(count + 1);
    for (int i = count; i > index; --i) {
        data[i] = data[i - 1];
    }
    data[index] = e;
    ++count;
}

template <class T>
T ArrayList<T>::removeAt(int index) {
    if (index < 0 || index >=count) throw std::out_of_range("ArrayList::removeAt - index out of range");
    
    T element = data[index];
    for (int i = index; i < count - 1; ++i) {
        data[i] = data[i + 1];
    }
    --count;
    return element;
}

template <class T>
void ArrayList<T>::clear() {
    delete[] data;
    capacity = 10;
    data = new T[capacity];
    count = 0;
}

template <class T>
bool ArrayList<T>::empty() const {
    return count == 0;
}

template <class T>
T& ArrayList<T>::get(int index) {
    if (index < 0 || index >= count) throw std::out_of_range("ArrayList::get - index out of range");
    
    return data[index];
}

template <class T>
void ArrayList<T>::set(int index, T e) {
    if (index < 0 || index >= count) throw std::out_of_range("ArrayList::set - index out of range");
    
    data[index] = e;
}

template <class T>
int ArrayList<T>::indexOf(T item) const {
    for (int i = 0; i < count; ++i) {
        if (data[i] == item) {
            return i;
        }
    }
    return -1;
}

template <class T>
bool ArrayList<T>::contains(T item) const {
    return indexOf(item) != -1;
}

template <class T>
string ArrayList<T>::toString(string (*item2str)(T&)) const {
    stringstream ss;
    ss << '[';
    for (int i = 0; i < count; ++i) {
        if (i > 0) ss << ", ";
        if (item2str) {
            ss << item2str(data[i]);
        } else {
            ss << data[i];
        }
    }
    ss << ']';
    return ss.str();
}

template <class T>
int ArrayList<T>::size() const {
    return count;
}


// ----------------- Iterator of ArrayList Implementation -----------------
template <class T>
ArrayList<T>::Iterator::Iterator(ArrayList<T>* pList, int index) {
    this->pList = pList; // code của thầy chưa ổn cần thêm kiểm tra index
    if (pList && (index < 0 || index > pList->count)) throw std::out_of_range("Index is out of range!");
    
    cursor = (pList) ? index : 0;
}

// TODO: implement other methods of ArrayList::Iterator

template <class T>
typename ArrayList<T>::Iterator ArrayList<T>::begin() {
    return Iterator(this, 0);
}

template <class T>
typename ArrayList<T>::Iterator ArrayList<T>::end() {
    return Iterator(this, count);
}

template <class T>
typename ArrayList<T>::Iterator& ArrayList<T>::Iterator::operator=(const Iterator& other) {
    if (this != &other) {
        pList = other.pList;
        cursor = other.cursor;
    }
    return *this;
}

template <class T>
T& ArrayList<T>::Iterator::operator*() {
    if (!pList || cursor < 0 || cursor >= pList->count) 
        throw std::out_of_range("Iterator is out of range!");
    
    return pList->data[cursor];
}

template <class T>
bool ArrayList<T>::Iterator::operator!=(const Iterator& other) const {
    return pList != other.pList || cursor != other.cursor;
}

template <class T>
typename ArrayList<T>::Iterator& ArrayList<T>::Iterator::operator++() {
    if (!pList || cursor >= pList->count) 
        throw std::out_of_range("Iterator cannot advance past end!");
    
    ++cursor;
    return *this;
}

template <class T>
typename ArrayList<T>::Iterator ArrayList<T>::Iterator::operator++(int) {
    if (!pList || cursor >= pList->count) throw std::out_of_range("Iterator cannot advance past end!");
    
    Iterator temp = *this;
    ++cursor;
    return temp;
}

template <class T>
typename ArrayList<T>::Iterator& ArrayList<T>::Iterator::operator--() {
    if (!pList || cursor <= 0) throw std::out_of_range("Iterator cannot move before begin!");
    
    --cursor;
    return *this;
}

template <class T>
typename ArrayList<T>::Iterator ArrayList<T>::Iterator::operator--(int) {
    if (!pList || cursor <= 0) 
        throw std::out_of_range("Iterator cannot move before begin!");
    
    Iterator temp = *this;
    --cursor;
    return temp;
}

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
template <class T>
void SinglyLinkedList<T>::clear() {
    while (head != nullptr) { // making sure all nodes are deleted to avoid memory leaks, takes O(n)
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    tail = nullptr;
    count = 0;
}

template <class T>
void SinglyLinkedList<T>::add(T e) {
    Node* newNode = new Node(e);
    if (!head) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    ++count;
}

template <class T>
T& SinglyLinkedList<T>::get(int index) {
    if (index < 0 || index >= count) 
        throw std::out_of_range("Index is invalid!");
    
    Node* current = head;
    for (int i = 0; i < index; ++i) {
        current = current->next;
    }
    return current->data;
}

template <class T>
int SinglyLinkedList<T>::size() const {
    return count;
}


// ----------------- Iterator of SinglyLinkedList Implementation -----------------
template <class T>
SinglyLinkedList<T>::Iterator::Iterator(Node* node) {
    current = node;
}

// TODO: implement other methods of SinglyLinkedList::Iterator



// ----------------- VectorStore Implementation -----------------

VectorStore::VectorStore(int dimension, EmbedFn setEmbeddingFunction) {
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

SinglyLinkedList<float>* VectorStore::preprocessing(string rawText) {
    SinglyLinkedList<float>* result = nullptr;
    if (embeddingFunction) {
        result = embeddingFunction(rawText); //Invoke embeddingFunction to map rawText into a vector.
    } else {
        result = new SinglyLinkedList<float>();
        int len = rawText.length() > dimension ? dimension : rawText.length();
        for (int i = 0; i < len; ++i) {
            result->add(static_cast<float>(rawText[i]));
        }
        // Pad with zeros if less than dimension
        while (result->size() < dimension) {
            result->add(0.0f);
        }
    }
    return result;
}

void VectorStore::addText(string rawText) {
    SinglyLinkedList<float>* vector = preprocessing(rawText);
    VectorRecord* record = new VectorRecord(count, rawText, vector);
    records.add(record);
    ++count;
}

SinglyLinkedList<float>& VectorStore::getVector(int index) {
    if (index < 0 || index >= records.size()) {
        throw std::out_of_range("Index is invalid!");
    }
    return *(records.get(index)->vector);
}

int VectorStore::size() const {
    return records.size();
}

bool VectorStore::empty() const {
    return records.size() == 0;
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
