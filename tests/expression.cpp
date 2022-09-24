#include"fx.h"
#include "FXExpression.h"


/* Evaluate expression with arguments */
int main(int argc,char **argv){
  FXdouble parameter[10];
  FXExpression::Error err;
  FXExpression expr;
  FXdouble result;
  FXint i;
#ifndef NDEBUG
  setTraceLevel(101);
#endif
  if(1<argc){
    fxmessage("evaluate(%s) :",argv[1]);
    err=expr.parse(argv[1],(2<argc)?argv[2]:nullptr);
    if(err!=FXExpression::ErrOK){
      fxwarning(" Error: %s\n",FXExpression::getError(err));
      exit(1);
      }
    for(i=3; i<argc; i++){
      parameter[i-3]=strtod(argv[i],nullptr);
      }
    result=expr.evaluate(parameter);
    fxmessage(" = %.17lg\n",result);
    }
  return 0;
  }


