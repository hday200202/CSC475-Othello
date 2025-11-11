/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: Utility functions for Othello.
*/

#pragma once
#include <SFML/Graphics.hpp>
#include <string>

sf::Color fromHex(const std::string& hex) {
    /*
        Convert a hex string (eg. #12345678) int to an sf::Color
    */
    std::string cleanHex = hex;
    if (cleanHex[0] == '#')
        cleanHex = cleanHex.substr(1);
    
    unsigned int r = 0, g = 0, b = 0, a = 255;
    
    if (cleanHex.length() == 6) {
        r = std::stoul(cleanHex.substr(0, 2), nullptr, 16);
        g = std::stoul(cleanHex.substr(2, 2), nullptr, 16);
        b = std::stoul(cleanHex.substr(4, 2), nullptr, 16);
    } else if (cleanHex.length() == 8) {
        r = std::stoul(cleanHex.substr(0, 2), nullptr, 16);
        g = std::stoul(cleanHex.substr(2, 2), nullptr, 16);
        b = std::stoul(cleanHex.substr(4, 2), nullptr, 16);
        a = std::stoul(cleanHex.substr(6, 2), nullptr, 16);
    }
    
    return sf::Color(r, g, b, a);
}