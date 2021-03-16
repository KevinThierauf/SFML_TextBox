#ifndef SFML_TEXTBOX_INPUTHANDLER_HPP
#define SFML_TEXTBOX_INPUTHANDLER_HPP

#include <SFML/Window/Keyboard.hpp>
#include <memory>

namespace sftb {
    class TextBox;

    using Char = sf::Uint32;

    class InputHandler {
        friend class TextBox;
    private:
        TextBox *textBox = nullptr;
    protected:
        static bool isNumeric(Char code) {
            return '0' <= code && code <= '9';
        }

        static bool isAlpha(Char code) {
            return ('A' <= code && code <= 'Z') || ('a' <= code && code <= 'z');
        }

        static bool isWhitespace(Char code) {
            return code == ' ' || code == '\t' || code == '\n' || code == '\r';
        }

        static bool isSymbol(Char code) {
            return ('!' <= code && code <= '/') || (':' <= code && code <= '@') || ('[' <= code && code <= '`') ||
                   ('{' <= code && code <= '~');
        }

    public:
        InputHandler() = default;

        InputHandler(const InputHandler &) : textBox(nullptr) {}

        InputHandler &operator=(const InputHandler &other) {
            if (this != &other)
                textBox = nullptr;
            return *this;
        }

        // managed exclusively by smart pointers -- smart pointers may be moved, but the InputHandler objects themselves
        // should not be
        InputHandler(InputHandler &&) = delete;
        InputHandler &operator=(InputHandler &&) = delete;

        static std::shared_ptr<InputHandler> basic() {
            return std::make_shared<InputHandler>();
        }

        static std::shared_ptr<InputHandler> standard();

        virtual ~InputHandler() = default;

        [[nodiscard]] TextBox &getTextBox() {
            return *textBox;
        }

        [[nodiscard]] const TextBox &getTextBox() const {
            return *textBox;
        }

        [[nodiscard]] virtual bool isTextInput(Char code) const {
            return isAlpha(code) || isNumeric(code) || isWhitespace(code) || isSymbol(code);
        }

        virtual void handle(sf::Keyboard::Key key, bool pressed, bool control, bool shift, bool alt) {
        }
    };
}

#endif //SFML_TEXTBOX_INPUTHANDLER_HPP
