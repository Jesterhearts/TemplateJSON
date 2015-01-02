#pragma once
#ifndef __JSON_TEST_POINTERS_HPP__
#define __JSON_TEST_POINTERS_HPP__

struct bare_pointers {
    bare_pointers() : pointer(new int(5)), null(nullptr) {}

    bare_pointers(int* pointer, int* null) : pointer(pointer), null(null) {}

    ~bare_pointers() {
        delete pointer;
    }

    int* pointer;
    int* null;
};

struct smart_pointers {
    smart_pointers() : unique(new int(3)), shared(new int(11)) {}

    smart_pointers(std::unique_ptr<int>&& u, std::shared_ptr<int>&& s)
        : unique(std::move(u)), shared(std::move(s)) {}

    smart_pointers(smart_pointers&& other)
        : unique(std::move(other.unique)), shared(std::move(other.shared)) {}

    std::unique_ptr<int> unique;
    std::shared_ptr<int> shared;

    smart_pointers(const smart_pointers&) = delete;
};

#endif
