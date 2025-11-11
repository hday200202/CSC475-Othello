/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: This class contains the window for the tree displays.
          Init or pass in a SearchTree. Calculates the node
          positions, connects children to their parents, and
          renders the tree. Handles input for moving the camera
          and zooming.
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>
#include <map>
#include <chrono>
#include "SearchTree.hpp"
#include "Util.hpp"
#include "../ext/UILO/assets/EmbeddedFont.hpp"

class TreeDisplay {
public:
    TreeDisplay(){}
    TreeDisplay(SearchTree& tree);

    void createWindow();
    void update();
    void render();
    void drawTree();
    void pollEvents();
    void setTree(SearchTree& tree);
    
    bool isRunning() const { return m_running; }

private:
    sf::RenderWindow m_window;
    sf::VideoMode m_screenRes;
    sf::View m_windowView;

    bool m_running = false;
    const sf::Vector2f m_boxSize = {256, 512};

    SearchTree m_tree;
    
    float m_viewSpeed = 500.0f;
    float m_zoomFactor = 1.0f;
    float m_zoomSpeed = 2.0f;
    
    std::chrono::steady_clock::time_point m_lastFrameTime;
    float m_deltaTime = 0.0f;
    
    bool m_positionsCalculated = false;
    
    std::shared_ptr<SearchNode> m_selectedNode = nullptr;
    
    void drawNode(std::shared_ptr<SearchNode> node, int level, int maxDepth, float nodeWidth, float nodeHeight, float verticalSpacing, sf::Font& font, bool fontLoaded);
    void collectNodesAtLevel(std::shared_ptr<SearchNode> node, int targetLevel, int currentLevel, std::vector<std::shared_ptr<SearchNode>>& nodes);
    void handleInput();
    void calculateNodePositions(std::shared_ptr<SearchNode> node, float startX, float nodeWidth, float horizontalSpacing);
    void storeNodeBounds(std::shared_ptr<SearchNode> node, int level, float nodeWidth, float nodeHeight, float verticalSpacing);
    float calculateSubtreeWidth(std::shared_ptr<SearchNode> node, float nodeWidth, float horizontalSpacing);
    void updateDeltaTime();
    bool isNodeInView(float x, float y, float nodeWidth, float nodeHeight);
    void centerViewOnRoot();
    
    std::map<std::shared_ptr<SearchNode>, float> m_nodePositions;
    std::map<std::shared_ptr<SearchNode>, sf::FloatRect> m_nodeBounds;
};

TreeDisplay::TreeDisplay(SearchTree& tree)
: m_tree(tree) { 
    m_lastFrameTime = std::chrono::steady_clock::now();
    createWindow(); 
}

void TreeDisplay::createWindow() {
    m_screenRes = sf::VideoMode::getDesktopMode();
    m_screenRes.size.x /= 2;
    m_screenRes.size.y /= 2;

    m_windowView.setSize(static_cast<sf::Vector2f>(m_screenRes.size));
    m_windowView.setCenter({m_windowView.getSize().x / 2, m_windowView.getSize().y / 2});

    m_window.create(
        m_screenRes,
        std::string{m_tree.getRoot()->turn == 'b' ? "Black Tree" : "WhiteTree"},
        sf::Style::Titlebar | sf::Style::Close,
        sf::State::Windowed
    );
    m_window.setVerticalSyncEnabled(true);
    m_window.setView(m_windowView);

    m_running = m_window.isOpen();
}

void TreeDisplay::update() {
    if (!m_running) return;

    updateDeltaTime();
    handleInput();
    pollEvents();

    if (m_running) render();
}

void TreeDisplay::render() {
    m_window.clear();
    m_window.setView(m_windowView);
    drawTree();
    m_window.display();
}

void TreeDisplay::drawTree() {
    if (!m_tree.getRoot()) return;
    
    sf::Font font;
    bool fontLoaded = font.openFromMemory(uilo::EMBEDDED_DEJAVUSANS_FONT.data(), uilo::EMBEDDED_DEJAVUSANS_FONT.size());
    
    const float nodeWidth = 120.0f;
    const float nodeHeight = 80.0f;
    const float verticalSpacing = 100.0f;
    
    int maxDepth = m_tree.getMaxDepth();
    
    if (!m_positionsCalculated) {
        m_nodePositions.clear();
        m_nodeBounds.clear();
        float horizontalSpacing = 20.0f;
        calculateNodePositions(m_tree.getRoot(), 0.0f, nodeWidth, horizontalSpacing);
        storeNodeBounds(m_tree.getRoot(), 0, nodeWidth, nodeHeight, verticalSpacing);
        m_positionsCalculated = true;        
        centerViewOnRoot();
    }
    
    drawNode(m_tree.getRoot(), 0, maxDepth, nodeWidth, nodeHeight, verticalSpacing, font, fontLoaded);
    
    if (m_selectedNode && fontLoaded) {
        sf::View originalView = m_window.getView();
        m_window.setView(m_window.getDefaultView());
        
        sf::Text sequenceText(font);
        sequenceText.setString("Sequence: " + m_selectedNode->moveSequence);
        sequenceText.setCharacterSize(32);
        sequenceText.setFillColor(sf::Color::White);
        sequenceText.setPosition({10.f, 10.f});
        
        sf::FloatRect textBounds = sequenceText.getLocalBounds();
        sf::RectangleShape background({textBounds.size.x + 20.f, textBounds.size.y + 20.f});
        background.setPosition({5.f, 5.f});
        background.setFillColor(sf::Color(0, 0, 0, 200));
        
        m_window.draw(background);
        m_window.draw(sequenceText);
        
        m_window.setView(originalView);
    }
}

void TreeDisplay::drawNode(std::shared_ptr<SearchNode> node, int level, int maxDepth, float nodeWidth, float nodeHeight, float verticalSpacing, sf::Font& font, bool fontLoaded) {
    if (!node) return;
    
    float x = m_nodePositions[node];
    float y = 50.0f + level * (nodeHeight + verticalSpacing);
    
    float nodeCenterX = x + nodeWidth / 2.0f;
    float nodeBottomY = y + nodeHeight;
    
    for (auto& child : node->children) {
        float childX = m_nodePositions[child];
        float childY = 50.0f + (level + 1) * (nodeHeight + verticalSpacing);
        float childCenterX = childX + nodeWidth / 2.0f;
        
        float midY = (nodeBottomY + childY) / 2.0f;
        
        sf::Vertex line[6];
        line[0].position = sf::Vector2f(childCenterX, childY);
        line[0].color = sf::Color::White;
        line[1].position = sf::Vector2f(childCenterX, midY);
        line[1].color = sf::Color::White;
        
        line[2].position = sf::Vector2f(childCenterX, midY);
        line[2].color = sf::Color::White;
        line[3].position = sf::Vector2f(nodeCenterX, midY);
        line[3].color = sf::Color::White;
        
        line[4].position = sf::Vector2f(nodeCenterX, midY);
        line[4].color = sf::Color::White;
        line[5].position = sf::Vector2f(nodeCenterX, nodeBottomY);
        line[5].color = sf::Color::White;
        
        m_window.draw(line, 6, sf::PrimitiveType::Lines);
    }
    
    if (!isNodeInView(x, y, nodeWidth, nodeHeight)) {
        for (auto& child : node->children)
            drawNode(child, level + 1, maxDepth, nodeWidth, nodeHeight, verticalSpacing, font, fontLoaded);
        return;
    }
    
    sf::RectangleShape nodeRect(sf::Vector2f(nodeWidth, nodeHeight));
    nodeRect.setPosition({x, y});
    
    sf::Color nodeColor;
    
    // Green = maximizing (White), Red = minimizing (Black)
    if (node->maximizing)
        nodeColor = fromHex("#2d5c2dff");
    else
        nodeColor = fromHex("#7d4040ff");
    
    if (m_selectedNode == node)
        nodeColor = fromHex("#ffaa00ff");
    
    nodeRect.setFillColor(nodeColor);
    nodeRect.setOutlineColor(sf::Color::White);
    nodeRect.setOutlineThickness(2.0f);
    
    m_window.draw(nodeRect);
    
    if (fontLoaded) {
        sf::Text nodeText(font);
        std::string text = "Move: (" + std::to_string(node->row) + "," + std::to_string(node->col) + ")\n";
        text += "Heuristic: " + std::to_string(node->heuristic) + "\n";
        text += "Depth: " + std::to_string(node->depth) + "\n";
        text += "Turn: " + std::string(1, node->turn) + "\n";
        text += "Score: " + std::to_string(node->whiteScore) + "-" + std::to_string(node->blackScore);
        
        nodeText.setString(text);
        nodeText.setCharacterSize(10);
        nodeText.setFillColor(sf::Color::White);
        nodeText.setPosition({x + 5, y + 5});
        
        m_window.draw(nodeText);
    }
    
    for (auto& child : node->children)
        drawNode(child, level + 1, maxDepth, nodeWidth, nodeHeight, verticalSpacing, font, fontLoaded);
}

void TreeDisplay::collectNodesAtLevel(std::shared_ptr<SearchNode> node, int targetLevel, int currentLevel, std::vector<std::shared_ptr<SearchNode>>& nodes) {
    if (!node) return;
    
    if (currentLevel == targetLevel) {
        nodes.push_back(node);
        return;
    }
    
    for (auto& child : node->children)
        collectNodesAtLevel(child, targetLevel, currentLevel + 1, nodes);
}

void TreeDisplay::handleInput() {
    sf::Vector2f movement(0.f, 0.f);
    
    float scaledViewSpeed = m_viewSpeed * m_zoomFactor * m_deltaTime;
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        movement.x -= scaledViewSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        movement.x += scaledViewSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        movement.y -= scaledViewSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        movement.y += scaledViewSpeed;
    
    if (movement.x != 0.f || movement.y != 0.f)
        m_windowView.move(movement);
    
    bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || 
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
    
    if (ctrlPressed) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal)) {
            m_zoomFactor -= m_zoomSpeed * m_deltaTime;
            if (m_zoomFactor < 0.1f) m_zoomFactor = 0.1f;
            sf::Vector2f size = static_cast<sf::Vector2f>(m_screenRes.size);
            m_windowView.setSize(size * m_zoomFactor);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Hyphen)) {
            m_zoomFactor += m_zoomSpeed * m_deltaTime;
            if (m_zoomFactor > 5.0f) m_zoomFactor = 5.0f;
            sf::Vector2f size = static_cast<sf::Vector2f>(m_screenRes.size);
            m_windowView.setSize(size * m_zoomFactor);
        }
    }
}

void TreeDisplay::pollEvents() {
    while (const auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            m_running = false;
        }
        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
                sf::Vector2f worldPos = m_window.mapPixelToCoords(mousePos);
                
                for (const auto& [node, bounds] : m_nodeBounds) {
                    if (bounds.contains(worldPos)) {
                        m_selectedNode = node;
                        break;
                    }
                }
            }
        }
    }
}

float TreeDisplay::calculateSubtreeWidth(std::shared_ptr<SearchNode> node, float nodeWidth, float horizontalSpacing) {
    if (!node || node->children.empty()) {
        return nodeWidth;
    }
    
    float totalChildrenWidth = 0.0f;
    for (size_t i = 0; i < node->children.size(); ++i) {
        if (i > 0) totalChildrenWidth += horizontalSpacing;
        totalChildrenWidth += calculateSubtreeWidth(node->children[i], nodeWidth, horizontalSpacing);
    }
    
    return std::max(nodeWidth, totalChildrenWidth);
}

void TreeDisplay::calculateNodePositions(std::shared_ptr<SearchNode> node, float startX, float nodeWidth, float horizontalSpacing) {
    if (!node) return;
    
    float subtreeWidth = calculateSubtreeWidth(node, nodeWidth, horizontalSpacing);    
    m_nodePositions[node] = startX + (subtreeWidth - nodeWidth) / 2.0f;
    
    if (!node->children.empty()) {
        float childStartX = startX;
        
        for (auto& child : node->children) {
            float childSubtreeWidth = calculateSubtreeWidth(child, nodeWidth, horizontalSpacing);
            calculateNodePositions(child, childStartX, nodeWidth, horizontalSpacing);
            childStartX += childSubtreeWidth + horizontalSpacing;
        }
    }
}

void TreeDisplay::updateDeltaTime() {
    auto currentTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastFrameTime);
    m_deltaTime = duration.count() / 1000000.0f;
    m_lastFrameTime = currentTime;
    
    if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;
}

bool TreeDisplay::isNodeInView(float x, float y, float nodeWidth, float nodeHeight) {
    sf::Vector2f viewCenter = m_windowView.getCenter();
    sf::Vector2f viewSize = m_windowView.getSize();
    
    float viewLeft = viewCenter.x - viewSize.x / 2.0f;
    float viewRight = viewCenter.x + viewSize.x / 2.0f;
    float viewTop = viewCenter.y - viewSize.y / 2.0f;
    float viewBottom = viewCenter.y + viewSize.y / 2.0f;
    
    return (x < viewRight && x + nodeWidth > viewLeft &&
            y < viewBottom && y + nodeHeight > viewTop);
}

void TreeDisplay::setTree(SearchTree& tree) {
    m_tree = tree;
    m_positionsCalculated = false;
    m_selectedNode = nullptr;
    
    if (m_tree.getRoot()) {
        std::string title = m_tree.getRoot()->turn == 'b' ? "Black Tree" : "White Tree";
        m_window.setTitle(title);
    }
    
    if (m_running) render();
}

void TreeDisplay::centerViewOnRoot() {
    if (!m_tree.getRoot() || m_nodePositions.empty()) return;
    
    float rootX = m_nodePositions[m_tree.getRoot()];
    const float nodeWidth = 120.0f;
    const float nodeHeight = 80.0f;
    const float rootY = 50.0f;
    
    float rootCenterX = rootX + nodeWidth / 2.0f;
    float rootCenterY = rootY + nodeHeight / 2.0f;
    
    sf::Vector2f viewSize = m_windowView.getSize();
    
    float viewCenterX = rootCenterX;
    float viewCenterY = rootCenterY + viewSize.y / 2.0f - 100.0f;
    
    m_windowView.setCenter({viewCenterX, viewCenterY});
    m_window.setView(m_windowView);
}

void TreeDisplay::storeNodeBounds(std::shared_ptr<SearchNode> node, int level, float nodeWidth, float nodeHeight, float verticalSpacing) {
    
    float x = m_nodePositions[node];
    float y = 50.0f + level * (nodeHeight + verticalSpacing);
    
    m_nodeBounds[node] = sf::FloatRect({x, y}, {nodeWidth, nodeHeight});
    
    for (auto& child : node->children) {
        storeNodeBounds(child, level + 1, nodeWidth, nodeHeight, verticalSpacing);
    }
}

