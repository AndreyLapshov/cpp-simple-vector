#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
   public:
    explicit ReserveProxyObj(size_t size) : capacity_{size} {}
    size_t GetSize() const {
        return capacity_;
    }

   private:
    size_t capacity_ = 0;
};

template <typename Type>
class SimpleVector {
   public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept;
  
    explicit SimpleVector(size_t size);

    explicit SimpleVector(ReserveProxyObj capacity);

    SimpleVector(size_t size, const Type& value);

    SimpleVector(Iterator begin, Iterator end);

    SimpleVector(ConstIterator begin, ConstIterator end);

    SimpleVector(std::initializer_list<Type> init);

    SimpleVector(const SimpleVector& other);

    SimpleVector(SimpleVector&& other);

    SimpleVector& operator=(const SimpleVector& rhs);

    SimpleVector& operator=(SimpleVector&& rhs);

    size_t GetSize() const noexcept;

    size_t GetCapacity() const noexcept;

    bool IsEmpty() const noexcept;
  
    Type& operator[](size_t index) noexcept;
 
    const Type& operator[](size_t index) const noexcept;

    Type& At(size_t index);

    const Type& At(size_t index) const;

    void Clear() noexcept;

    void Resize(size_t new_size);

    Iterator begin() noexcept;

    Iterator end() noexcept;

    ConstIterator begin() const noexcept;

    ConstIterator end() const noexcept;

    ConstIterator cbegin() const noexcept;

    ConstIterator cend() const noexcept;

    template <class T>
    void PushBack(T&& item);

    template <typename T>
    Iterator Insert(ConstIterator pos, T&& value);

    void PopBack() noexcept;

    Iterator Erase(ConstIterator pos);

    void swap(SimpleVector& other) noexcept;

    void Reserve(size_t new_capacity);

   private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void FillDefaultValues(Iterator begin, Iterator end) {
        assert(begin <= end);
        size_t size = (size_t)(end - begin);
        ArrayPtr<Type> tmp{size};
        Type* ptr = tmp.Get();
        std::move(ptr, ptr + size, begin);
    }
};

// ----------------------------------------------------------------
// SimpleVector operators overrides and helper functions
// ----------------------------------------------------------------

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return &lhs == &rhs || (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

inline ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
//------------------------------------------------------------------------------------
template <typename Type>
SimpleVector<Type>::SimpleVector() noexcept : SimpleVector(0) {}

template <typename Type>
SimpleVector<Type>::SimpleVector(size_t size) : array_(size), size_{size}, capacity_{size} {}

template <typename Type>
SimpleVector<Type>::SimpleVector(ReserveProxyObj capacity) : SimpleVector() {
    Reserve(capacity.GetSize());
}

template <typename Type>
SimpleVector<Type>::SimpleVector(size_t size, const Type& value) : array_(size, value), size_{size}, capacity_{size} {}

template <typename Type>
SimpleVector<Type>::SimpleVector(Iterator begin, Iterator end) : SimpleVector((assert(end >= begin), (size_t)(end - begin))) {
    if (begin == end) {
        return;
    }
    std::move(begin, end, this->begin());
}

template <typename Type>
SimpleVector<Type>::SimpleVector(ConstIterator begin, ConstIterator end) : SimpleVector((assert(end >= begin), end - begin)) {
    if (begin == end) {
        return;
    }
    std::copy(begin, end, this->begin());
}

template <typename Type>
SimpleVector<Type>::SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.begin(), init.end()) {}

template <typename Type>
SimpleVector<Type>::SimpleVector(const SimpleVector& other) {
    *this = other;
}

template <typename Type>
SimpleVector<Type>::SimpleVector(SimpleVector&& other) {
    SimpleVector<Type> empty{};
    this->swap(other);
    other.swap(empty);
}

template <typename Type>
SimpleVector<Type>& SimpleVector<Type>::operator=(const SimpleVector& rhs) {
    if (*this == rhs) {
        return *this;
    }
    if (rhs.GetCapacity() == 0) {
        Clear();
    }
    SimpleVector tmp(rhs.cbegin(), rhs.cend());
    this->swap(tmp);

    return *this;
}

template <typename Type>
SimpleVector<Type>& SimpleVector<Type>::operator=(SimpleVector&& rhs) {
    if (*this == rhs) {
        return *this;
    }
    SimpleVector<Type> empty{};
    this->swap(rhs);
    rhs.swap(empty);
    return *this;
}

template <typename Type>
size_t SimpleVector<Type>::GetSize() const noexcept {
    return size_;
}

template <typename Type>
size_t SimpleVector<Type>::GetCapacity() const noexcept {
    return capacity_;
}

template <typename Type>
bool SimpleVector<Type>::IsEmpty() const noexcept {
    return size_ == 0;
}

template <typename Type>
Type& SimpleVector<Type>::operator[](size_t index) noexcept {
    assert(index < size_);
    return begin()[index];
}

template <typename Type>
const Type& SimpleVector<Type>::operator[](size_t index) const noexcept {
    assert(index < size_);
    return begin()[index];
}

template <typename Type>
Type& SimpleVector<Type>::At(size_t index) {
    if (index >= GetSize()) {
        throw std::out_of_range("index out of range");
    }
    return array_[index];
}

template <typename Type>
const Type& SimpleVector<Type>::At(size_t index) const {
    if (index >= GetSize()) {
        throw std::out_of_range("index out of range");
    }
    return array_[index];
}

template <typename Type>
void SimpleVector<Type>::Clear() noexcept {
    Resize(0);
}

template <typename Type>
void SimpleVector<Type>::Resize(size_t new_size) {
    if (new_size > size_ && new_size <= capacity_) {
        size_t old_size = size_;
        size_ = new_size;
        FillDefaultValues(begin() + old_size, begin() + new_size);
    } else if (new_size > capacity_) {
        SimpleVector buffer{begin(), end()};
        buffer.Reserve(new_size);
        buffer.size_ = new_size;
        FillDefaultValues(buffer.begin() + size_, buffer.end());
        this->swap(buffer);
    } else {
        size_ = new_size;
    }
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::begin() noexcept {
    return size_ ? &(array_[0]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::end() noexcept {
    return size_ ? &(array_[size_]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::begin() const noexcept {
    return size_ ? &(array_[0]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::end() const noexcept {
    return size_ ? &(array_[size_]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::cbegin() const noexcept {
    return begin();
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::cend() const noexcept {
    return end();
}

template <typename Type>
template <typename T>
void SimpleVector<Type>::PushBack(T&& item) {
    size_t old_size = size_;
    if (size_ == capacity_) {
        size_t new_capacity = std::max((unsigned long)capacity_, 1ul) * 2;
        Reserve(new_capacity);
    }
    Resize(old_size + 1);
    array_[old_size] = std::move(item);
}

template <typename Type>
template <typename T>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::Insert(ConstIterator pos, T&& value) {
    assert(pos >= begin() && pos <= end());

    if (pos == end() && capacity_ > size_) {
        array_[size_++] = std::move(value);
        return end() - 1;
    }
    size_t new_capacity = size_ == capacity_ ? std::max((unsigned long)capacity_, 1ul) * 2 : capacity_;
    size_t new_size = size_ + 1;
    size_t input_index = (size_t)(pos - cbegin());
    SimpleVector buffer{begin(), end()};
    buffer.Reserve(new_capacity);
    buffer.Resize(new_size);

    std::move_backward(buffer.begin() + input_index, buffer.begin() + size_, buffer.end());

    buffer[input_index] = std::move(value);
    this->swap(buffer);

    return begin() + input_index;
}

template <typename Type>
void SimpleVector<Type>::PopBack() noexcept {
    assert(size_);
    --size_;
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::Erase(ConstIterator pos) {
    assert(pos >= begin() && pos < end());

    size_t erase_index = (size_t)(pos - cbegin());
    Iterator new_first = &(array_[erase_index]);
    std::move(begin() + erase_index + 1, end(), new_first);
    --size_;
    return begin() + erase_index;
}

template <typename Type>
void SimpleVector<Type>::swap(SimpleVector& other) noexcept {
    if (this == &other) {
        return;
    }
    this->array_.swap(other.array_);
    std::swap(this->size_, other.size_);
    std::swap(this->capacity_, other.capacity_);
}

template <typename Type>
void SimpleVector<Type>::Reserve(size_t new_capacity) {
    if (new_capacity <= capacity_) {
        return;
    }
    SimpleVector tmp(new_capacity);
    tmp.size_ = size_;
    std::move(begin(), end(), tmp.begin());
    this->swap(tmp);
}