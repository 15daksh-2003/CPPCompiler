#include "Lexer.h"
#include <cctype>
#include <unordered_set>
#include <iostream> // For reportError method

namespace CPPCompiler {
    namespace Lexer {

        Lexer::Lexer(const std::string& source)
            : sourceBuffer(source), currentPosition(0), line(1), column(1) {
            initialize();
            initializeAutomata();
        }

        void Lexer::initialize() {
            // Initialize keyword,operator and separators sets
            keywords = {
                "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
            };

            operators = {
                // Arithmetic Operators
                "+", "-", "*", "/", "%",
                // Increment and Decrement Operators
                "++", "--",
                // Relational Operators
                "==", "!=", "<", ">", "<=", ">=",
                // Logical Operators
                "&&", "||", "!",
                // Bitwise Operators
                "&", "|", "^", "~", "<<", ">>",
                // Assignment Operators
                "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
                // Member and Pointer Operators
                ".", "->", ".*", "->*",
                // Conditional Operator
                "?", ":",
                // Scope Resolution Operator
                "::",
                // Three-Way Comparison Operator
                "<=>",
                // Other Operators
                "::", ".*", "->*"
            };

            separators = {
                ";", ",", "(", ")", "{", "}", "[", "]", ":", "...", "->", ".*", "->*"
            };
        }

        void Lexer::initializeAutomata() {
            // Initialize Identifier Automaton
            identifierAutomaton.startState = 0;
            identifierAutomaton.acceptingStates = { 1 };
            populateIdentifierTransitions();

            // Initialize Number Automaton
            numberAutomaton.startState = 0;
            numberAutomaton.acceptingStates = { 1, 3, 6 }; // Accept integers, decimals, exponents
            populateNumberTransitions();

            // Initialize String Automaton
            stringAutomaton.startState = 0;
            stringAutomaton.acceptingStates = { 2 };
            populateStringTransitions();

            // Initialize Operator Automaton
            operatorAutomaton.startState = 0;
            // Accepting states will be set during population
            populateOperatorTransitions();

            // Initialize Separator Automaton
            separatorAutomaton.startState = 0;
            separatorAutomaton.acceptingStates = { 1 };
            populateSeparatorTransitions();
        }

        Token Lexer::getNextToken() {

            std::cerr <<"source buffer size is: " << sourceBuffer.size() << std::endl;

            skipWhitespaceAndComments();

            if (isEOF()) {
                return Token{ TokenType::EndOfFile, "", line, column };
            }

            size_t tokenLine = line;
            size_t tokenColumn = column;

            char ch = peekChar(0);

            if (isIdentifierStart(ch)) {
                return runAutomaton(identifierAutomaton);
            }
            else if (isDigit(ch) || (ch == '.' && isDigit(peekChar(1)))) {
                return runAutomaton(numberAutomaton);
            }
            else if (ch == '"' || ch == '\'') {
                return runAutomaton(stringAutomaton);
            }
            // 4. Ellipsis (specific separator before general operators/separators)
            else if (ch == '.' && peekChar(1) == '.' && peekChar(2) == '.') {
                // Ensure separatorAutomaton handles '...' correctly starting from '.'
                return runAutomaton(separatorAutomaton);
            }
            else if (isOperatorStart(ch)) {
                return runAutomaton(operatorAutomaton);
            }
            else if (isSeparatorStart(ch)) {
                return runAutomaton(separatorAutomaton);
            }
            else {
                reportError("Unrecognized character");
                std::string lexeme(1, readChar());
                return Token{ TokenType::Unknown, lexeme, tokenLine, tokenColumn };
            }
        }

        Token Lexer::runAutomaton(const Automaton& automaton) {
            State currentState = automaton.startState;
            size_t startPosition = currentPosition;
            size_t tokenLine = line;
            size_t tokenColumn = column;
            std::string lexeme;

            char firstChar = peekChar(0);
            char stringTerminator = '\0';
            if (&automaton == &stringAutomaton && (firstChar == '"' || firstChar == '\'')) {
                stringTerminator = firstChar; // Remember the type of quote
            }

            while (!isEOF()) {
                char ch = peekChar(0);
                auto stateTransitions = automaton.transitions.find(currentState);

                if (stateTransitions != automaton.transitions.end()) {
                    auto charTransition = stateTransitions->second.find(ch);

                    if (charTransition != stateTransitions->second.end()) {
                        currentState = charTransition->second;
                        lexeme += readChar();

                        // Special handling for string literals
                        if (&automaton == &stringAutomaton && ch == stringTerminator && currentState == 2) {
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }

            if (automaton.acceptingStates.count(currentState)) {
                TokenType type = determineTokenType(lexeme, automaton);
                return Token{ type, lexeme, tokenLine, tokenColumn };
            }
            else {
                reportError("Invalid token: " + lexeme);
                currentPosition = startPosition + 1; // Move past the invalid character
                return getNextToken();
            }
        }

        TokenType Lexer::determineTokenType(const std::string& lexeme, const Automaton& automaton) {
            if (&automaton == &identifierAutomaton) {
                return keywords.count(lexeme) ? TokenType::Keyword : TokenType::Identifier;
            }
            else if (&automaton == &numberAutomaton) {
                return TokenType::Literal;
            }
            else if (&automaton == &stringAutomaton) {
                return TokenType::Literal;
            }
            else if (&automaton == &operatorAutomaton) {
                return TokenType::Operator;
            }
            else if (&automaton == &separatorAutomaton) {
                return TokenType::Separator;
            }
            else {
                return TokenType::Unknown;
            }
        }

        void Lexer::populateSeparatorTransitions() {
            // For single-character separators
            std::unordered_set<char> singleSeparators = { ';', ',', '(', ')', '{', '}', '[', ']', ':' };
            for (char sep : singleSeparators) {
                separatorAutomaton.transitions[0][sep] = 1;
            }

            // For multi-character separators
            // Handle '::', '...', '->', '.*', '->*'
            separatorAutomaton.transitions[0][':'] = 2;
            separatorAutomaton.transitions[2][':'] = 1; // Accept '::'

            separatorAutomaton.transitions[0]['.'] = 3;
            separatorAutomaton.transitions[3]['.'] = 4;
            separatorAutomaton.transitions[4]['.'] = 1; // Accept '...'

            separatorAutomaton.transitions[0]['-'] = 5;
            separatorAutomaton.transitions[5]['>'] = 1; // Accept '->'

            separatorAutomaton.transitions[0]['.'] = 3;
            separatorAutomaton.transitions[3]['*'] = 1; // Accept '.*'

            separatorAutomaton.transitions[5]['>'] = 6;
            separatorAutomaton.transitions[6]['*'] = 1; // Accept '->*'

            // Update accepting states if needed
            separatorAutomaton.acceptingStates.insert(1);
        }

        void Lexer::populateIdentifierTransitions() {
            // From State 0 to State 1: letters and '_'
            for (char ch = 'A'; ch <= 'Z'; ++ch) {
                identifierAutomaton.transitions[0][ch] = 1;
                identifierAutomaton.transitions[1][ch] = 1;
            }
            for (char ch = 'a'; ch <= 'z'; ++ch) {
                identifierAutomaton.transitions[0][ch] = 1;
                identifierAutomaton.transitions[1][ch] = 1;
            }
            identifierAutomaton.transitions[0]['_'] = 1;
            identifierAutomaton.transitions[1]['_'] = 1;

            // From State 1 to State 1: letters, digits, and '_'
            for (char ch = '0'; ch <= '9'; ++ch) {
                identifierAutomaton.transitions[1][ch] = 1;
            }
        }

        /*
        States:

        State 0: Start state.

        State 1: Integer part.

        State 2: Decimal point encountered.

        State 3: Fractional part.

        State 4: Exponent symbol encountered ('e' or 'E').

        State 5: Exponent sign.

        State 6: Exponent part.

        State 7: Invalid state.
        */

        void Lexer::populateNumberTransitions() {
            // Digits 0-9
            for (char ch = '0'; ch <= '9'; ++ch) {
                // From Start State to Integer Part
                numberAutomaton.transitions[0][ch] = 1;
                // Integer Part to Integer Part
                numberAutomaton.transitions[1][ch] = 1;
                // Fractional Part to Fractional Part
                numberAutomaton.transitions[3][ch] = 3;
                // Exponent Part to Exponent Part
                numberAutomaton.transitions[6][ch] = 6;
            }

            // Decimal Point
            numberAutomaton.transitions[1]['.'] = 2; // Integer Part to Decimal Point
            numberAutomaton.transitions[0]['.'] = 2; // Start State to Decimal Point

            // After Decimal Point
            for (char ch = '0'; ch <= '9'; ++ch) {
                numberAutomaton.transitions[2][ch] = 3; // Decimal Point to Fractional Part
            }

            // Exponent Symbol
            numberAutomaton.transitions[1]['e'] = 4;
            numberAutomaton.transitions[1]['E'] = 4;
            numberAutomaton.transitions[3]['e'] = 4;
            numberAutomaton.transitions[3]['E'] = 4;

            // Exponent Sign
            numberAutomaton.transitions[4]['+'] = 5;
            numberAutomaton.transitions[4]['-'] = 5;

            // Exponent Part
            for (char ch = '0'; ch <= '9'; ++ch) {
                numberAutomaton.transitions[5][ch] = 6; // After Exponent Sign
                numberAutomaton.transitions[4][ch] = 6; // Directly after 'e' or 'E'
                numberAutomaton.transitions[6][ch] = 6; // Continue Exponent Part
            }
        }

        /*
        States:

        State 0: Start state.

        State 1: Inside string.

        State 2: Accepting state (end of string).

        State 3: Escape character.

        */
        void Lexer::populateStringTransitions() {
            // Opening Quotes
            stringAutomaton.transitions[0]['"'] = 1;
            stringAutomaton.transitions[0]['\''] = 1;

            // Any character inside the string (excluding special characters)
            for (int ch = 32; ch <= 126; ++ch) {
                char c = static_cast<char>(ch);
                if (c != '"' && c != '\'' && c != '\\') {
                    stringAutomaton.transitions[1][c] = 1;
                }
            }

            // Handling escape sequences
            stringAutomaton.transitions[1]['\\'] = 3; // Escape character
            // After escape character, accept any character
            for (int ch = 0; ch <= 127; ++ch) {
                char c = static_cast<char>(ch);
                stringAutomaton.transitions[3][c] = 1;
            }

            // Closing Quotes
            stringAutomaton.transitions[1]['"'] = 2;
            stringAutomaton.transitions[1]['\''] = 2;
        }

        void Lexer::populateOperatorTransitions() {
            operatorAutomaton.startState = 0;
            int nextState = 1;

            for (const std::string& op : operators) {
                int currentState = operatorAutomaton.startState;
                for (char ch : op) {
                    if (operatorAutomaton.transitions[currentState].find(ch) == operatorAutomaton.transitions[currentState].end()) {
                        operatorAutomaton.transitions[currentState][ch] = nextState++;
                    }
                    currentState = operatorAutomaton.transitions[currentState][ch];
                }
                operatorAutomaton.acceptingStates.insert(currentState);
            }
        }


        // Implementations of helper methods...
        bool Lexer::isIdentifierStart(char ch) const {
            return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
        }

        bool Lexer::isOperatorStart(char ch) const {
            auto stateTransitionsIt = operatorAutomaton.transitions.find(operatorAutomaton.startState);
            return stateTransitionsIt != operatorAutomaton.transitions.end() &&
                stateTransitionsIt->second.count(ch) > 0;
        }

        bool Lexer::isSeparatorStart(char ch) const {
            auto stateTransitionsIt = separatorAutomaton.transitions.find(separatorAutomaton.startState);
            return stateTransitionsIt != separatorAutomaton.transitions.end() &&
                stateTransitionsIt->second.count(ch) > 0;
        }

        bool Lexer::isDigit(char ch) const {
            return std::isdigit(static_cast<unsigned char>(ch));
        }

        char Lexer::peekChar(int offset) const {
            size_t pos = currentPosition + offset;
            if (pos < sourceBuffer.size()) {
                return sourceBuffer[pos];
            }
            else {
                return '\0';
            }
        }

        char Lexer::readChar() {
            if (currentPosition >= sourceBuffer.size()) {
                return '\0';
            }
            char ch = sourceBuffer[currentPosition++];
            if (ch == '\n') {
                line++;
                column = 1;
            }
            else {
                column++;
            }
            return ch;
        }

        bool Lexer::isEOF() const {
            return currentPosition >= sourceBuffer.size();
        }

        void Lexer::skipWhitespaceAndComments() {
            while (!isEOF()) {
                char ch = peekChar(0);
                if (std::isspace(static_cast<unsigned char>(ch))) {
                    readChar();
                }
                else if (ch == '/') {
                    char nextChar = sourceBuffer[currentPosition + 1];
                    if (nextChar == '/') {
                        // Single-line comment
                        while (!isEOF() && readChar() != '\n');
                    }
                    else if (nextChar == '*') {
                        // Multi-line comment
                        readChar(); // Consume '/'
                        readChar(); // Consume '*'
                        while (!isEOF()) {
                            char c = readChar();
                            if (c == '*' && peekChar(0) == '/') {
                                readChar(); // Consume '/'
                                break;
                            }
                        }
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }

        void Lexer::reportError(const std::string& message) {
            std::cerr << "Lexer error at Line " << line << ", Column " << column << ": " << message << std::endl;
        }

    }
}
