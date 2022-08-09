
#include "Syntax/SyntaxTree.h"
#include "Binding/Binder.h"
#include "Evaluator.h"
int main(int argc, char **argv) {
  std::cout << "warfLang 1.0" << std::endl;
  bool showTree = false;
  while (true) {
    try {
      std::string line = "";
      std::cout << ">>> ";
      std::getline(std::cin, line);
      if (line == "#showTree") {
        showTree = !showTree;
        std:: cout << (showTree ? "Showing parse trees." : "Not showing parse trees") << std::endl;
        continue;
      }
      auto syntaxTree = SyntaxTree::Parse(line);
      auto binder = std::make_unique<Binder>();
      auto boundExpression = binder->BindExpression(syntaxTree->Root());
      
      if(showTree) {
        syntaxTree->PrintTree();
      }

      if(binder->Diagnostics().empty()) {
        auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
        if (eval->Errors().size() == 0) {
          int /*Value*/ result = eval->Evaluate();
          std::cout << result << std::endl;
        } else {
          for (auto error : eval->Errors()) {
            std::cerr << error << std::endl;
          }
        }
      } else {
        for (auto error : binder->Diagnostics()) {
          std::cerr << error << std::endl;
        }
      }

    } catch (std::runtime_error &error) {
      std::cerr << error.what() << std::endl;
      std::cin.get();
    } catch (std::exception &error) {
      std::cerr << error.what() << std::endl;
      std::cin.get();
    }
  }
}