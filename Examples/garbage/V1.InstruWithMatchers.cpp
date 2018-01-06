#include <string>
#include <iostream>
#include <fstream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/SourceLocation.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");

//int libr = 0;
int nr_func = 0;
int ma = 0;
std::vector<std::string> name_funcs;

class Handler : public MatchFinder::MatchCallback {
public:
  Handler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {

    SourceManager &srcMgr = Result.Context->getSourceManager();

    //Transforms the number of the function in a string
    std::string s = std::to_string(nr_func); 

    //Check the header files (working)
    /*if (const TranslationUnitDecl *TS = Result.Nodes.getNodeAs<clang::TranslationUnitDecl>("begin")){
      ASTContext &coise = TS->getASTContext();
      DynTypedNodeList abc = coise->getParents(TS);
      abc->dump();
    }*/

      /*if (const TypedefDecl *TD = Result.Nodes.getNodeAs<clang::TypedefDecl>("begin")){
          SourceRange ax = TD->getSourceRange();
          SourceLocation in = ax.getBegin();
          SourceLocation out = ax.getEnd();
          Rewrite.InsertText(in.getLocWithOffset(0), "#include<stdio.h>\n", true, true); 
      }*/


    //Checks if the node is a function
    if (const FunctionDecl *FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("func")){
            //if(libr==0){
              //SourceRange ax = FS->getSourceRange();
              //SourceLocation in = ax.getBegin();
              //Rewrite.InsertText(FS->getPointOfInstantiation(), "\n#include<stdio.h>\n", true, true); 
              //libr=1;
            //}
        if(FS->hasBody()){
        //Puts the info about the function in a string vector to insert into index.txt
          name_funcs.push_back(s+":"+FS->getLocStart().printToString(srcMgr)+":"+FS->getNameInfo().getAsString()); 

        //It will take the body of the function
          const Stmt *Bod = FS->getBody();

        //Checks if it is the main function
          if(FS->isMain()){
            ma=1;
            Rewrite.InsertText(Bod->getLocStart().getLocWithOffset(1), 
              "\n \tinitMeasure(); \n \tCRapl rapl = create_rapl(0); \n\trapl_before(rapl);", true, true);  
          }
          else{
            Rewrite.InsertText(Bod->getLocStart().getLocWithOffset(1), 
              "\n \tCRapl rapl = create_rapl(0); \n\trapl_before(rapl);", true, true);
          }

        //Checks if the function has a return statement
          if(FS->getReturnType().getAsString() == "void"){
            Rewrite.InsertText(Bod->getLocEnd(), 
              "\trapl_after("+s+",rapl); \n", true, true);
          }
        }
      }

    //Checks if the node is a return statement
      if (const ReturnStmt *RS = Result.Nodes.getNodeAs<clang::ReturnStmt>("return")){
      if(ma==1){
        Rewrite.InsertText(RS->getLocStart(), 
          "\nrapl_after("+s+",rapl); \nwriteMeasure(\"programa\"); \n", true, true);
      }
      else{        
        Rewrite.InsertText(RS->getLocStart(), 
          "rapl_after("+s+",rapl); \n", true, true);
      }
    }
     nr_func++;
  }

private:
  Rewriter &Rewrite;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : HandlerForFunc(R), HandlerForReturn(R) {
    // Add Matcher for find functionDecl and returnStmt
    Matcher.addMatcher(functionDecl().bind("func"), &HandlerForFunc);
    Matcher.addMatcher(returnStmt().bind("return"), &HandlerForReturn);
    //Matcher.addMatcher(typedefDecl().bind("begin"), &HandlerForLibraries);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    // Run the matchers when we have the whole TU parsed.
    Matcher.matchAST(Context);
  }

private:
  Handler HandlerForFunc;
  Handler HandlerForReturn;
  //Handler HandlerForLibraries;
  MatchFinder Matcher;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
    .write(llvm::outs());
    TheRewriter.overwriteChangedFiles();
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
   StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};


void writeIndex(){
  std::ofstream myfile;
  myfile.open ("../Generated/index.txt");
  for(auto const& value: name_funcs) {
    myfile << value + "\n";
  }
  myfile.close();
}


int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  int result = Tool.run(newFrontendActionFactory<MyFrontendAction>().get());

  writeIndex();

  return result;
}