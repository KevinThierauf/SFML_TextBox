#ifndef SFML_TEXTBOX_REFERENCE_HPP
#define SFML_TEXTBOX_REFERENCE_HPP

#include <memory>
#include <cassert>
#include <iostream>

namespace sftb {
    template<typename T>
    class Reference {
    public:
        using Ref = T**;
        using ConstRef = const T**;
    private:
        std::unique_ptr<T *> pointer;

        inline T *getThis() {
            return static_cast<T *>(this);
        }
    public:
        Reference() : pointer(std::make_unique<T *>(getThis())) {
        }

        Reference(const Reference &other) : pointer(std::make_unique<T *>(getThis())) {
        }

        Reference &operator=(const Reference &other) {
            // no copy needed
            return *this;
        }

        Reference(Reference &&other) noexcept: pointer(std::move(other.pointer)) {
            *pointer = getThis();
        }

        Reference &operator=(Reference &&other) noexcept {
            pointer = std::move(other.pointer);
            *pointer = getThis();
            return *this;
        }

        [[nodiscard]] Ref getReference() {
            return pointer.get();
        }

        [[nodiscard]] ConstRef getReference() const {
            return pointer.get();
        }
    };
}

#endif //SFML_TEXTBOX_REFERENCE_HPP
