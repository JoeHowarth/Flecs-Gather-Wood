#pragma once

#include <array>

template <typename T, std::size_t N>
class ArrayVec {
   public:
    ArrayVec() : size_(0) {}

    ArrayVec(std::initializer_list<T> init_list) : size_(0) {
        if (init_list.size() > N) {
            throw std::out_of_range(
                "Initializer list size exceeds ArrayVec capacity"
            );
        }
        for (const auto& value : init_list) {
            data_[size_++] = value;
        }
    }

    void push_back(const T& value) {
        if (size_ < N) {
            data_[size_++] = value;
        } else {
            throw std::out_of_range("ArrayVec is full");
        }
    }

    std::size_t size() const {
        return size_;
    }

    const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    const T* begin() const {
        return data_.begin();
    }

    const T* end() const {
        return data_.begin() + size_;
    }

   private:
    std::array<T, N> data_;
    std::size_t      size_;
};