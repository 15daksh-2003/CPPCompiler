#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace CPPCompiler {
    namespace Lexer {

        enum class TokenType {
            Identifier,
            Keyword,
            Literal,
            Operator,
            Separator,
            Comment,
            PreprocessorDirective,
            Unknown,
            EndOfFile
            // ... other token types
        };

        struct Token {
            TokenType type;
            std::string lexeme;
            size_t line;
            size_t column;
            // Additional data members as needed
        };

        // Define a type for state identifiers
        using State = int;

        // Structure to represent a finite automaton
        struct Automaton {
            State startState;
            std::unordered_set<State> acceptingStates;
            std::unordered_map<State, std::unordered_map<char, State>> transitions;
        };


        class ILexer {
        public:
            virtual Token getNextToken() = 0;
            virtual Token runAutomaton(const Automaton& automaton) = 0;
            virtual TokenType determineTokenType(const std::string& lexeme, const Automaton& automaton) = 0;
            virtual ~ILexer() = default;
        };

    }
}
