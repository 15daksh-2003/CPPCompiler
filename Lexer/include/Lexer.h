#pragma once

#include "ILexer.h"
#include <string>
#include <unordered_set>

namespace CPPCompiler {
    namespace Lexer {

        class Lexer : public ILexer {
        public:
            Lexer(const std::string& source);

            Token getNextToken() override;

			Token runAutomaton(const Automaton& automaton) override;

			TokenType determineTokenType(const std::string& lexeme, const Automaton& automaton) override;

        private:
            // Data Members
            std::string sourceBuffer;
            size_t currentPosition;
            size_t line;
            size_t column;

            std::unordered_set<std::string> keywords;
            std::unordered_set<std::string> operators;
			std::unordered_set<std::string> separators;

            Automaton identifierAutomaton;
            Automaton numberAutomaton;
            Automaton stringAutomaton;
            Automaton operatorAutomaton;
            Automaton separatorAutomaton;

            // Methods
            void initialize();
            void skipWhitespaceAndComments();
            void initializeAutomata();
            void populateIdentifierTransitions();
            void populateNumberTransitions();
            void populateStringTransitions();
            void populateOperatorTransitions();
            void populateSeparatorTransitions();
            void reportError(const std::string& message);

            char peekChar(int offset) const;
            char readChar();
            bool isEOF() const;

            // Helper methods
            bool isIdentifierStart(char ch) const;
            bool isSeparatorStart(char ch) const;
            bool isDigit(char ch) const;
            bool isOperatorStart(char ch) const;
        };

    }
}
