/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: Othello class manages the game and UI
*/

#pragma once

#include "../ext/UILO/UILO.hpp"
#include "../ext/UILO/assets/EmbeddedFont.hpp"

#include "Board.hpp"
#include "OthelloBot.hpp"
#include "Util.hpp"
#include "TreeDisplay.hpp"

#include <thread>
#include <atomic>
#include <chrono>
#include <future>

using namespace uilo;

class Othello {
public:
    Othello();
    ~Othello();

    bool initUI();

    void update();
    void render();

    void updateGame();
    void renderGrid();
    void startBotThinking();
    
    std::pair<int, int> mouseToGridPos(sf::Vector2i mousePos);

    bool isRunning() const { return m_running; }

private:
    sf::RenderWindow m_window;
    sf::VideoMode m_screenRes;
    sf::View m_windowView;
    std::unique_ptr<UILO> m_ui;

    Row* m_topBar = nullptr;
    Row* m_mainContentRow = nullptr;
    Button* m_resetButton = nullptr;
    Button* m_pauseButton = nullptr;
    Button* m_moveButton = nullptr;
    Button* m_enableBlackButton = nullptr;
    Button* m_enableWhiteButton = nullptr;
    ScrollableColumn* m_blackColumn = nullptr;
    ScrollableColumn* m_whiteColumn = nullptr;
    Slider* m_blackDepthSlider = nullptr;
    Slider* m_whiteDepthSlider = nullptr;
    Button* m_blackAlphaBetaToggle = nullptr;
    Button* m_whiteAlphaBetaToggle = nullptr;
    Button* m_enableBlackTree = nullptr;
    Button* m_enableWhiteTree = nullptr;
    Button* m_printSequenceButton = nullptr;

    Board::State m_board;
    OthelloBot m_blackBot;
    OthelloBot m_whiteBot;

    bool m_running = false;
    bool m_blackEnabled = false;
    bool m_whiteEnabled = false;
    bool m_paused = false;
    
    float m_gridX = 0.f;
    float m_gridY = 0.f;
    float m_gridSize = 0.f;
    float m_cellSize = 0.f;
    
    bool m_mouseWasPressed = false;
    
    std::atomic<bool> m_botThinking = false;
    std::future<std::pair<int, int>> m_botMoveResult;
    std::chrono::steady_clock::time_point m_botStartTime;
    std::chrono::milliseconds m_botMoveDelay{500};
    bool m_waitingForTimer = false;
    std::pair<int, int> m_pendingMove = {-1, -1};

    int m_blackDepth = 4;
    int m_whiteDepth = 4;

    bool m_fullscreen = false;

    TreeDisplay* m_blackTree = nullptr;
    TreeDisplay* m_whiteTree = nullptr;
    
    std::string m_lastMoveSequence;
    std::vector<std::string> m_moveHistory;
};

struct Theme {
    sf::Color bgColor               = fromHex("#5e3333ff");
    sf::Color middleColor           = fromHex("#2a2a2aff");
    sf::Color fgColor               = fromHex("#505050ff");
    sf::Color buttonColor           = fromHex("#7d4040ff");
    sf::Color textColor             = fromHex("#d8d8d8ff");
} uiTheme;

Othello::Othello() { m_running = initUI(); }

Othello::~Othello() {}

bool Othello::initUI() {
    /*
        Initialize the UI window, ui elements, and ui.
    */
    m_screenRes = sf::VideoMode::getDesktopMode();

    if (!m_fullscreen) {
        m_screenRes.size.x /= 2;
        m_screenRes.size.y /= 2;
    }

    m_windowView.setSize(static_cast<sf::Vector2f>(m_screenRes.size));

    m_window.create(
        m_screenRes,
        "Othello",
        sf::Style::Titlebar | sf::Style::Close,
        m_fullscreen ? sf::State::Fullscreen : sf::State::Windowed
    );

    m_window.setVerticalSyncEnabled(true);
    m_window.setPosition({
        (sf::VideoMode::getDesktopMode().size.x / 2) - (m_window.getSize().x / 2),
        (sf::VideoMode::getDesktopMode().size.y / 2) - (m_window.getSize().y / 2)
    });

    m_ui = std::make_unique<UILO>(m_window, m_windowView);

    m_enableBlackButton = button(
        Modifier()
            .setfixedHeight(48.f)
            .setfixedWidth(96.f)
            .align(Align::LEFT | Align::CENTER_Y)
            .setColor(sf::Color::Black)
            .onLClick([&](){ 
                m_blackColumn->m_modifier.setVisible(!m_blackColumn->m_modifier.isVisible()); 
                m_blackEnabled = !m_blackEnabled;
                m_enableBlackButton->m_modifier.setColor(m_blackEnabled ? uiTheme.buttonColor : sf::Color::Black);
                // std::cout << "Black AI Enabled.\n";
            }),
        ButtonStyle::Pill,
        "Black",
        "",
        uiTheme.textColor,
        "enable_black_button"
    );

    m_enableWhiteButton = button(
        Modifier()
            .setfixedHeight(48.f)
            .setfixedWidth(96.f)
            .align(Align::RIGHT | Align::CENTER_Y)
            .setColor(sf::Color::Black)
            .onLClick([&](){ 
                m_whiteColumn->m_modifier.setVisible(!m_whiteColumn->m_modifier.isVisible()); 
                m_whiteEnabled = !m_whiteEnabled;
                m_enableWhiteButton->m_modifier.setColor(m_whiteEnabled ? uiTheme.buttonColor : sf::Color::Black);
                // std::cout << "White AI Enabled.\n";
            }),
        ButtonStyle::Pill,
        "White",
        "",
        uiTheme.textColor,
        "enable_white_button"
    );

    m_resetButton = button(
        Modifier()
            .setfixedHeight(48.f)
            .setfixedWidth(96.f)
            .align(Align::CENTER_X | Align::CENTER_Y)
            .setColor(uiTheme.buttonColor)
            .onLClick([&](){ 
                m_board.clear(); 
                m_moveHistory.clear();
            }),
        ButtonStyle::Pill,
        "Reset",
        "",
        uiTheme.textColor,
        "reset_button"
    );

    m_pauseButton = button(
        Modifier()
            .setfixedHeight(48.f)
            .setfixedWidth(96.f)
            .align(Align::CENTER_X | Align::CENTER_Y)
            .setColor(uiTheme.buttonColor)
            .onLClick([&](){ 
                m_paused = !m_paused; 
                m_pauseButton->setText(m_paused ? "Resume" : "Pause");
            }),
        ButtonStyle::Pill,
        "Pause",
        "",
        uiTheme.textColor,
        "pause_button"
    );

    m_moveButton = button(
        Modifier()
            .setfixedHeight(48.f)
            .setfixedWidth(96.f)
            .align(Align::CENTER_X | Align::CENTER_Y)
            .setColor(uiTheme.buttonColor)
            .onLClick([&](){ 
                if (m_paused && !m_botThinking && !m_waitingForTimer) {
                    if ((m_board.turn == 'b' && m_blackEnabled) || (m_board.turn == 'w' && m_whiteEnabled)) {
                        Board::State tempBoard = m_board;
                        auto move = (m_board.turn == 'b') ? m_blackBot.getBestMove(tempBoard) : m_whiteBot.getBestMove(tempBoard);
                        
                        auto& tree = (m_board.turn == 'b') ? m_blackBot.getSearchTree() : m_whiteBot.getSearchTree();
                        if (tree.getRoot()) {
                            for (auto& child : tree.getRoot()->children) {
                                if (child->row == move.first && child->col == move.second) {
                                    m_lastMoveSequence = child->moveSequence;
                                    break;
                                }
                            }
                        }
                        
                        std::cout << (m_board.turn == 'b' ? "Black: " : "White: ");
                        std::cout << std::to_string(move.first) + " " + std::to_string(move.second) + "\n";
                        char turn = m_board.turn;
                        m_board.place(move.first, move.second);
                        m_moveHistory.push_back(std::string(1, turn == 'b' ? 'B' : 'W') + ": " + std::to_string(move.first) + ":" + std::to_string(move.second));
                        
                        if (m_blackTree && m_blackTree->isRunning())
                            m_blackTree->setTree(const_cast<SearchTree&>(m_blackBot.getSearchTree()));
                        if (m_whiteTree && m_whiteTree->isRunning())
                            m_whiteTree->setTree(const_cast<SearchTree&>(m_whiteBot.getSearchTree()));
                    }
                }
            }),
        ButtonStyle::Pill,
        "Move",
        "",
        uiTheme.textColor,
        "move_button"
    );

    m_printSequenceButton = button(
        Modifier()
            .setfixedHeight(48.f)
            .setfixedWidth(96.f)
            .align(Align::CENTER_X | Align::CENTER_Y)
            .setColor(uiTheme.buttonColor)
            .onLClick([&](){ 
                std::cout << "\n=== Move History ===\n\n";
                std::cout << "Black: " << (m_blackEnabled ? "AI" : "Player") << "\n";
                std::cout << "White: " << (m_whiteEnabled ? "AI" : "Player") << "\n\n";
                for (const auto& move : m_moveHistory) {
                    std::cout << "\t" + move + "\n";
                }
                std::cout << "\nBlack: " << m_board.black << "\n";
                std::cout << "White: " << m_board.white << "\n";
                std::cout << "Winner: " << (m_board.black > m_board.white ? "Black" : m_board.black == m_board.white ? "Tie" : "White") << "\n";
                std::cout << "\n====================\n";
            }),
        ButtonStyle::Pill,
        "Print",
        "",
        uiTheme.textColor,
        "print_sequence_button"
    );

    m_topBar = row(
        Modifier().setfixedHeight(64.f).align(Align::TOP).setColor(uiTheme.fgColor),
    contains{
        spacer(Modifier().align(Align::LEFT).setfixedWidth(16)),
        m_enableBlackButton,
        m_resetButton,
        spacer(Modifier().align(Align::CENTER_X).setfixedWidth(16)),
        m_pauseButton,
        spacer(Modifier().align(Align::CENTER_X).setfixedWidth(16)),
        m_moveButton,
        spacer(Modifier().align(Align::CENTER_X).setfixedWidth(16)),
        m_printSequenceButton,
        m_enableWhiteButton,
        spacer(Modifier().align(Align::RIGHT).setfixedWidth(16)),
    });

    m_blackDepthSlider = slider(
        Modifier(),
        sf::Color::White,
        sf::Color::Black,
        SliderOrientation::Horizontal,
        0.4f
    );
    m_blackDepthSlider->setQuantization(9);

    m_whiteDepthSlider = slider(
        Modifier(),
        sf::Color::White,
        sf::Color::Black,
        SliderOrientation::Horizontal,
        0.4f
    );
    m_whiteDepthSlider->setQuantization(9);

    m_blackAlphaBetaToggle = button(
        Modifier()
            .setfixedHeight(24.f)
            .setfixedWidth(24.f)
            .align(Align::RIGHT | Align::CENTER_Y)
            .setColor(m_blackBot.alphaBetaEnabled() ? uiTheme.buttonColor : sf::Color::Black)
            .onLClick([&](){ 
                m_blackBot.toggleAlphaBeta(); 
                m_blackAlphaBetaToggle->m_modifier.setColor(m_blackBot.alphaBetaEnabled() ? uiTheme.buttonColor : sf::Color::Black); 
            }),
        ButtonStyle::Pill,
        "",
        "",
        uiTheme.textColor,
        "black_alpha_beta_toggle"
    );

    m_whiteAlphaBetaToggle = button(
        Modifier()
            .setfixedHeight(24.f)
            .setfixedWidth(24.f)
            .align(Align::RIGHT | Align::CENTER_Y)
            .setColor(m_whiteBot.alphaBetaEnabled() ? uiTheme.buttonColor : sf::Color::Black)
            .onLClick([&](){ 
                m_whiteBot.toggleAlphaBeta(); 
                m_whiteAlphaBetaToggle->m_modifier.setColor(m_whiteBot.alphaBetaEnabled() ? uiTheme.buttonColor : sf::Color::Black); 
            }),
        ButtonStyle::Pill,
        "",
        "",
        uiTheme.textColor,
        "white_alpha_beta_toggle"
    );

    m_enableBlackTree = button(
        Modifier()
            .setfixedHeight(32.f)
            .setfixedWidth(120.f)
            .align(Align::CENTER_X | Align::CENTER_Y)
            .setColor(uiTheme.buttonColor)
            .onLClick([&](){ 
                if (!m_blackTree) {
                    m_blackTree = new TreeDisplay(const_cast<SearchTree&>(m_blackBot.getSearchTree()));
                }
            }),
        ButtonStyle::Pill,
        "Enable Tree",
        "",
        uiTheme.textColor,
        "enable_black_tree_button"
    );

    m_enableWhiteTree = button(
        Modifier()
            .setfixedHeight(32.f)
            .setfixedWidth(120.f)
            .align(Align::CENTER_X | Align::CENTER_Y)
            .setColor(uiTheme.buttonColor)
            .onLClick([&](){ 
                if (!m_whiteTree) {
                    m_whiteTree = new TreeDisplay(const_cast<SearchTree&>(m_whiteBot.getSearchTree()));
                }
            }),
        ButtonStyle::Pill,
        "Enable Tree",
        "",
        uiTheme.textColor,
        "enable_white_tree_button"
    );

    m_blackColumn = scrollableColumn(
        Modifier().setfixedWidth(256).setColor(uiTheme.middleColor).align(Align::LEFT),
    contains{
        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(32).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            text(
                Modifier().setfixedHeight(24).setColor(uiTheme.textColor).align(Align::CENTER_Y),
                "Depth",
                "",
                "black_depth_text"
            )
        }),

        row(
            Modifier().setfixedHeight(32).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            m_blackDepthSlider
        }),

        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(32).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            text(
                Modifier().setfixedHeight(24).setColor(uiTheme.textColor).align(Align::CENTER_Y),
                "Alpha / Beta",
                ""
            ),
            m_blackAlphaBetaToggle
        }),

        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(40).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            m_enableBlackTree
        }),

        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(24).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            text(
                Modifier().setfixedHeight(24).setColor(uiTheme.textColor).align(Align::CENTER_Y),
                "",
                "",
                "black_states_text"
            )
        }),
    }); m_blackColumn->m_modifier.setVisible(false);

    m_whiteColumn = scrollableColumn(
        Modifier().setfixedWidth(256).setColor(uiTheme.middleColor).align(Align::RIGHT),
    contains{
        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(32).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            text(
                Modifier().setfixedHeight(24).setColor(uiTheme.textColor).align(Align::CENTER_Y),
                "Depth",
                "",
                "white_depth_text"
            )
        }),

        row(
            Modifier().setfixedHeight(32).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            m_whiteDepthSlider
        }),

        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(32).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            text(
                Modifier().setfixedHeight(24).setColor(uiTheme.textColor).align(Align::CENTER_Y),
                "Alpha / Beta",
                ""
            ),
            m_whiteAlphaBetaToggle
        }),

        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(40).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            m_enableWhiteTree
        }),

        spacer(Modifier().setfixedHeight(16)),

        row(
            Modifier().setfixedHeight(24).setWidth(0.8f).align(Align::CENTER_X),
        contains{
            text(
                Modifier().setfixedHeight(24).setColor(uiTheme.textColor).align(Align::CENTER_Y),
                "",
                "",
                "white_states_text"
            )
        }),
    }); m_whiteColumn->m_modifier.setVisible(false);

    m_mainContentRow = row(
        Modifier().setColor(uiTheme.bgColor),
    contains{
        m_blackColumn,
        m_whiteColumn
    });

    m_ui->addPage(page({
        column(
            Modifier(),
        contains{ m_topBar, m_mainContentRow })
    }), "base_page");

    return m_window.isOpen() && m_ui->isRunning();
}

void Othello::update() {
    /*
        Base update function. Update game, UI, and handle key presses for main window
    */
    m_running = m_ui->isRunning() && m_window.isOpen();

    static bool prevF11 = false;
    bool f11 = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F11);

    if (f11 && !prevF11) {
        m_fullscreen = !m_fullscreen;
        initUI();
    }

    updateGame();
    m_ui->forceUpdate();

    if (m_blackTree) {
        m_blackTree->update();
        if (!m_blackTree->isRunning()) {
            delete m_blackTree;
            m_blackTree = nullptr;
        }
    }
    if (m_whiteTree) {
        m_whiteTree->update();
        if (!m_whiteTree->isRunning()) {
            delete m_whiteTree;
            m_whiteTree = nullptr;
        }
    }

    prevF11 = f11;
}

void Othello::render() {
    /*
        Render the UILO UI and the grid
    */
    if (m_ui->windowShouldUpdate()) {
        m_window.clear();
        m_ui->render();
        renderGrid();
        m_window.display();
    }
}

void Othello::updateGame() {
    /*
        Update board state, check for player move, execute bot moves
    */
    m_blackDepth = static_cast<int>((m_blackDepthSlider->getValue() * 10) + 1);
    if (m_blackDepth == 11) m_blackDepth -= 1;
    m_blackBot.setDepth(m_blackDepth);

    m_whiteDepth = static_cast<int>((m_whiteDepthSlider->getValue() * 10) + 1);
    if (m_whiteDepth == 11) m_whiteDepth -= 1;
    m_whiteBot.setDepth(m_whiteDepth);

    m_ui->getText("black_depth_text")->setString("Depth: " + std::to_string(m_blackDepth));
    m_ui->getText("white_depth_text")->setString("Depth: " + std::to_string(m_whiteDepth));
    
    m_ui->getText("black_states_text")->setString("States: " + std::to_string(m_blackBot.getTreeSize()));
    m_ui->getText("white_states_text")->setString("States: " + std::to_string(m_whiteBot.getTreeSize()));

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    if (mousePressed && !m_mouseWasPressed) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
        auto [row, col] = mouseToGridPos(mousePos);
        
        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            m_board.updatePossibleStates();
            std::string key = std::to_string(row) + ":" + std::to_string(col);
            
            if (m_board.possibleStates.find(key) != m_board.possibleStates.end()) {
                char turn = m_board.turn;
                m_board.place(row, col);
                m_moveHistory.push_back(std::string(1, turn) + ": " + key);
            }
        }
    }
    m_mouseWasPressed = mousePressed;

    m_board.updatePossibleStates();

    if (!m_paused && !m_botThinking && !m_waitingForTimer)
        if ((m_board.turn == 'b' && m_blackEnabled) || (m_board.turn == 'w' && m_whiteEnabled))
            startBotThinking();
    
    if (m_botThinking && m_botMoveResult.valid()) {
        if (m_botMoveResult.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            m_pendingMove = m_botMoveResult.get();
            m_botThinking = false;
            m_waitingForTimer = true;
            m_botStartTime = std::chrono::steady_clock::now();
            
            if (m_blackTree && m_blackTree->isRunning())
                m_blackTree->setTree(const_cast<SearchTree&>(m_blackBot.getSearchTree()));
            if (m_whiteTree && m_whiteTree->isRunning())
                m_whiteTree->setTree(const_cast<SearchTree&>(m_whiteBot.getSearchTree()));
        }
    }
    
    if (m_waitingForTimer) {
        auto elapsed = std::chrono::steady_clock::now() - m_botStartTime;
        if (elapsed >= m_botMoveDelay) {
            m_waitingForTimer = false;
            if (m_pendingMove.first != -1 && m_pendingMove.second != -1) {
                char turn = m_board.turn == 'b' ? 'B' : 'W';
                m_board.place(m_pendingMove.first, m_pendingMove.second);
                m_moveHistory.push_back(std::string(1, turn) + ": " + std::to_string(m_pendingMove.first) + ":" + std::to_string(m_pendingMove.second));
                m_pendingMove = {-1, -1};
            }
        }
    }
}

void Othello::renderGrid() {
    /*
        Render the 8x8 grid
    */
    float leftColumnWidth = m_blackColumn->m_modifier.isVisible() ? 256.f : 0.f;
    float rightColumnWidth = m_whiteColumn->m_modifier.isVisible() ? 256.f : 0.f;
    float topBarHeight = 64.f;
    
    m_gridX = leftColumnWidth;
    m_gridY = topBarHeight;
    float gridWidth = m_screenRes.size.x - leftColumnWidth - rightColumnWidth;
    float gridHeight = m_screenRes.size.y - topBarHeight;
    
    m_gridSize = std::min(gridWidth, gridHeight);    
    m_gridX += (gridWidth - m_gridSize) * 0.5f;
    m_gridY += (gridHeight - m_gridSize) * 0.5f;
    
    m_cellSize = m_gridSize / 8.0f;
    
    sf::RectangleShape line;
    line.setFillColor(uiTheme.textColor);
    
    for (int i = 0; i <= 8; ++i) {
        line.setSize({2.f, m_gridSize});
        line.setPosition({m_gridX + i * m_cellSize - 1.f, m_gridY});
        m_window.draw(line);
    }
    
    for (int i = 0; i <= 8; ++i) {
        line.setSize({m_gridSize, 2.f});
        line.setPosition({m_gridX, m_gridY + i * m_cellSize - 1.f});
        m_window.draw(line);
    }
    
    sf::RectangleShape piece({m_cellSize * 0.6f, m_cellSize * 0.6f});
    piece.setOrigin({m_cellSize * 0.3f, m_cellSize * 0.3f});
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            char cell = m_board.board[row][col];
            if (cell != ' ') {
                piece.setFillColor(cell == 'b' ? sf::Color::Black : sf::Color::White);
                piece.setPosition({
                    m_gridX + row * m_cellSize + m_cellSize * 0.5f,
                    m_gridY + col * m_cellSize + m_cellSize * 0.5f
                });
                m_window.draw(piece);
            }
        }
    }
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
    auto [row, col] = mouseToGridPos(mousePos);
    
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        m_board.updatePossibleStates();
        std::string key = std::to_string(row) + ":" + std::to_string(col);
        
        if (m_board.possibleStates.find(key) != m_board.possibleStates.end()) {
            sf::Color hoverColor = (m_board.turn == 'b') ? sf::Color::Black : sf::Color::White;
            hoverColor.a = 100;
            
            piece.setFillColor(hoverColor);
            piece.setPosition({
                m_gridX + row * m_cellSize + m_cellSize * 0.5f,
                m_gridY + col * m_cellSize + m_cellSize * 0.5f
            });
            m_window.draw(piece);
        }
    }
}

std::pair<int, int> Othello::mouseToGridPos(sf::Vector2i mousePos) {
    /*
        Convert screen coordinates to board coordinates
    */
    if (m_cellSize == 0.f) return {-1, -1};
    
    float relativeX = mousePos.x - m_gridX;
    float relativeY = mousePos.y - m_gridY;
    
    if (relativeX < 0 || relativeY < 0 || relativeX >= m_gridSize || relativeY >= m_gridSize)
        return {-1, -1};
    
    int row = static_cast<int>(relativeX / m_cellSize);
    int col = static_cast<int>(relativeY / m_cellSize);
    
    return {row, col};
}

void Othello::startBotThinking() {
    /*
        Execute Bot turn in separate thread to not block UI
    */
    m_botThinking = true;
    
    Board::State boardCopy = m_board;
    char currentTurn = m_board.turn;
    
    if (currentTurn == 'b') {
        m_botMoveResult = std::async(std::launch::async, [this, boardCopy]() mutable {
            return m_blackBot.getBestMove(boardCopy);
        });
    } else {
        m_botMoveResult = std::async(std::launch::async, [this, boardCopy]() mutable {
            return m_whiteBot.getBestMove(boardCopy);
        });
    }
}