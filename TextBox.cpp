#include "TextBox.hpp"

namespace sftb {
    void TextBox::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        setRedrawRequired(false);
    }
}