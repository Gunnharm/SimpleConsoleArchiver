#pragma once

#include <vector>
#include <functional>
#include <stdexcept>

template <typename T, typename Container = std::vector<T>, typename Comparator = std::less<T>>
class MyPriorityQueue {
public:
    explicit MyPriorityQueue(const Comparator& cmp = std::less<T>());
    explicit MyPriorityQueue(const Container& container, const Comparator& cmp = std::less<T>());

    void push(const T& value);  // NOLINT
    void pop();                 // NOLINT
    T top() const;              // NOLINT
    size_t size() const;        // NOLINT
    bool empty() const;         // NOLINT

protected:
    void Heapify(size_t id);
    void Pull(size_t id);

    Container container_;
    Comparator cmp_;
};

template <typename T, typename Container, typename Comparator>
T MyPriorityQueue<T, Container, Comparator>::top() const {
    return container_[0];
}
template <typename T, typename Container, typename Comparator>
void MyPriorityQueue<T, Container, Comparator>::Heapify(size_t id) {
    while (true) {
        size_t left_child = id * 2 + 1;
        size_t right_child = id * 2 + 2;
        size_t greatest_element = id;
        if (left_child < container_.size() && cmp_(container_[greatest_element], container_[left_child])) {
            greatest_element = left_child;
        }
        if (right_child < container_.size() && cmp_(container_[greatest_element], container_[right_child])) {
            greatest_element = right_child;
        }
        if (greatest_element == id) {
            break;
        }
        std::swap(container_[id], container_[greatest_element]);
        id = greatest_element;
    }
}
template <typename T, typename Container, typename Comparator>
MyPriorityQueue<T, Container, Comparator>::MyPriorityQueue(const Container& container, const Comparator& cmp)
    : container_(container), cmp_(cmp) {
    for (ssize_t id = static_cast<ssize_t>(container_.size()) - 1; id >= 0; --id) {
        Heapify(id);
    }
}
template <typename T, typename Container, typename Comparator>
MyPriorityQueue<T, Container, Comparator>::MyPriorityQueue(const Comparator& cmp) : cmp_(cmp) {
}
template <typename T, typename Container, typename Comparator>
void MyPriorityQueue<T, Container, Comparator>::push(const T& value) {
    container_.emplace_back(value);
    Pull(container_.size() - 1);
}
template <typename T, typename Container, typename Comparator>
void MyPriorityQueue<T, Container, Comparator>::pop() {
    if (container_.empty()) {
        throw std::range_error("Trying to pop from empty priority_queue");
    }
    std::swap(container_[0], container_.back());
    container_.pop_back();
    Heapify(0);
}
template <typename T, typename Container, typename Comparator>
void MyPriorityQueue<T, Container, Comparator>::Pull(size_t id) {
    while (id > 0 && cmp_(container_[(id - 1) / 2], container_[id])) {
        std::swap(container_[id], container_[(id - 1) / 2]);
        id = (id - 1) / 2;
    }
}
template <typename T, typename Container, typename Comparator>
size_t MyPriorityQueue<T, Container, Comparator>::size() const {
    return container_.size();
}
template <typename T, typename Container, typename Comparator>
bool MyPriorityQueue<T, Container, Comparator>::empty() const {
    return container_.empty();
}
