#include "Caret.hpp"
#include "TextBox.hpp"
#include <algorithm>

namespace sftb {
    Caret::Caret(TextBox &box, Pos position) :
            reference(box.getReference()), pos(reference->getCharPos(position)), selectionEndPos(nullptr),
            style(reference->getCaretStyle()) {
    }

    Pos Caret::getClosestPos(const Pos &position) const {
        auto line = std::min(reference->getNumberLines(), position.line);
        return {line, std::min(reference->getLineLength(line), position.position)};
    }

    Pos Caret::getPosition() const {
        return reference->getPositionOfChar(pos);
    }

    void Caret::setPosition(const Pos &position) {
        Pos previous = getPosition();
        pos = reference->getCharPos(position);
        removeSelection();
        reference->setRedrawRequired();
        style->notifyPositionChange(*this, previous);
    }

    Pos Caret::getSelectionEndPos() const {
        return selectionEndPos == nullptr ? getPosition() : reference->getPositionOfChar(selectionEndPos);
    }

    void Caret::setSelectionEndPos(const Pos &position) {
        selectionEndPos = reference->getCharPos(position);
        if(selectedTextHighlight.isRemoved())
            selectedTextHighlight.setHighlight(getTextBox().highlight(getPosition(), position, style->getSelectedTextHighlighter(*this)));
        else selectedTextHighlight->setEnd(position);
    }

    void Caret::removeSelectedText() {
        if (hasSelection()) {
            reference->removeText(getPosition(), getSelectionEndPos());
            removeSelection();
        }
    }

    sf::String Caret::getSelectedText() const {
        return hasSelection() ? reference->getTextFrom(getPosition(), getSelectionEndPos()) : "";
    }

    void Caret::insert(const sf::String &string) {
        removeSelectedText();
        setPosition(reference->insertText(getPosition(), string));
    }
}