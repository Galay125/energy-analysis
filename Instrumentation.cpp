#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/StmtIterator.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
static llvm::cl::opt<std::string>  Output ("o", llvm::cl::desc("file Output"));
static llvm::cl::opt<int>  Lines ("l", llvm::cl::desc("number of statements"));
static llvm::cl::opt<std::string>  Dir ("d", llvm::cl::desc("directory"));
static llvm::cl::opt<std::string>  Functions ("fns", llvm::cl::desc("functions that you want to instrument"));

//Rewriter rewriter;
int exist = 0;
int nr_func = -1;
int ma = 0;
std::string funct_name;
std::vector<std::string> name_funcs;

//Number of lines of a function (count stmts)
int count_children(Stmt::const_child_iterator b, Stmt::const_child_iterator e){
  int a=0;
  for(Stmt::const_child_iterator it = b; it != e; it++){

    if(const Stmt *currStmt = *it){
      if(isa<CompoundStmt>(currStmt) || isa<IfStmt>(currStmt) || isa<ForStmt>(currStmt) || isa<WhileStmt>(currStmt)
        || isa<DoStmt>(currStmt) || isa<SwitchCase>(currStmt) || isa<SwitchStmt>(currStmt))
        a+= count_children(currStmt->child_begin(), currStmt->child_end());

      if(isa<DeclStmt>(currStmt)){
        const DeclStmt *child1 = cast<DeclStmt>(currStmt);
        if(child1->getDeclGroup().isSingleDecl()){
          const Decl *child2 = child1->getSingleDecl();
          if(isa<VarDecl>(child2)){
            const VarDecl *child3 = cast<VarDecl>(child2);
            //Check if the RAPL system already exists in the programme
            if(child3->getNameAsString() == "rapl")
              exist = 1;
          }
        }
      }
      a++;
    }
  }
  return a;
}

//Get the name of the file without his Path
std::string getFileName(std::string directory, std::string filePath){
  filePath.erase(0,directory.size());
  filePath.erase(0, 1);
  return filePath;
}

//checks if its header or a filesystem
int isHeaderFile(const FunctionDecl *FS, ASTContext *Context){
 SourceManager &srcMgr = Context->getSourceManager();
 std::string filename = FS->getLocStart().printToString(srcMgr);
 //std::cout << filename + "\n";
 if(!(filename.find(".h")!=std::string::npos) && !(filename.find("/usr/")!=std::string::npos))
  return 0;
else
  return 1;
}

//Put the name of the index and the name of the function in name_funcs for index.txt
void insertIntoIndex(const FunctionDecl *FS, ASTContext *Context){
  SourceManager &srcMgr = Context->getSourceManager();
  std::string filename = FS->getLocStart().printToString(srcMgr);
  
  nr_func++;
  std::string s = std::to_string(nr_func);
  s += ":"+ filename +":"+ FS->getNameInfo().getAsString();
  name_funcs.push_back(s); 
}

//Rewrite rapl_after
void insertRaplAfter(SourceLocation loc, Rewriter &rewriter){
  std::string s = std::to_string(nr_func);
  rewriter.InsertText(loc,"rapl_after("+s+",rapl);\n", true, true);
}

//Rewrite rapl_before
void insertRaplBefore(SourceLocation loc, Rewriter &rewriter){
  std::string s = std::to_string(nr_func);
  rewriter.InsertText(loc.getLocWithOffset(1),
    "\n \tCRapl rapl = create_rapl("+s+"); \n\trapl_before(rapl);", true, true);
}

//Insert Braces in if's and else's
void insertBraces(Stmt *IforElse, Rewriter &rewriter, ASTContext *Context){
  SourceManager &srcMgr = Context->getSourceManager();
  bool invalid;

  CharSourceRange conditionRange = CharSourceRange::getTokenRange(IforElse->getLocStart(), IforElse->getLocStart());
  std::string str = clang::Lexer::getSourceText(conditionRange, srcMgr, Context->getLangOpts(), &invalid);

  if(!invalid && str.compare("{")){
    rewriter.InsertText(IforElse->getLocStart().getLocWithOffset(-1), "{\n", true, true);
    //skip ';'
    str = srcMgr.getCharacterData(IforElse->getLocEnd(), &invalid);
    std::size_t found = str.find(";");

    if(rewriter.InsertText(IforElse->getLocEnd().getLocWithOffset(found+1), "\n}", true, true)){
      str = srcMgr.getCharacterData(IforElse->getLocStart(), &invalid);
      found = str.find(";");
      rewriter.InsertTextAfterToken(IforElse->getLocStart().getLocWithOffset(found+1), "\n}");
    }
  }
}

//Search for { } or ; to know the location where you need to insert rapls
std::size_t SearchCloser(SourceLocation st, SourceManager &srcMgr, std::string brace){
  bool invalid;
  std::string str = srcMgr.getCharacterData(st, &invalid);
  std::size_t foundComma = str.find_first_of(";");
  std::size_t foundBrace = str.find_first_of(brace);

  return foundComma < foundBrace ? foundComma : foundBrace;
}

//Search for the previous Sibling of the call
SourceLocation getPreviousLoc(const Stmt *Compou, const Stmt *Cl, SourceManager &srcMgr){
  SourceLocation aux = Compou->getLocStart();
  std::size_t found;

  if(isa<CompoundStmt>(Compou)){
    const CompoundStmt *cmp = cast<CompoundStmt>(Compou);

    //the call is the only Stmt of { }
    if(cmp->body_front() == cmp->body_back()){
      found = SearchCloser(Compou->getLocStart(), srcMgr, "{");
      return Compou->getLocStart().getLocWithOffset(found+1);
    }
    //the call is the first Stmt of { }
    if(Cl == cmp->body_front()){
      found = SearchCloser(Compou->getLocStart(), srcMgr, "{");
      return Compou->getLocStart().getLocWithOffset(found+1);
    }

    Stmt::const_child_iterator b (Compou->child_begin());
    const Stmt *au;

    //the call is in the middle or in the end of { }
    for(Stmt::const_child_iterator it = b; it != Compou->child_end(); it++){
      if(Cl == *it){
        found = SearchCloser(au->getLocEnd(), srcMgr, "}");
        return au->getLocEnd().getLocWithOffset(found+1);;
      }
      au = *it;
    }
  }
  return aux;
}



class RaplVisitor : public RecursiveASTVisitor<RaplVisitor> {  
public:
  explicit RaplVisitor(ASTContext *Context, Rewriter &rewriter) : Context(Context), rewriter(rewriter)   {}

  bool TraverseDecl(Decl *D){
    RecursiveASTVisitor<RaplVisitor>::TraverseDecl(D);
    return true; // Return false to stop the AST analyzing
  }

  //Visit functions
  bool VisitFunctionDecl(FunctionDecl *FS){
    //isMain?
    ma=0;

    //Check if it is parsing an header file or not
    if(!isHeaderFile(FS, Context) && FS->isThisDeclarationADefinition()){
      funct_name = FS->getNameInfo().getName().getAsString();

      if(!(FS->isMain() || !Functions.compare(funct_name) || Functions.empty()))
        return false;

      const Stmt *Bod = FS->getBody();

      if(FS->isMain() || (count_children(Bod->child_begin(), Bod->child_end()) >= Lines && !exist)){
        insertIntoIndex(FS, Context);

        //Checks if it is the main function
        if(FS->isMain()){
          ma=1;
          rewriter.InsertText(Bod->getLocStart().getLocWithOffset(1),"\n \tinitMeasure();", true, true);
          insertRaplBefore(Bod->getLocStart(), rewriter);
        }
        else{
          insertRaplBefore(Bod->getLocStart(), rewriter);
        }
        
        //if void type
        if(FS->getReturnType().getAsString() == "void"){
          insertRaplAfter(Bod->getLocEnd(), rewriter);
        }
      }
      //Skip the function (node) if doesn't contain more than X Lines of code or RAPL already exists in the progamme
      else
        return false; 
    }
    //If it is an header file skip the function
    else
      return false;

    return true;
  }

  //Add Braces to If and Else
  bool VisitIfStmt(IfStmt *i){
    Stmt *Then = i->getThen();
    insertBraces(Then, rewriter, Context);

    Stmt *Else = i->getElse();
    if(Else){
      if(isa<IfStmt>(Else))
        return true;

      insertBraces(Else, rewriter, Context);
    }
    return true;
  }

  //Return Visitor
  bool VisitReturnStmt(ReturnStmt *ret){
    //isMain?
    if(ma==1){
      SourceManager &srcMgr = Context->getSourceManager();

      std::string st = getFileName(
        srcMgr.getFileEntryForID(srcMgr.getMainFileID())->getDir()->getName().str(),
        srcMgr.getFileEntryForID(srcMgr.getMainFileID())->tryGetRealPathName().str());

      unsigned found = st.find_last_of(".");
      st = st.substr(0, found);
      insertRaplAfter(ret->getReturnLoc(), rewriter);
      rewriter.InsertText(ret->getReturnLoc(), "writeMeasure(\""+st+"\"); \n", true, true);
    }
    //not main
    else{
      if(ret->getRetValue())
        insertRaplAfter(ret->getReturnLoc(), rewriter);
      else
        insertRaplAfter(ret->getLocStart(), rewriter);
    }
    return true;
  }

  //Visit Calls
  bool VisitCallExpr(CallExpr *E){

    if(E->getDirectCallee()){
      const char *call_name = E->getDirectCallee()->getNameInfo().getName().getAsString().c_str();

      //same function? (recursivity?)
      if(strcmp(call_name, funct_name.c_str())==0){

        std::string s = std::to_string(nr_func);
        int aux = -1;

        //getParents of the call
        auto parentNow = Context->getParents(*E).begin();

        const Stmt *ST = parentNow->get<Stmt>();
        const Decl *DC = parentNow->get<Decl>();
        const Stmt *STaux = cast<Stmt>(E);
        SourceRange rang(STaux->getSourceRange());

        //Decl or Stmt - to pick up the CompoundStmt { } to insert rapls
        while(ST || DC){
          if(ST){
            aux = 0;
            if(isa<CompoundStmt>(ST)){
              break;
            }
            //dont do nothing for this cases (maybe later switch will have)
            if(isa<ReturnStmt>(ST) || isa<SwitchCase>(ST) || isa<SwitchStmt>(ST)){
              return true;
            }
            if(Context->getParents(*ST).begin())
              parentNow = Context->getParents(*ST).begin();
            STaux = ST;
          }
          if(DC){
            aux = 1;
            if(Context->getParents(*DC).begin())
              parentNow = Context->getParents(*DC).begin();
          }
          if(parentNow){
            ST = parentNow->get<Stmt>();
            DC = parentNow->get<Decl>();
          }
        }

        if(aux==0){
          //simple call, without decl or other stmts
          if(STaux != E){
            std::size_t found = SearchCloser(STaux->getLocEnd(), Context->getSourceManager(), "}");
            rewriter.InsertText(STaux->getLocEnd().getLocWithOffset(found+1), "\nrapl_before(rapl);\n");
          }
          //more complex call with many parents
          else{
            std::size_t found = SearchCloser(E->getLocStart(), Context->getSourceManager(), "}");
            rewriter.InsertText(E->getLocStart().getLocWithOffset(found+1), "\nrapl_before(rapl);\n");
          }
          SourceLocation prev = getPreviousLoc(ST, STaux, Context->getSourceManager());
          rewriter.InsertText(prev, "\nrapl_after("+s+",rapl);\n", true, true);
        }
      }
    }
    return true;
  }

  //Insert rapl_after before throw
  bool VisitCXXThrowExpr(CXXThrowExpr *tr){
    insertRaplAfter(tr->getLocStart(), rewriter);
    return true;
  }

private:
  ASTContext *Context;
  Rewriter &rewriter;
};



class FindNamedCallConsumer : public clang::ASTConsumer {  
public:
  explicit FindNamedCallConsumer(ASTContext *Context, Rewriter &R)
  : Visitor(Context, R) {
    R.setSourceMgr(Context->getSourceManager(),
      Context->getLangOpts());
  }

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  RaplVisitor Visitor;
};

class FindNamedCallAction : public clang::ASTFrontendAction {  
public:
  FindNamedCallAction() { }

  void EndSourceFileAction() override {
    if(!exist){
      SourceManager &SM = TheRewriter.getSourceMgr();
      TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());

      const char* st;

      if(Output.empty()){
        st = SM.getFileEntryForID(SM.getMainFileID())->tryGetRealPathName().str().c_str();
      }
      else
        st = Output.c_str();

      StringRef file(st);
      std::error_code EC;
      llvm::raw_fd_ostream BOS(file, EC, llvm::sys::fs::F_None);

      BOS<< "#include <crapl/rapl_interface.h>\n"
      "#include <crapl/measures.h>\n";

      TheRewriter.getEditBuffer(SM.getMainFileID()).write(BOS);
    }
    exist = 0;
  }

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
      new FindNamedCallConsumer(&Compiler.getASTContext(), TheRewriter));
  }

private:
  Rewriter TheRewriter;
};


//Write the functions of the parsed progamme
void writeIndex(){

  DIR* dir = opendir("CRapl_Gen");
  if (dir){
    closedir(dir);
  }
  else{
    system("mkdir CRapl_Gen");
  }

  std::ofstream myfile("CRapl_Gen/index.txt");
  for(auto const& value: name_funcs) {
    myfile << value + "\n";
  }

  myfile.close();
}

//Get the files in directory if -d
const std::vector<std::string> filesOfDirectory(const char *ext){
   std::vector<std::string> files_in = {};

  FILE *fp;
  char path[1035];
  char expre[100];
  strcat(expre, "find ");
  strcat(expre, Dir.c_str());
  strcat(expre, " -type f -name \"*");
  strcat(expre, ext);
  strcat(expre, "\"*");
  /* Open the command for reading. */

  fp = popen(expre, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }
  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    strtok(path, "\n");
    files_in.push_back(path);
  }
  /* close */
  pclose(fp);

  return files_in;
}

int main(int argc, const char **argv) {

  //std::cout << Dir <<std::endl;
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  int result = 0;

  //directory or not?
  if(Dir.empty()){
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    result = Tool.run(newFrontendActionFactory<FindNamedCallAction>().get());
  }
  else{
    std::string ext = op.getSourcePathList()[0];
    std::vector<std::string> CommandLineArguments = {}; // Get the arguments
    const clang::tooling::FixedCompilationDatabase Acompilations(Dir.c_str(), CommandLineArguments);
    const std::vector<std::string> files = filesOfDirectory(ext.c_str());
    ClangTool Tool(Acompilations, files);
    result = Tool.run(newFrontendActionFactory<FindNamedCallAction>().get());
  }
  


  writeIndex();

  printf("Number of functions with CRAPL: %d\n", nr_func+1);

  return result;
}