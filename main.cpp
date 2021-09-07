
#include "SyntaxTree.h"


int main(int argc, char **argv) {
    std::cout << "warfLang 1.0" << std::endl;
    while (true) {
        try {
            std::string line = "";
            std::cout << ">>> ";
            std::getline(std::cin, line);
            
            auto syntaxTree = SyntaxTree::Parse(line);
            syntaxTree->PrintTree();
            for (auto error : syntaxTree->Errors()) {
                std::cerr << error << std::endl;
            }
            if (syntaxTree->Errors().size() == 0) {
                std::cout << syntaxTree->Evaluate() << std::endl;
            }
            //Lexer lex(line);
            //while (lex.CurrentToken() != '\0') {
            //    auto sToken = lex.NextToken();
            //    std::cout << sToken->Type() << ": " << sToken->Text();
            //    if (sToken->HasValue())
            //        std::cout << " " << sToken->Value();
            //    std::cout << "\n";
            //}
        }
        catch (std::runtime_error& error) {
            std::cerr << error.what() << std::endl;
            std::cin.get();
        }
        catch (std::exception& error) {
            std::cerr << error.what() << std::endl;
            std::cin.get();
        }
    }
}