#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <numbers>
#include <optional>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;
const int FRAMES_PER_ANIM = 60;
const float RADIUS = 20.f;
const float GRAPH_LEFT = 100.f;
const float GRAPH_BOTTOM = 750.f;
const float GRAPH_WIDTH = 600.f;
const float GRAPH_HEIGHT = 300.f;
const int GRAPH_SAMPLES = 100;

int frameCount = 0;

float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

// global tween function
std::function<float(float, float, float)> tween = [](float a, float b, float t) {
    return (1 - t) * a + t * b;
};

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            const float pi = std::numbers::pi_v<float>;
            switch (key->code) {
                //Linear case
                case sf::Keyboard::Key::Num1:
                    tween = [](float a, float b, float t) {
                        return lerp(a, b, t);
                    };
                    break;
                //Ease-in quad
                case sf::Keyboard::Key::Num2:
                    tween = [](float a, float b, float t) {
                        return lerp(a, b, t * t);
                    };
                    break;
                //Sine in/out
                case sf::Keyboard::Key::Num3:
                    tween = [pi](float a, float b, float t) {
                        return lerp(a, b, (std::sin((t - 0.5f) * pi) + 1) / 2);
                    };
                    break;
                //Smooth
                case sf::Keyboard::Key::Num4:
                    tween = [](float a, float b, float t) {
                        float tmp1 = t * t * t;
                        float tmp2 = (1 - t) * (1 - t) * (1 - t);
                        float mix = (1 - t) * tmp1 + t * (1 - tmp2);
                        return lerp(a, b, mix);
                    };
                    break;
                //Cubic Bezier
                case sf::Keyboard::Key::Num5:
                    tween = [](float a, float b, float t) {
                        const float p1 = 0.f, p2 = -0.5f, p3 = 1.5f, p4 = 1.f;
                        float u = 1 - t;
                        float e = (u * u * u * p1) + (3 * u * u * t * p2) + (3 * u * t * t * p3) + (t * t * t * p4);
                        return lerp(a, b, e);
                    };
                    break;
                //Ease-out cubic
                case sf::Keyboard::Key::Num6:
                    tween = [](float a, float b, float t) {
                        return lerp(a, b, 1 - std::pow(1 - t, 3.f));
                    };
                    break;
                //Ease-out bounce
                case sf::Keyboard::Key::Num7:
                    tween = [](float a, float b, float t) {
                        const float n1 = 7.5625f;
                        const float d1 = 2.75f;
                        float e;
                        if (t < 1 / d1) {
                            e = n1 * t * t;
                        } else if (t < 2 / d1) {
                            t -= 1.5f / d1;
                            e = n1 * t * t + 0.75f;
                        } else if (t < 2.5f / d1) {
                            t -= 2.25f / d1;
                            e = n1 * t * t + 0.9375f;
                        } else {
                            t -= 2.625f / d1;
                            e = n1 * t * t + 0.984375f;
                        }
                        return lerp(a, b, e);
                    };
                    break;
                //Ease-out elastic
                case sf::Keyboard::Key::Num8:
                    tween = [pi](float a, float b, float t) {
                        if (t <= 0) return a;
                        if (t >= 1) return b;
                        const float c4 = (2 * pi) / 3;
                        float e = std::pow(2.f, -10 * t) * std::sin((t * 10 - 0.75f) * c4) + 1;
                        return lerp(a, b, e);
                    };
                    break;
                //Ease-in/out expo
                case sf::Keyboard::Key::Num9:
                    tween = [](float a, float b, float t) {
                        if (t <= 0) return a;
                        if (t >= 1) return b;
                        float e = t < 0.5f ? std::pow(2.f, 20 * t - 10) / 2
                                           : (2 - std::pow(2.f, -20 * t + 10)) / 2;
                        return lerp(a, b, e);
                    };
                    break;
                default:
                    break;
            }
        }
    }
}

void render(sf::RenderWindow& window) {

    window.clear(sf::Color::Black);

    float t = (frameCount % FRAMES_PER_ANIM) / static_cast<float>(FRAMES_PER_ANIM);
    frameCount++;

    //circle
    float x = tween(RADIUS, WINDOW_WIDTH - RADIUS, t);
    float y = WINDOW_HEIGHT / 3.f;

    sf::CircleShape circle(RADIUS);
    circle.setOrigin({RADIUS, RADIUS});
    circle.setPosition({x, y});
    circle.setFillColor(sf::Color::Yellow);
    window.draw(circle);

    const float graphRight = GRAPH_LEFT + GRAPH_WIDTH;
    const float graphTop = GRAPH_BOTTOM - GRAPH_HEIGHT;

    sf::Vertex axes[] = {
        {{GRAPH_LEFT, GRAPH_BOTTOM}}, {{graphRight, GRAPH_BOTTOM}},
        {{GRAPH_LEFT, GRAPH_BOTTOM}}, {{GRAPH_LEFT, graphTop}},
    };
    window.draw(axes, 4, sf::PrimitiveType::Lines);

    //Graph line
    sf::VertexArray curve(sf::PrimitiveType::LineStrip, GRAPH_SAMPLES);
    for (int i = 0; i < GRAPH_SAMPLES; i++) {
        float gx = i / static_cast<float>(GRAPH_SAMPLES - 1);
        curve[i].position = {lerp(GRAPH_LEFT, graphRight, gx), tween(GRAPH_BOTTOM, graphTop, gx)};
        curve[i].color = sf::Color::Yellow;
    }
    window.draw(curve);

    //Current point on the curve
    const float DOT_RADIUS = 5.f;
    sf::CircleShape dot(DOT_RADIUS);
    dot.setOrigin({DOT_RADIUS, DOT_RADIUS});
    dot.setPosition({lerp(GRAPH_LEFT, graphRight, t), tween(GRAPH_BOTTOM, graphTop, t)});
    dot.setFillColor(sf::Color::Yellow);
    window.draw(dot);

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {

        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Tween");
        window.setFramerateLimit(FPS_LIMIT);
 
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;

        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
