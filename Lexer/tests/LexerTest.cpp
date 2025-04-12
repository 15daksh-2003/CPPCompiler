#include <gtest/gtest.h>
#include "Lexer.h"
#include <sstream>

namespace CPPCompiler {
	namespace Lexer {

        TEST(LexerTest, TestIdentiferTokenization) {
            std::string source = "int main";
            Lexer lexer(source);

            Token token1 = lexer.getNextToken();
            EXPECT_EQ(token1.type, TokenType::Keyword);
            EXPECT_EQ(token1.lexeme, "int");
            EXPECT_EQ(token1.line, 1);
            EXPECT_EQ(token1.column, 1);

            Token token2 = lexer.getNextToken();
            EXPECT_EQ(token2.type, TokenType::Identifier);
            EXPECT_EQ(token2.lexeme, "main");
            EXPECT_EQ(token2.line, 1);
            EXPECT_EQ(token2.column, 5);

            Token token3 = lexer.getNextToken();
            EXPECT_EQ(token3.type, TokenType::EndOfFile);
        }

        TEST(LexerTest, TestNumericLiteralTokenization) {
            std::string source = "42 3.14 6.022e23";
            Lexer lexer(source);

            Token token1 = lexer.getNextToken();
            EXPECT_EQ(token1.type, TokenType::Literal);
            EXPECT_EQ(token1.lexeme, "42");

            Token token2 = lexer.getNextToken();
            EXPECT_EQ(token2.type, TokenType::Literal);
            EXPECT_EQ(token2.lexeme, "3.14");

            Token token3 = lexer.getNextToken();
            EXPECT_EQ(token3.type, TokenType::Literal);
            EXPECT_EQ(token3.lexeme, "6.022e23");

            Token token4 = lexer.getNextToken();
            EXPECT_EQ(token4.type, TokenType::EndOfFile);
        }

        TEST(LexerTest, TestStringLiteralTokenization) {
            std::string source = "\"Hello, World!\"";
            Lexer lexer(source);

            Token token = lexer.getNextToken();
            EXPECT_EQ(token.type, TokenType::Literal);
            EXPECT_EQ(token.lexeme, "\"Hello, World!\"");

            Token eofToken = lexer.getNextToken();
            EXPECT_EQ(eofToken.type, TokenType::EndOfFile);
        }

        TEST(LexerTest, TestCommentSkipping) {
            std::string source = "int x; // This is a comment\nfloat y; /* Multi-line\n comment */ double z;";
            Lexer lexer(source);

            // Token: int
            Token token1 = lexer.getNextToken();
            EXPECT_EQ(token1.type, TokenType::Keyword);
            EXPECT_EQ(token1.lexeme, "int");

            // Token: x
            Token token2 = lexer.getNextToken();
            EXPECT_EQ(token2.type, TokenType::Identifier);
            EXPECT_EQ(token2.lexeme, "x");

            // Token: ;
            Token token3 = lexer.getNextToken();
            EXPECT_EQ(token3.type, TokenType::Separator);

            // Token: float
            Token token4 = lexer.getNextToken();
            EXPECT_EQ(token4.type, TokenType::Keyword);
            EXPECT_EQ(token4.lexeme, "float");

            // Token: y
            Token token5 = lexer.getNextToken();
            EXPECT_EQ(token5.type, TokenType::Identifier);
            EXPECT_EQ(token5.lexeme, "y");

            // Token: ;
            Token token6 = lexer.getNextToken();
            EXPECT_EQ(token6.type, TokenType::Separator);

            // Token: double
            Token token7 = lexer.getNextToken();
            EXPECT_EQ(token7.type, TokenType::Keyword);
            EXPECT_EQ(token7.lexeme, "double");

            // Token: z
            Token token8 = lexer.getNextToken();
            EXPECT_EQ(token8.type, TokenType::Identifier);
            EXPECT_EQ(token8.lexeme, "z");

            // Token: ;
            Token token9 = lexer.getNextToken();
            EXPECT_EQ(token9.type, TokenType::Separator);

            Token eofToken = lexer.getNextToken();
            EXPECT_EQ(eofToken.type, TokenType::EndOfFile);
        }

        TEST(LexerTest, TestErrorHandling) {
            std::string source = "@";
            Lexer lexer(source);

            testing::internal::CaptureStderr();
            Token token = lexer.getNextToken();
            std::string errorOutput = testing::internal::GetCapturedStderr();

            EXPECT_EQ(token.type, TokenType::Unknown);
            EXPECT_FALSE(errorOutput.empty());
            EXPECT_NE(errorOutput.find("Lexer error"), std::string::npos);

            Token eofToken = lexer.getNextToken();
            EXPECT_EQ(eofToken.type, TokenType::EndOfFile);
        }

        TEST(LexerTest, TestSeparatorTokenization) {
            std::string source = "int main() { return 0; }";
            Lexer lexer(source);

            std::vector<Token> tokens;
            Token token;
            while ((token = lexer.getNextToken()).type != TokenType::EndOfFile) {
                tokens.push_back(token);
            }

            std::vector<TokenType> expectedTypes = {
                TokenType::Keyword,     // int
                TokenType::Identifier,  // main
                TokenType::Separator,   // (
                TokenType::Separator,   // )
                TokenType::Separator,   // {
                TokenType::Keyword,     // return
                TokenType::Literal,     // 0
                TokenType::Separator,   // ;
                TokenType::Separator    // }
            };

            ASSERT_EQ(tokens.size(), expectedTypes.size());

            for (size_t i = 0; i < tokens.size(); ++i) {
                EXPECT_EQ(tokens[i].type, expectedTypes[i]) << "Token mismatch at position " << i;
            }
        }

        TEST(LexerTest, TestOperatorTokenization) {
            std::string source = "+ - * / % ++ -- == != < > <= >= && || ! & | ^ ~ << >> = += -= *= /= %= &= |= ^= <<= >>= ? : :: <=> . -> .* ->*";
            Lexer lexer(source);

            std::vector<std::string> expectedOperators = {
                "+", "-", "*", "/", "%", "++", "--", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "!", "&",
                "|", "^", "~", "<<", ">>", "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
                "?", ":", "::", "<=>", ".", "->", ".*", "->*"
            };

            for (const auto& expectedOp : expectedOperators) {
                Token token = lexer.getNextToken();
                EXPECT_EQ(token.type, TokenType::Operator) << "Failed on operator: " << expectedOp;
                EXPECT_EQ(token.lexeme, expectedOp);
            }

            Token eofToken = lexer.getNextToken();
            EXPECT_EQ(eofToken.type, TokenType::EndOfFile);
        }

        TEST(LexerTest, TestDotEdgeCases) {
            // Number starting with '.'
            Lexer lexer1(".5");
            Token token1 = lexer1.getNextToken();
            EXPECT_EQ(token1.type, TokenType::Literal);
            EXPECT_EQ(token1.lexeme, ".5");

            // Member access operator
            Lexer lexer2("object.method()");
            Token token2 = lexer2.getNextToken(); // 'object'
            EXPECT_EQ(token2.type, TokenType::Identifier);

            Token token3 = lexer2.getNextToken(); // '.'
            EXPECT_EQ(token3.type, TokenType::Operator);
            EXPECT_EQ(token3.lexeme, ".");

            Token token4 = lexer2.getNextToken(); // 'method'
            EXPECT_EQ(token4.type, TokenType::Identifier);
        }

        TEST(LexerTest, TestEllipsis) {
            Lexer lexer("...");

            Token token = lexer.getNextToken();
            EXPECT_EQ(token.type, TokenType::Separator);
            EXPECT_EQ(token.lexeme, "...");

            Token eofToken = lexer.getNextToken();
            EXPECT_EQ(eofToken.type, TokenType::EndOfFile);
        }
	}
}
