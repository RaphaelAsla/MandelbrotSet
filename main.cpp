#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <thread>

static constexpr std::size_t window_size = 600;

int getValue(std::complex<long double> c, int iterations) {
    std::complex<long double> z{0.f, 0.f};
    for (int i = 0; i < iterations; i++) {
        z = z * z + c;
        if (std::norm(z) > 50) {
            return i;
        }
    }
    return 0;
}

sf::Color toHSV(int hue, float sat, float val) {
    if (sat == 0)
        return {0, 0, 0};

    hue %= 360;
    sat /= 100;
    val /= 100;

    uint8_t h = hue / 60;
    float f = static_cast<float>(hue) / 60 - h;
    float p = val * (1.f - sat);
    float q = val * (1.f - sat * f);
    float t = val * (1.f - sat * (1 - f));

    switch (h) {
        case 1:
            return {static_cast<uint8_t>(q * 255), static_cast<uint8_t>(val * 255), static_cast<uint8_t>(p * 255)};
        case 2:
            return {static_cast<uint8_t>(p * 255), static_cast<uint8_t>(val * 255), static_cast<uint8_t>(t * 255)};
        case 3:
            return {static_cast<uint8_t>(p * 255), static_cast<uint8_t>(q * 255), static_cast<uint8_t>(val * 255)};
        case 4:
            return {static_cast<uint8_t>(t * 255), static_cast<uint8_t>(p * 255), static_cast<uint8_t>(val * 255)};
        case 5:
            return {static_cast<uint8_t>(val * 255), static_cast<uint8_t>(p * 255), static_cast<uint8_t>(q * 255)};
        default:
            return {static_cast<uint8_t>(val * 255), static_cast<uint8_t>(t * 255), static_cast<uint8_t>(p * 255)};
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(window_size, window_size), "Fractal", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Clock clock;

    sf::Image output_image;
    output_image.create(window_size, window_size);

    sf::Texture output_texture;
    output_texture.loadFromImage(output_image);

    sf::Sprite output_sprite;
    output_sprite.setTexture(output_texture);

    sf::Font font;

    if (!font.loadFromFile("../font/font.ttf")) {
        std::cerr << "Font not found\n";
    }

    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::White);
    text.setOutlineThickness(2);
    text.setOutlineColor(sf::Color::Black);

    int iterations = 250;
    long double zoom = 1.f;
    std::complex<long double> xyoff;
    std::array<sf::Color, window_size * window_size> pixels;
    auto threads = std::vector<std::thread>(std::thread::hardware_concurrency());

    const size_t render_rows = threads.size();
    const size_t render_rows_size = window_size / render_rows;

    const auto make_pixels = [&pixels, &zoom, &xyoff, &iterations](const size_t start, const size_t end) {
        for (size_t y = start; y < end; y++) {
            for (size_t x = 0; x < window_size; x++) {
                long double py = std::lerp(-2.f, 2.f, static_cast<long double>(y) / window_size);
                long double px = std::lerp(-2.f, 2.f, static_cast<long double>(x) / window_size);
                int value = getValue(zoom * std::complex<long double>(px, py) + xyoff, iterations);
                pixels[x + window_size * y] = toHSV(value, (value == 0) ? 0 : 75, 100);
            }
        }
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::S) {
                    xyoff = {xyoff.real(), xyoff.imag() + zoom / 27};
                } else if (event.key.code == sf::Keyboard::W) {
                    xyoff = {xyoff.real(), xyoff.imag() - zoom / 27};
                } else if (event.key.code == sf::Keyboard::D) {
                    xyoff = {xyoff.real() + zoom / 27, xyoff.imag()};
                } else if (event.key.code == sf::Keyboard::A) {
                    xyoff = {xyoff.real() - zoom / 27, xyoff.imag()};
                } else if (event.key.code == sf::Keyboard::J) {
                    zoom *= 1.5f;
                } else if (event.key.code == sf::Keyboard::K) {
                    zoom /= 1.5f;
                } else if (event.key.code == sf::Keyboard::Up) {
                    iterations += 25;
                } else if (event.key.code == sf::Keyboard::Down) {
                    iterations -= 25;
                } else if (event.key.code == sf::Keyboard::P) {
                    output_image.saveToFile("../images/fractal" + std::to_string(time(NULL)) + ".jpg");
                } else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                long double ratio_x = static_cast<long double>(event.mouseButton.x) / window_size;
                long double ratio_y = static_cast<long double>(event.mouseButton.y) / window_size;

                if (event.mouseButton.button == sf::Mouse::Left) {
                    xyoff += std::complex<long double>(std::lerp(-2.f, 2.f, ratio_x), std::lerp(-2.f, 2.f, ratio_y)) * zoom;
                    zoom /= 1.5f;
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    xyoff -= std::complex<long double>(std::lerp(-2.f, 2.f, ratio_x), std::lerp(-2.f, 2.f, ratio_y)) * zoom;
                    zoom *= 1.5f;
                }
            }
        }

        window.clear();

        for (size_t i = 0; i < render_rows; i++) {
            threads[i] = std::thread(make_pixels, i * render_rows_size, (i + 1) * render_rows_size);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        output_image.create(window_size, window_size, reinterpret_cast<uint8_t*>(pixels.data()));

        output_texture.update(output_image);
        output_sprite.setTexture(output_texture);
        window.draw(output_sprite);
        window.draw(text);
        window.display();

        auto text_builder = std::ostringstream();
        text_builder << std::setw(4) << static_cast<int>(1 / clock.restart().asSeconds()) << " fps\n";
        text_builder << std::setw(4) << iterations << " iters\n";
        text_builder << std::setw(4) << zoom << " zoom\n";
        text.setString(text_builder.str());
    }
}
