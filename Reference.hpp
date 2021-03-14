#ifndef SFML_TEXTBOX_REFERENCE_HPP
#define SFML_TEXTBOX_REFERENCE_HPP

#include <memory>
#include <cassert>

namespace sftb {
    template<typename T>
    class Reference {
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

        [[nodiscard]] T *getReference() {
            return *pointer;
        }

        [[nodiscard]] const T *getReference() const {
            return *pointer;
        }
    };
}

#endif //SFML_TEXTBOX_REFERENCE_HPP
