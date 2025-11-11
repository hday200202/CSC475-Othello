/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: Minimax implementation wrapped in a class. Builds search tree
          as the bot explores possibilities, to use in TreeDisplay.
*/

#include "Board.hpp"
#include "SearchTree.hpp"
#include <climits>
#include <algorithm>

class OthelloBot {
public:
    OthelloBot(){}
    OthelloBot(int depth) : m_depth(depth) {}

    void setDepth(int depth) { m_depth = depth; }
    void toggleAlphaBeta() { m_alphaBetaOn = !m_alphaBetaOn; }
    bool alphaBetaEnabled() const { return m_alphaBetaOn; }
    
    SearchTree& getSearchTree() { return m_searchTree; }
    size_t getTreeSize() const { return m_statesExamined; }

    /*
        get the best move for the current player, 
        for the current state using minimax
    */
    std::pair<int, int> getBestMove(Board::State& state) {
        state.updatePossibleStates();
        
        // game over
        if (state.possibleStates.empty()) return {-1, -1};
        
        m_statesExamined = 0;
        
        // the root of the search tree, will be used in TreeDisplay
        auto searchRoot = std::make_shared<SearchNode>();
        searchRoot->turn = state.turn;
        searchRoot->whiteScore = state.white;
        searchRoot->blackScore = state.black;
        searchRoot->depth = m_depth;
        searchRoot->maximizing = (state.turn == 'w');
        searchRoot->moveSequence = "Root";
        
        m_searchTree.setRoot(searchRoot);
        m_statesExamined++;
        
        // white player is maximizing eval (white - black)
        bool maximizing = (state.turn == 'w');
        int bestValue = maximizing ? INT_MIN : INT_MAX;
        std::pair<int, int> bestMove = {-1, -1};
        
        for (auto& [key, nextState] : state.possibleStates) {
            // create a search node for every possible state
            auto childNode = std::make_shared<SearchNode>();
            size_t colonPos = key.find(':');
            childNode->row = std::stoi(key.substr(0, colonPos));
            childNode->col = std::stoi(key.substr(colonPos + 1));
            childNode->turn = nextState.turn;
            childNode->whiteScore = nextState.white;
            childNode->blackScore = nextState.black;
            childNode->depth = m_depth - 1;
            childNode->maximizing = !maximizing;
            childNode->moveSequence = key;
            
            // determine which minimax function to call (alpha-beta on/off)
            int eval;
            if (m_alphaBetaOn) eval = minimax(nextState, childNode, m_depth - 1, !maximizing, INT_MIN, INT_MAX);
            else eval = minimax(nextState, childNode, m_depth - 1, !maximizing);
            
            // update search node with eval, add it to the tree
            childNode->heuristic = eval;
            searchRoot->children.push_back(childNode);
            
            if ((maximizing && eval > bestValue) || (!maximizing && eval < bestValue)) {
                bestValue = eval;
                bestMove = {childNode->row, childNode->col};
            }
        }
        
        // update heuristic, and number of states explored
        searchRoot->heuristic = bestValue;
        m_searchTree.setSize(m_statesExamined);
        return bestMove;
    }

private:
    int m_depth = 4;
    bool m_alphaBetaOn = false;
    SearchTree m_searchTree;
    size_t m_statesExamined = 0;

    /*
        minimax without alpha-beta pruning
    */
    int minimax(Board::State state, std::shared_ptr<SearchNode> node, int depth, bool maximizing) {
        m_statesExamined++;
        
        // reached max depth
        if (depth == 0) {
            int eval = state.white - state.black;
            node->heuristic = eval;
            return eval;
        }
        
        // update with all possible moves for the state
        state.updatePossibleStates();
        
        // game over
        if (state.possibleStates.empty()) {
            int eval = state.white - state.black;
            node->heuristic = eval;
            return eval;
        }
        
        // white move
        if (maximizing) {
            int maxEval = INT_MIN;
            for (auto& [key, nextState] : state.possibleStates) {
                // create search tree node
                auto childNode = std::make_shared<SearchNode>();
                size_t colonPos = key.find(':');
                childNode->row = std::stoi(key.substr(0, colonPos));
                childNode->col = std::stoi(key.substr(colonPos + 1));
                childNode->turn = nextState.turn;
                childNode->whiteScore = nextState.white;
                childNode->blackScore = nextState.black;
                childNode->depth = depth;
                childNode->maximizing = false;
                childNode->moveSequence = node->moveSequence + " -> " + key;
                
                // recursive call to minimax, update heuristic, add node to tree
                int eval = minimax(nextState, childNode, depth - 1, false);
                childNode->heuristic = eval;
                node->children.push_back(childNode);
                maxEval = std::max(maxEval, eval);
            }
            node->heuristic = maxEval;
            return maxEval;
        } 
        
        // black move
        else {
            int minEval = INT_MAX;
            for (auto& [key, nextState] : state.possibleStates) {
                // create search tree node
                auto childNode = std::make_shared<SearchNode>();
                size_t colonPos = key.find(':');
                childNode->row = std::stoi(key.substr(0, colonPos));
                childNode->col = std::stoi(key.substr(colonPos + 1));
                childNode->turn = nextState.turn;
                childNode->whiteScore = nextState.white;
                childNode->blackScore = nextState.black;
                childNode->depth = depth;
                childNode->maximizing = true;
                childNode->moveSequence = node->moveSequence + " -> " + key;
                
                // recursive call to minimax, update heuristic, add node to tree
                int eval = minimax(nextState, childNode, depth - 1, true);
                childNode->heuristic = eval;
                node->children.push_back(childNode);
                minEval = std::min(minEval, eval);
            }
            node->heuristic = minEval;
            return minEval;
        }
    }

    /*
        minimax with alpha-beta pruning
    */
    int minimax(Board::State state, std::shared_ptr<SearchNode> node, int depth, bool maximizing, int alpha, int beta) {
        m_statesExamined++;

        // reached max depth
        if (depth == 0) {
            int eval = state.white - state.black;
            node->heuristic = eval;
            return eval;
        }

        // update with all possible moves for the state
        state.updatePossibleStates();
        
        // game over
        if (state.possibleStates.empty()) {
            int eval = state.white - state.black;
            node->heuristic = eval;
            return eval;
        }
        
        // white move
        if (maximizing) {
            int maxEval = INT_MIN;
            for (auto& [key, nextState] : state.possibleStates) {
                // create search tree node
                auto childNode = std::make_shared<SearchNode>();
                size_t colonPos = key.find(':');
                childNode->row = std::stoi(key.substr(0, colonPos));
                childNode->col = std::stoi(key.substr(colonPos + 1));
                childNode->turn = nextState.turn;
                childNode->whiteScore = nextState.white;
                childNode->blackScore = nextState.black;
                childNode->depth = depth;
                childNode->maximizing = false;
                childNode->moveSequence = node->moveSequence + " -> " + key;
                
                // recursive call to minimax, update heuristic, add node to tree
                int eval = minimax(nextState, childNode, depth - 1, false, alpha, beta);
                childNode->heuristic = eval;
                node->children.push_back(childNode);
                maxEval = std::max(maxEval, eval);
                
                // update alpha (best value white can guarantee)
                alpha = std::max(alpha, eval);
                
                // prune if black can already guarantee better elsewhere
                if (beta <= alpha) break;
            }
            node->heuristic = maxEval;
            return maxEval;
        } 
        
        // black move
        else {
            int minEval = INT_MAX;
            for (auto& [key, nextState] : state.possibleStates) {
                // create search tree node
                auto childNode = std::make_shared<SearchNode>();
                size_t colonPos = key.find(':');
                childNode->row = std::stoi(key.substr(0, colonPos));
                childNode->col = std::stoi(key.substr(colonPos + 1));
                childNode->turn = nextState.turn;
                childNode->whiteScore = nextState.white;
                childNode->blackScore = nextState.black;
                childNode->depth = depth;
                childNode->maximizing = true;
                childNode->moveSequence = node->moveSequence + " -> " + key;
                
                // recursive call to minimax, update heuristic, add node to tree
                int eval = minimax(nextState, childNode, depth - 1, true, alpha, beta);
                childNode->heuristic = eval;
                node->children.push_back(childNode);
                minEval = std::min(minEval, eval);
                
                // update beta (best value black can guarantee)
                beta = std::min(beta, eval);
                
                // prune if white can already guarantee better elsewhere
                if (beta <= alpha) break;
            }
            node->heuristic = minEval;
            return minEval;
        }
    }
};