#include "fx.h"


// Print options
void printusage(const char* prog){
  fxmessage("%s options:\n",prog);
  fxmessage("  --load <file>            Load json file.\n");
  fxmessage("  --save <file>            Save json file.\n");
  fxmessage("  --tracelevel <level>     Set trace level.\n");
  fxmessage("  --precision <prec>       Set numeric precision for floating point.\n");
  fxmessage("  --format <format>        Set exponent format for floating point (0=Never,1=Always,2=As-Needed).\n");
  fxmessage("  --flow <flow>            Set output flow format (0=Stream,1=Compact,2=Pretty).\n");
  fxmessage("  --dent <dent>            Set indentation amount.\n");
  fxmessage("  --wrap <columns>         Set line wrap columns.\n");
  fxmessage("  --esc <mode>             Set unicode escape mode (0=OFF,1=\\xHH,2=\\uHHHH).\n");
  fxmessage("  -h, --help               Print help.\n");
  }


// Test JSON I/O
int main(int argc,char *argv[]){
  const FXchar* savefile="test.json";
  const FXchar* loadfile=NULL;
  FXint precision;
  FXint format;
  FXint flow;
  FXint dent;
  FXint wrap;
  FXint esc;

  // JSON I/O
  FXJSONFile json;

  // Variant to save
  FXVariant var;

  // Assume defaults for starters
  precision=json.getNumericPrecision();
  format=json.getNumericFormat();
  flow=json.getOutputFlow();
  dent=json.getIndentation();
  wrap=json.getLineWrap();
  esc=json.getEscapeMode();

  // Grab a few arguments
  for(FXint arg=1; arg<argc; ++arg){
    if(strcmp(argv[arg],"-h")==0 || strcmp(argv[arg],"--help")==0){
      printusage(argv[0]);
      exit(0);
      }
    else if(strcmp(argv[arg],"--load")==0){
      if(++arg>=argc){ fxmessage("Missing load filename argument.\n"); exit(1); }
      loadfile=argv[arg];
      }
    else if(strcmp(argv[arg],"--save")==0){
      if(++arg>=argc){ fxmessage("Missing save filename argument.\n"); exit(1); }
      savefile=argv[arg];
      }
    else if(strcmp(argv[arg],"--tracelevel")==0){
      if(++arg>=argc){ fxmessage("Missing tracelevel number argument.\n"); exit(1); }
      setTraceLevel(strtoul(argv[arg],NULL,0));
      }
    else if(strcmp(argv[arg],"--precision")==0){
      if(++arg>=argc){ fxmessage("Missing precision number argument.\n"); exit(1); }
      precision=strtoul(argv[arg],NULL,0);
      }
    else if(strcmp(argv[arg],"--format")==0){
      if(++arg>=argc){ fxmessage("Missing format number argument.\n"); exit(1); }
      format=strtoul(argv[arg],NULL,0);
      }
    else if(strcmp(argv[arg],"--flow")==0){
      if(++arg>=argc){ fxmessage("Missing flow number argument.\n"); exit(1); }
      flow=strtoul(argv[arg],NULL,0);
      }
    else if(strcmp(argv[arg],"--dent")==0){
      if(++arg>=argc){ fxmessage("Missing indentation amount number argument.\n"); exit(1); }
      dent=strtoul(argv[arg],NULL,0);
      }
    else if(strcmp(argv[arg],"--wrap")==0){
      if(++arg>=argc){ fxmessage("Missing line wrap columns argument.\n"); exit(1); }
      wrap=strtoul(argv[arg],NULL,0);
      }
    else if(strcmp(argv[arg],"--esc")==0){
      if(++arg>=argc){ fxmessage("Missing escape mode argument.\n"); exit(1); }
      esc=strtoul(argv[arg],NULL,0);
      }
    else{
      fxmessage("Bad argument.\n");
      printusage(argv[0]);
      exit(1);
      }
    }

  // If not loading, make up some data
  if(!loadfile){

    // Simple values
    var["real"]=PI;
    var["bool"]=true;
    var["int"]=87654321;
    var["string"]="variant";
    var["char"]='X';
    var["null"]=FXVariant::null;
    var["array"][5]=10.0;
    var["array"][3][2]=10.0;
    var["array"][3][1]='x';
    var["array"][2]="variant";
    var["array"][4]=2.0;
    var["array"][0]=var["array"][5];
    var["array"][1]='c';
    var["map"]["key"]="value";
    var["map"]["more"]["pi"]=3.141592653;
    var["map"]["more"]["e"]=2.718281828;
    var["map"]["more"]["c"]=299792458.0;
    var["map"]["more"]["answer"]=42.0;
    var["map"]["more"]["letter"]="Unicode: \xC3\xBC Hex:\377\xff\b\n\f\v\"";
    var["emptymap"].setType(FXVariant::VMap);
    var["emptyarray"].setType(FXVariant::VArray);
    for(FXival i=0; i<100; ++i){
      var["bigarray"][i]=Math::sin(0.005*i/PI);
      }
    }

  // Load some data
  if(loadfile){

    // Load test
    if(!json.open(loadfile,FXJSON::Load)){
      fxwarning("Error: unable to open: \"%s\" for reading.\n",loadfile);
      return 0;
      }
    fxmessage("Start load from: %s\n",loadfile);
    FXJSON::Error loaderr=json.load(var);
    fxmessage("Loaded %lld bytes, %lld lines\n",json.getOffset(),json.getLine());
    if(loaderr!=FXJSON::ErrOK){
      fxmessage("Error: %s:%d:%d: %s\n",loadfile,json.getLine(),json.getColumn(),FXJSON::getError(loaderr));
      }
    else{
      fxmessage("OK\n");
      }
    json.close();
    }

  // Save some data
  if(savefile){

    // Set precision and format
    json.setNumericPrecision(precision);
    json.setNumericFormat(format);
    json.setIndentation(dent);
    json.setOutputFlow(flow);
    json.setLineWrap(wrap);
    json.setEscapeMode(esc);

    // Report float precision used to save
    fxmessage("Precision: %d format: %d flow: %d dent: %d wrap: %d\n",precision,format,flow,dent,wrap);

    // Save test
    if(!json.open(savefile,FXJSON::Save)){
      fxwarning("Error: unable to open: \"%s\" for writing.\n",savefile);
      return 0;
      }
    fxmessage("Start save to: %s\n",savefile);
    FXJSON::Error saveerr=json.save(var);
    fxmessage("Stored %lld bytes, %lld lines\n",json.getOffset(),json.getLine());
    if(saveerr!=FXJSON::ErrOK){
      fxmessage("Error: %s:%d:%d: %s\n",savefile,json.getLine(),json.getColumn(),FXJSON::getError(saveerr));
      }
    else{
      fxmessage("OK\n");
      }
    json.close();
    }

  return 0;
  }


