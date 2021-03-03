#include <SFML/Graphics.hpp>
#include <TextBox.hpp>
#include <iostream>

const char *fontFile = "font.ttf";

int main() {
    // This demo is a modified version of the code from sfml's "Drawing 2D stuff" tutorial
    // https://www.sfml-dev.org/tutorials/2.5/

    // load font
    sf::Font font;

    if (!font.loadFromFile(fontFile)) {
        std::cerr << "A font must be provided in order for this demo to work!" << std::endl;
        std::cerr << "Make sure a font file named " + std::string(fontFile) + " exists next to this executable." << std::endl;
        std::cerr << "Press Enter to exit." << std::endl;
        std::cin.get();
        return -1;
    }
    // create textbox using font
    sftb::TextBox box{font};

    sf::RenderWindow window(sf::VideoMode(), "SFML_TextBox demo");

    while (window.isOpen()) {
        sf::Event event{};

        // wait for something to happen (user enters input, window is resized, etc.)
        window.waitEvent(event);
        do {
            // handle event
            if (event.type == sf::Event::Closed)
                window.close();
            // there may be more events later in the queue -- poll for additional activity
        } while (window.pollEvent(event));

        // event queue is now empty

        // if content has changed, clear the screen and redraw
        if (box.isRedrawRequired()) {
            window.clear(sf::Color::Black);
            // draw text box
            window.draw(box);
            // end
            window.display();
        }
    }
}