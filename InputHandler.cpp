#include <SFML/Window/Clipboard.hpp>
#include "Caret.hpp"
#include "InputHandler.hpp"
#include "TextBox.hpp"

namespace sftb {
    namespace {
        void copySelectedOrLine(Caret &caret) {
            if (!caret.hasSelection()) {
                caret.setPosition({caret.getPosition().line, 0});
                caret.setSelectionEndPos({caret.getPosition().line + 1, 0});
            }

            sf::Clipboard::setString(caret.getSelectedText());
        }

        void removeText(Caret &caret, bool direction) {
            auto caretPosition = caret.getPosition();
            auto removeToPosition = caret.getTextBox().getRelativeCharacters(caretPosition, direction ? -1 : 1);
            caret.getTextBox().removeText(removeToPosition, caretPosition);
        }
    }

    std::unique_ptr<InputHandler> InputHandler::standard() {
        class StandardInputHandler : public InputHandler {
        public:
            void handle(sf::Keyboard::Key key, bool pressed, bool control, bool shift, bool alt) override {
                if (!pressed) return;

                switch (key) {
                    case sf::Keyboard::A:
                        if (control) {
                            Caret &caret = getTextBox().getPrimaryCaret();

                            caret.setPosition({0, 0});
                            caret.setSelectionEndPos(getTextBox().getEndPos());
                        }
                        break;
                    case sf::Keyboard::C:
                        if (control) {
                            // copy selection, or line
                            copySelectedOrLine(getTextBox().getPrimaryCaret());
                        }
                        break;
                    case sf::Keyboard::D:
                        if (control) {
                            getTextBox().removeLine(getTextBox().getPrimaryCaret().getPosition().line);
                        }
                        break;
                    case sf::Keyboard::F:
                        if (control) {
                            // todo - find
                        }
                        break;
                    case sf::Keyboard::R:
                        if (control) {
                            // todo - replace
                        }
                        break;
                    case sf::Keyboard::V:
                        if (control) {
                            getTextBox().getPrimaryCaret().insert(sf::Clipboard::getString());
                        }
                        break;
                    case sf::Keyboard::X: {
                        if (control) {
                            // cut selection, or line
                            Caret &caret = getTextBox().getPrimaryCaret();
                            copySelectedOrLine(caret);
                            caret.removeSelectedText();
                        }
                    }
                        break;
                    case sf::Keyboard::Y:
                        if (control) {
                            // todo - redo
                        }
                        break;
                    case sf::Keyboard::Z:
                        if (control) {
                            // todo - undo
                        }
                        break;
                    case sf::Keyboard::Escape:
                        getTextBox().getPrimaryCaret().removeSelection();
                        break;
                    case sf::Keyboard::LBracket:
                        // todo - smart brackets
                        break;
                    case sf::Keyboard::RBracket:
                        // todo - smart brackets
                        break;
                    case sf::Keyboard::Quote:
                        // todo - smart brackets
                        break;
                    case sf::Keyboard::Backspace:
                        // remove character to left
                        removeText(getTextBox().getPrimaryCaret(), true);
                        // todo - smart brackets (ignore if highlighted)
                        break;
                    case sf::Keyboard::PageUp:
                        // todo
                        break;
                    case sf::Keyboard::PageDown:
                        // todo
                        break;
                    case sf::Keyboard::End: {
                        // move caret to end of line
                        Caret &caret = getTextBox().getPrimaryCaret();
                        auto caretLine = caret.getPosition().line;
                        caret.setPosition({caretLine, getTextBox().getLineLength(caretLine)});
                    }
                        break;
                    case sf::Keyboard::Home: {
                        // move caret to start of line
                        Caret &caret = getTextBox().getPrimaryCaret();
                        caret.setPosition({caret.getPosition().line, 0});
                    }
                        break;
                    case sf::Keyboard::Delete:
                        // remove character to right
                        removeText(getTextBox().getPrimaryCaret(), false);
                        break;
                    case sf::Keyboard::Left: {
                        Caret &caret = getTextBox().getPrimaryCaret();
                        if (shift)
                            caret.setSelectionEndPos(
                                    getTextBox().getRelativeCharacters(caret.getSelectionEndPos(), -1));
                        else caret.setPosition(getTextBox().getRelativeCharacters(caret.getPosition(), -1));
                    }
                        break;
                    case sf::Keyboard::Right: {
                        Caret &caret = getTextBox().getPrimaryCaret();
                        if (shift)
                            caret.setSelectionEndPos(getTextBox().getRelativeCharacters(caret.getSelectionEndPos(), 1));
                        else caret.setPosition(getTextBox().getRelativeCharacters(caret.getPosition(), 1));
                    }
                        break;
                    case sf::Keyboard::Up: {
                        Caret &caret = getTextBox().getPrimaryCaret();
                        if (shift)
                            caret.setSelectionEndPos(
                                    getTextBox().getVisibleRelativeLine(caret.getSelectionEndPos(), -1));
                        else caret.setPosition(getTextBox().getVisibleRelativeLine(caret.getPosition(), -1));
                    }
                        break;
                    case sf::Keyboard::Down: {
                        Caret &caret = getTextBox().getPrimaryCaret();
                        if (shift)
                            caret.setSelectionEndPos(
                                    getTextBox().getVisibleRelativeLine(caret.getSelectionEndPos(), 1));
                        else caret.setPosition(getTextBox().getVisibleRelativeLine(caret.getPosition(), 1));
                    }
                    default:
                        break;
                }
            }
        };
        return std::make_unique<StandardInputHandler>();
    }
}