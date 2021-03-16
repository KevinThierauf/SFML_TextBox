#include <SFML/Graphics.hpp>
#include <TextBox.hpp>
#include <iostream>

const char *fontFile = "font.ttf";
constexpr unsigned WIDTH = 800;
constexpr unsigned HEIGHT = 600;

int main() {
    // This demo is a modified version of the code from sfml's "Drawing 2D stuff" tutorial
    // https://www.sfml-dev.org/tutorials/2.5/

    // load font
    sf::Font font;
    if (!font.loadFromFile(fontFile)) {
        std::cerr << "A font must be provided in order for this demo to work!" << std::endl;
        std::cerr << "Make sure a monospaced font file named " + std::string(fontFile) + " exists next to this executable."
                  << std::endl;
        std::cerr << "Press Enter to exit." << std::endl;
        std::cin.get();
        return -1;
    }
    std::cout << "Font " << std::string(fontFile) << " loaded successfully!" << std::endl;

    // create textbox using font
    sftb::TextBox box{font, {WIDTH, HEIGHT}};
    const unsigned backgroundBrightness = 60;
    box.setBackgroundColor(sf::Color(backgroundBrightness, backgroundBrightness, backgroundBrightness));
    box.insertText({0, 0}, "Hello, World!");
    box.insertLine(1, "0123456789");

    // create window
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML_TextBox demo");

    // standard event loop
    while (window.isOpen()) {
        sf::Event event{};

        // poll events (user enters input, window is resized, etc.)
        while (window.pollEvent(event)) {
            // handle event
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::Resized) {
                // update draw area to reflect updated size
                window.setView(sf::View(sf::FloatRect(0.0f, 0.0f, event.size.width, event.size.height)));
                box.setSize(sf::Vector2f(event.size.width, event.size.height));
            } else box.handleEvent(event); // send user input to text box
        }

        // event queue is now empty

        // if content has changed, clear the screen and redraw
        if (box.isRedrawRequired()) {
            window.clear(sf::Color::Magenta);
            // draw text box
            window.draw(box);
            // end
            window.display();
        }
    }
}