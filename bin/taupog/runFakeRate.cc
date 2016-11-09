//
// Pietro Vischia, <pietro.vischia@gmail.com>
//
// Tau fakes analysis
//

#include <string>
#include <vector>
#include <iostream>
//#include <list>
//#include <iterator>
//#include <algorithm>
//#include <unordered_map>

#include "TSystem.h"
#include "TROOT.h"
#include "TError.h"
#include "TFile.h"
#include "TColor.h"
//#include "TDirectory.h"
//#include "TChain.h"
#include "TObject.h"
#include "TCanvas.h"
//#include "TMath.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TGraph.h"
#include "TH1.h"
//#include "TH2.h"
//#include "TH3.h"
//#include "TTree.h"
//#include "TF1.h"
//#include "TCutG.h"
//#include "TGraphErrors.h"
//#include "TGraphAsymmErrors.h"
//#include "TMultiGraph.h"
//#include "TPaveText.h"
#include "TText.h"
#include "THStack.h"
#include "TRandom.h"

#include "UserCode/llvv_fwk/interface/tdrstyle.h"

using namespace std;

bool debug=false;

int cutIndex=-1;
string cutIndexStr="";
double iLumi = 2007;
double iEcm=8;
bool showChi2 = false;
bool showUnc=false;
double baseRelUnc=0.027;
bool noLog=false;
bool logX=false;
bool isSim=false;
bool doData = true;
bool do2D   = true;
bool do1D   = true;
bool doTex  = true;
bool doPowers = true;
bool StoreInFile = true;
bool doPlot = true;
bool splitCanvas = false;
bool onlyCutIndex = false;
string inDir   = "OUTNew/";
string jsonFile = "../../data/beauty-samples.json";
string outDir  = "Img/";
std::vector<string> plotExt;
string outFile = "plotter.root";
string cutflowhisto = "all_cutflow";
bool forceMerge=false;
bool useMerged=false;
bool jodorStyle=false;
bool generatePseudoData=false;
TH1* myPseudoData=NULL;
TString myPseudoDataName="";

Double_t ptbins[20] = { 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 120., 140., 160., 180., 200., 250., 300., 400., 500.};

class TauDiscriminatorSet {
  
public:














  TauDiscriminatorSet(TString name, TString fancy):
    name_(name),
    fancy_(fancy)
  {     
    if(name_=="CombinedIsolationDeltaBetaCorr3Hits")
      {
        out_="combIsoDB3Hits";
        wp_.push_back("Loose"); colour_.push_back(TColor::GetColor("#009900"));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");colour_.push_back(TColor::GetColor("#ff6600"));marker_.push_back(26);dataMarker_.push_back(22);;
        wp_.push_back("Tight"); colour_.push_back(TColor::GetColor("#990099"));marker_.push_back(32);dataMarker_.push_back(23);
      }
    else if(name_=="IsolationMVArun2v1DBdR03oldDMwLT")
      {
        out_="MVDB03oldDM";
        wp_.push_back("VLoose");  colour_.push_back(TColor::GetColor(102, 153, 255));marker_.push_back(24);dataMarker_.push_back(20);
        wp_.push_back("Loose");   colour_.push_back(TColor::GetColor(  0, 153,   0));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");  colour_.push_back(TColor::GetColor(255, 102,   0));marker_.push_back(26);dataMarker_.push_back(22);
        wp_.push_back("Tight");   colour_.push_back(TColor::GetColor(153,   0, 153));marker_.push_back(32);dataMarker_.push_back(23);
        wp_.push_back("VTight");  colour_.push_back(TColor::GetColor( 51,  51, 255));marker_.push_back(27);dataMarker_.push_back(33);
        wp_.push_back("VVTight"); colour_.push_back(TColor::GetColor(127, 127, 127));marker_.push_back(30);dataMarker_.push_back(29);
          
      }
    else if(name_=="IsolationMVArun2v1DBnewDMwLT")
      {
        out_="mvaDBnewDM";
        wp_.push_back("VLoose");   colour_.push_back(TColor::GetColor(102, 153, 255));marker_.push_back(24);dataMarker_.push_back(20);
        wp_.push_back("Loose");    colour_.push_back(TColor::GetColor(  0, 153,   0));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");   colour_.push_back(TColor::GetColor(255, 102,   0));marker_.push_back(26);dataMarker_.push_back(22);
        wp_.push_back("Tight");    colour_.push_back(TColor::GetColor(153,   0, 153));marker_.push_back(32);dataMarker_.push_back(23);
        wp_.push_back("VTight");   colour_.push_back(TColor::GetColor( 51,  51, 255));marker_.push_back(27);dataMarker_.push_back(33);
        wp_.push_back("VVTight");  colour_.push_back(TColor::GetColor(127, 127, 127));marker_.push_back(30);dataMarker_.push_back(29);
      }
    else if(name_=="IsolationMVArun2v1DBoldDMwLT")
      {
        out_="mvaDBoldDM";
        wp_.push_back("VLoose");  colour_.push_back(TColor::GetColor(102, 153, 255));marker_.push_back(24);dataMarker_.push_back(20);
        wp_.push_back("Loose");   colour_.push_back(TColor::GetColor(  0, 153,   0));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");  colour_.push_back(TColor::GetColor(255, 102,   0));marker_.push_back(26);dataMarker_.push_back(22);
        wp_.push_back("Tight");   colour_.push_back(TColor::GetColor(153,   0, 153));marker_.push_back(32);dataMarker_.push_back(23);
        wp_.push_back("VTight");  colour_.push_back(TColor::GetColor( 51,  51, 255));marker_.push_back(27);dataMarker_.push_back(33);
        wp_.push_back("VVTight"); colour_.push_back(TColor::GetColor(127, 127, 127));marker_.push_back(30);dataMarker_.push_back(29);
      }
    else if(name_=="IsolationMVArun2v1PWdR03oldDMwLT")
      {
        out_="mvaPW03oldDM";
        wp_.push_back("VLoose");   colour_.push_back(TColor::GetColor(102, 153, 255));marker_.push_back(24);dataMarker_.push_back(20);
        wp_.push_back("Loose");    colour_.push_back(TColor::GetColor(  0, 153,   0));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");   colour_.push_back(TColor::GetColor(255, 102,   0));marker_.push_back(26);dataMarker_.push_back(22);
        wp_.push_back("Tight");    colour_.push_back(TColor::GetColor(153,   0, 153));marker_.push_back(32);dataMarker_.push_back(23);
        wp_.push_back("VTight");   colour_.push_back(TColor::GetColor( 51,  51, 255));marker_.push_back(27);dataMarker_.push_back(33);
        wp_.push_back("VVTight");  colour_.push_back(TColor::GetColor(127, 127, 127));marker_.push_back(30);dataMarker_.push_back(29);
      }
    else if(name_=="IsolationMVArun2v1PWnewDMwLT")
      {
        out_="mvaPWnewDM";
        wp_.push_back("VLoose");   colour_.push_back(TColor::GetColor(102, 153, 255));marker_.push_back(24);dataMarker_.push_back(20);
        wp_.push_back("Loose");    colour_.push_back(TColor::GetColor(  0, 153,   0));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");   colour_.push_back(TColor::GetColor(255, 102,   0));marker_.push_back(26);dataMarker_.push_back(22);
        wp_.push_back("Tight");    colour_.push_back(TColor::GetColor(153,   0, 153));marker_.push_back(32);dataMarker_.push_back(23);
        wp_.push_back("VTight");   colour_.push_back(TColor::GetColor( 51,  51, 255));marker_.push_back(27);dataMarker_.push_back(33);
        wp_.push_back("VVTight");  colour_.push_back(TColor::GetColor(127, 127, 127));marker_.push_back(30);dataMarker_.push_back(29);
      }
    else if(name_=="IsolationMVArun2v1PWoldDMwLT")
      {
        out_="mvaPWoldDM";
        wp_.push_back("VLoose");   colour_.push_back(TColor::GetColor(102, 153, 255));marker_.push_back(24);dataMarker_.push_back(20);
        wp_.push_back("Loose");    colour_.push_back(TColor::GetColor(  0, 153,   0));marker_.push_back(25);dataMarker_.push_back(21);
        wp_.push_back("Medium");   colour_.push_back(TColor::GetColor(255, 102,   0));marker_.push_back(26);dataMarker_.push_back(22);
        wp_.push_back("Tight");    colour_.push_back(TColor::GetColor(153,   0, 153));marker_.push_back(32);dataMarker_.push_back(23);
        wp_.push_back("VTight");   colour_.push_back(TColor::GetColor( 51,  51, 255));marker_.push_back(27);dataMarker_.push_back(33);
        wp_.push_back("VVTight");  colour_.push_back(TColor::GetColor(127, 127, 127));marker_.push_back(30);dataMarker_.push_back(29);
      }
    else
      {
        cout << "Discriminator is not supported" << endl;
      }
    
  };
  
  ~TauDiscriminatorSet(){};

  TString name()             { return name_;                       };
  size_t nwp()               { return wp_.size();                  };
  TString wp(size_t i)       { return TString("by")+wp_[i]+name_;  };
  Int_t colour(size_t i)     { return colour_[i];                  };
  Int_t marker(size_t i)     { return marker_[i];                  };
  Int_t dataMarker(size_t i) { return dataMarker_[i];              };  
  TString fancy(size_t i)    { return fancy_+TString(" ")+wp_[i];  };
  TString out()              { return out_;                        };
private:
  
  TString name_;
  std::vector<TString> wp_;
  std::vector<Int_t> colour_;
  std::vector<Int_t> marker_;
  std::vector<Int_t> dataMarker_;
  TString fancy_;
  TString out_;

};

typedef std::vector<TauDiscriminatorSet*> TauDiscriminatorSetCollection;


// data (single mu)
// TTbar
// Top ---> no.
// W#rightarrow l#nu
// W#rightarrow l#nu, HT>100
// QCD, HT>100
// QCD_EMEnr
// #gamma+jets;1#gamma+jets

class FakeRateAnalysis {
public:
  FakeRateAnalysis(TString name, bool doTheData):
    name_(name),
    doData_(doTheData)
  {
      if(name_ == "wjet")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step5");
          rawname_=name_;
        }
      else if(name_ == "wjet_wonly")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step5");
          rawname_="wjet";
        }
      else if(name_ == "wjet_tonly")
        {
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step5");
          rawname_="wjet";
        }
      else if(name_ == "wjetnob")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step6");
          rawname_="wjet";
        }
      else if(name_ == "wjetnob_wonly")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step6");
          rawname_="wjet";
        }
      else if(name_ == "wjetnob_tonly")
        {
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step6");
          rawname_="wjet";
        }
      else if(name_ == "wjetnoblepveto")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step7");
          rawname_="wjet";
        }
      else if(name_ == "wjetnoblepveto_wonly")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step7");
          rawname_="wjet";
        }
      else if(name_ == "wjetnoblepveto_tonly")
        {
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step7");
          rawname_="wjet";
        }
      else if(name_ == "wjetnoblepjetveto")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step8");
          rawname_="wjet";
        }
      else if(name_ == "wjetnoblepjetveto_wonly")
        {
          sample_.push_back("W#rightarrow l#nu");
          sample_.push_back("QCD_EMEnr");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step8");
          rawname_="wjet";
        }
      else if(name_ == "wjetnoblepjetveto_tonly")
        {
          sample_.push_back("TTbar");
          if(doData_) data_ = "data (single mu)";
          step_=TString("step8");
          rawname_="wjet";
        }
      else if(name_ == "qcd")
        {
          sample_.push_back("TTbar");
          sample_.push_back("QCD, HT>100");
          if(doData_) data_ = "JetHT data";
          step_=TString("step3");
          rawname_=name_;
        }
      else if(name_ == "qcd_qonly")
        {
          sample_.push_back("QCD, HT>100");
          if(doData_) data_ = "JetHT data";
          step_=TString("step3");
          rawname_="qcd";
        }
      else if(name_ == "qcd_tonly")
        {
          sample_.push_back("TTBar");
          if(doData_) data_ = "JetHT data";
          step_=TString("step3");
          rawname_="qcd";
        }
      else
        {
          cout << "Analysis type not supported" << endl;
        }

  };

  ~FakeRateAnalysis(){};
  
  TString name()                { return name_;          };
  TString rawname()             { return rawname_;       };
  TString step()                { return step_;          };
  size_t nsamples()             { return sample_.size(); };
  TString sample(size_t i)      { return sample_[i];     };
  TString data()                { return data_;          };

private:
  TString name_;
  TString rawname_;
  std::vector<TString> sample_;
  TString step_;
  bool doData_;
  TString data_;
};

typedef std::vector<FakeRateAnalysis*> FakeRateAnalysisCollection;


class FakesVariable {
public:
  FakesVariable(TString name, Int_t rebin):
    name_(name),
    rebin_(rebin)
  {
  };
  ~FakesVariable(){};

  TString name(){ return name_;  };
  Int_t rebin() { return rebin_; };

private:
  TString name_;
  Int_t rebin_;
};

typedef std::vector<FakesVariable*> FakesVariableCollection;

int main (int argc, char *argv[])
{
  
  //setTDRStyle();  
  //gStyle->SetPadTopMargin   (0.06);
  //gStyle->SetPadBottomMargin(0.12);
  //gStyle->SetPadRightMargin (0.16);
  //gStyle->SetPadLeftMargin  (0.14);
  //gStyle->SetTitleSize(0.04, "XYZ");
  //gStyle->SetTitleXOffset(1.1);
  //gStyle->SetTitleYOffset(1.45);
  //gStyle->SetPalette(1);
  //gStyle->SetNdivisions(505);

  gErrorIgnoreLevel = 3000;

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  TH1::SetDefaultSumw2();
  

  for(int i=1;i<argc;i++){
    string arg(argv[i]);
    //printf("--- %i - %s\n",i,argv[i]);
    
    if(arg.find("--help")!=string::npos){
      printf("--help   --> print this helping text\n");

      printf("--doData  --> process data = true by default\n");
      
      printf("--iLumi   --> integrated luminosity to be used for the MC rescale\n");
      printf("--iEcm    --> center of mass energy (TeV) = 8 TeV by default\n");
      printf("--isSim   --> print CMS Simulation instead of the standard title\n");
      printf("--inDir   --> path to the directory containing the .root files to process\n");
      printf("--outDir  --> path of the directory that will contains the output plots and tables\n");
      printf("--outFile --> path of the output summary .root file\n");
      printf("--json    --> containing list of process (and associated style) to process to process\n");
      printf("--onlyContain    --> processing only the objects containing the following argument in their name\n");
      printf("--onlyStartWith  --> processing only the objects starting with the following argument in their name\n");
      printf("--index   --> will do the projection on that index for histos of type cutIndex\n");
      printf("--chi2    --> show the data/MC chi^2\n"); 
      printf("--showUnc --> show stat uncertainty (if number is given use it as relative bin by bin uncertainty (e.g. lumi)\n"); 
      printf("--noLog   --> use linear scale\n");
      printf("--logX    --> use log scale on X\n");
      printf("--no1D   --> Skip processing of 1D objects\n");
      printf("--no2D   --> Skip processing of 2D objects\n");
      printf("--noTex  --> Do not create latex table (when possible)\n");
      printf("--noPowers --> Do not use powers of 10 for numbers in tables\n");
      printf("--noRoot --> Do not make a summary .root file\n");
      printf("--noPlot --> Do not creates plot files (useful to speedup processing)\n");
      printf("--plotExt --> extension to save\n");
      printf("--cutflow --> name of the histogram with the original number of events (cutflow by default)\n");
      printf("--splitCanvas --> (only for 2D plots) save all the samples in separated pltos\n");
      printf("--forceMerge --> merge splitted samples\n");
      printf("--useMerged --> use merged splitted samples\n");
      printf("--jodorStyle --> use plotting style requested from Jodor");
      printf("--generatePseudoData --> generate pseudo-data by poisson-smearing the total SM MC distribution");
      
      printf("command line example: runPlotter --json ../data/beauty-samples.json --iLumi 2007 --inDir OUT/ --outDir OUT/plots/ --outFile plotter.root --noRoot --noPlot\n");
      return 0;
    }
    
    string sdodata("");
    if(arg.find("--doData" )!=string::npos && i+1<argc){ sdodata = argv[i+1]; i++; if(sdodata=="true") doData=true; if(sdodata=="false") doData=false; }
    printf(doData? "Processing data sample" : "Not processing data sample (run MonteCarlo simulation only)");
    if(arg.find("--iLumi"  )!=string::npos && i+1<argc){ sscanf(argv[i+1],"%lf",&iLumi); i++; printf("Lumi = %f\n", iLumi); }
    if(arg.find("--iEcm"  )!=string::npos && i+1<argc){ sscanf(argv[i+1],"%lf",&iEcm); i++; printf("Ecm = %f TeV\n", iEcm); }
    
    if(arg.find("--inDir"  )!=string::npos && i+1<argc){ inDir    = argv[i+1];  i++;  printf("inDir = %s\n", inDir.c_str());  }
    if(arg.find("--outDir" )!=string::npos && i+1<argc){ outDir   = argv[i+1];  i++;  printf("outDir = %s\n", outDir.c_str());  }
    if(arg.find("--outFile")!=string::npos && i+1<argc){ outFile  = argv[i+1];  i++; printf("output file = %s\n", outFile.c_str()); }
    if(arg.find("--json"   )!=string::npos && i+1<argc){ jsonFile = argv[i+1];  i++;  }
    if(arg.find("--index"  )!=string::npos && i+1<argc)         { sscanf(argv[i+1],"%d",&cutIndex); i++; onlyCutIndex=(cutIndex>=0); printf("index = %i\n", cutIndex);  }
    if(arg.find("--chi2"  )!=string::npos)                      { showChi2 = true;  }
    if(arg.find("--showUnc") != string::npos) { 
      showUnc=true; 
      if(i+1<argc) { 
        string nextArg(argv[i+1]); 
        if(nextArg.find("--")==string::npos)
          {
            sscanf(argv[i+1],"%lf",&baseRelUnc);
            i++;
          }
      }
      printf("Uncertainty band will be included for MC with base relative uncertainty of: %3.2f",baseRelUnc);
     }
    if(arg.find("--isSim")!=string::npos){ isSim = true;    }
    if(arg.find("--forceMerge")!=string::npos){ forceMerge = true;  useMerged=true;  }
    if(arg.find("--useMerged")!=string::npos){ useMerged = true;    }
    if(arg.find("--jodorStyle")!=string::npos){ jodorStyle = true;  }
    if(arg.find("--generatePseudoData")!=string::npos){ generatePseudoData = true; }
    if(arg.find("--noLog")!=string::npos){ noLog = true;    }
    if(arg.find("--logX")!=string::npos){ logX = true;    }
    if(arg.find("--no2D"  )!=string::npos){ do2D = false;    }
    if(arg.find("--no1D"  )!=string::npos){ do1D = false;    }
    if(arg.find("--noTex" )!=string::npos){ doTex= false;    }
    if(arg.find("--noPowers" )!=string::npos){ doPowers= false;    }
    if(arg.find("--noRoot")!=string::npos){ StoreInFile = false;    }
    if(arg.find("--noPlot")!=string::npos){ doPlot = false;    }
    if(arg.find("--debug")!=string::npos){ debug = true;    }
    if(arg.find("--plotExt" )!=string::npos && i+1<argc){ plotExt.push_back(argv[i+1]);  i++;  printf("saving plots as = %s\n", plotExt[plotExt.size()-1].c_str());  }
    if(arg.find("--cutflow" )!=string::npos && i+1<argc){ cutflowhisto   = argv[i+1];  i++;  printf("Normalizing from 1st bin in = %s\n", cutflowhisto.c_str());  }
    if(arg.find("--splitCanvas")!=string::npos){ splitCanvas = true;    }
  }
  

  TauDiscriminatorSetCollection discriminators;
  
  discriminators.push_back( new TauDiscriminatorSet("CombinedIsolationDeltaBetaCorr3Hits", "HPS #delta#beta 3-hit" ) );
  discriminators.push_back( new TauDiscriminatorSet("IsolationMVArun2v1DBdR03oldDMwLT"   , "MVA DB dR03 oldDM"     ) );
  discriminators.push_back( new TauDiscriminatorSet("IsolationMVArun2v1DBnewDMwLT"       , "MVA DB newDM"          ) );
  discriminators.push_back( new TauDiscriminatorSet("IsolationMVArun2v1DBoldDMwLT"       , "MVA DB oldDM"          ) ); 
  discriminators.push_back( new TauDiscriminatorSet("IsolationMVArun2v1PWdR03oldDMwLT"   , "MVA PW dR03 oldDM"     ) );
  discriminators.push_back( new TauDiscriminatorSet("IsolationMVArun2v1PWnewDMwLT"       , "MVA PW newDM"          ) );
  discriminators.push_back( new TauDiscriminatorSet("IsolationMVArun2v1PWoldDMwLT"       , "MVA PW oldDM"          ) );
  
  FakeRateAnalysisCollection analyses;

  // Quark-jets selection
  //analyses.push_back( new FakeRateAnalysis("wjet"      , doData) );
  
  // Gluon-jets selection
  analyses.push_back( new FakeRateAnalysis("qcd"       , doData) );

  analyses.push_back( new FakeRateAnalysis("qcd_qonly", doData) ); // Compute fakes for wjets MC only
  analyses.push_back( new FakeRateAnalysis("qcd_tonly", doData) ); // Compute fakes for ttbar MC only

  //analyses.push_back( new FakeRateAnalysis("wjet_wonly", doData) ); // Compute fakes for wjets MC only
  //analyses.push_back( new FakeRateAnalysis("wjet_tonly", doData) ); // Compute fakes for ttbar MC only

  //analyses.push_back( new FakeRateAnalysis("wjetnob", doData) ); // Compute fakes for wjets MC only
  //analyses.push_back( new FakeRateAnalysis("wjetnob_tonly", doData) ); // Compute fakes for ttbar MC only
  //analyses.push_back( new FakeRateAnalysis("wjetnob_wonly", doData) ); // Compute fakes for wjets MC only


  //analyses.push_back( new FakeRateAnalysis("wjetnoblepveto", doData) ); // Compute fakes for wjets MC only
  //analyses.push_back( new FakeRateAnalysis("wjetnoblepveto_tonly", doData) ); // Compute fakes for ttbar MC only
  //analyses.push_back( new FakeRateAnalysis("wjetnoblepveto_wonly", doData) ); // Compute fakes for wjets MC only
  
  //analyses.push_back( new FakeRateAnalysis("wjetnoblepjetveto", doData) ); // Compute fakes for wjets MC only
  //analyses.push_back( new FakeRateAnalysis("wjetnoblepjetveto_tonly", doData) ); // Compute fakes for ttbar MC only
  //analyses.push_back( new FakeRateAnalysis("wjetnoblepjetveto_wonly", doData) ); // Compute fakes for wjets MC only
  

  FakesVariableCollection vars;
  vars.push_back( new FakesVariable("pt"     , 1) );
  vars.push_back( new FakesVariable("met"    , 1) );
  vars.push_back( new FakesVariable("recomet", 1) );
  vars.push_back( new FakesVariable("eta"    , 0) );
  vars.push_back( new FakesVariable("radius" , 0) );
  vars.push_back( new FakesVariable("nvtx"   , 0) );

  
  for(FakeRateAnalysisCollection::iterator ianal=analyses.begin(); ianal!=analyses.end(); ++ianal)
    {
      FakeRateAnalysis* anal = *ianal;
      cout << "Processing anal: " << anal->name();
      TFile* f = TFile::Open(inDir+TString("/plotter_")+anal->rawname()+TString(".root"));
      cout << " using file " << inDir << "/plotter_" << anal->rawname() << ".root" << endl;
      
      for(TauDiscriminatorSetCollection::iterator idiscr=discriminators.begin(); idiscr!=discriminators.end(); ++idiscr)
        {
          TauDiscriminatorSet* discr = *idiscr;

          if(debug) cout << "\t Processing discriminator: " << discr->name() << endl;

          for(FakesVariableCollection::iterator ivar=vars.begin(); ivar!=vars.end(); ++ivar)
            {
              FakesVariable* var = *ivar;
              if(debug) cout << "\t \t Processing variable: " << var->name() << endl;
              
              vector<TH1*> numerator;
              TH1* denominator = NULL;
              
              vector<TH1*> data_numerator;
              TH1* data_denominator = NULL;
              
              for(size_t isample = 0; isample<anal->nsamples(); ++isample)
                {
                  TString sample(anal->sample(isample));
                  if(debug) cout << "\t \t \t Processing sample: " << sample  << ", name: " << sample+TString("/")+anal->rawname()+TString("_")+anal->step()+var->name()+TString("_denominator") << endl;
                  // Denominator is common (independent on tauID)
                  if(!denominator)
                    denominator = (TH1*)     f->Get(sample+TString("/")+anal->rawname()+TString("_")+anal->step()+var->name()+TString("_denominator"));
                  else
                    denominator ->Add((TH1*) f->Get(sample+TString("/")+anal->rawname()+TString("_")+anal->step()+var->name()+TString("_denominator"))); 
                
                }
              
              if(doData)
                data_denominator = (TH1*) f->Get(anal->data()+TString("/")+anal->rawname()+TString("_")+anal->step()+var->name()+TString("_denominator"));     
              
              for(size_t l=0; l<discr->nwp(); ++l) // Loop on working points
                {
                  TString tcat(discr->wp(l));
                  if(debug) cout << "\t \t \t Processing working point: " << tcat << endl;
                  Int_t colour(discr->colour(l));
                  Int_t marker(discr->marker(l));
                  Int_t dataMarker(discr->dataMarker(l));

                  TH1* temp_numerator     = NULL;
                  TH1* data_temp_numerator     = NULL;
                  
                  for(size_t isample = 0; isample<anal->nsamples(); ++isample)
                    {
                      TString sample(anal->sample(isample));
                      if(debug) cout << "\t \t \t \t Processing sample: " << sample << ", name: " << sample+TString("/")+anal->rawname()+TString("_")+anal->step()+tcat+var->name()+TString("_numerator") << endl;
                      if(!temp_numerator)
                        temp_numerator = (TH1*)     f->Get(sample+TString("/")+anal->rawname()+TString("_")+anal->step()+tcat+var->name()+TString("_numerator"));    
                      else
                        temp_numerator ->Add((TH1*) f->Get(sample+TString("/")+anal->rawname()+TString("_")+anal->step()+tcat+var->name()+TString("_numerator"))); 
                    }
                  
                  if(!temp_numerator) cout << "temp_numerator is NULL" << endl;          
                  
                  temp_numerator    ->Sumw2();          
                  
                  if(var->rebin())
                    {
                      TH1* temp_binned = temp_numerator    ->Rebin(19, "", ptbins);          
                      temp_numerator=temp_binned;
                      //temp_numerator   ->Rebin(var->rebin());
                    }
                  
                  temp_numerator->SetMarkerColor(colour);
                  temp_numerator->SetLineColor(colour);
                  temp_numerator->SetLineWidth(2);
                  temp_numerator->SetMarkerStyle(marker);
                  temp_numerator->SetMarkerSize(1.5);
                  
                  numerator.push_back(temp_numerator);
                  
                  // Data
                  if(doData)
                    {
                      data_temp_numerator     = (TH1*) f->Get(anal->data()+TString("/")+anal->rawname()+TString("_")+anal->step()+tcat+var->name()+TString("_numerator"));
                 
                      data_temp_numerator    ->Sumw2();          
                  
                      if(var->rebin())
                        {                      
                          TH1* data_temp_binned = data_temp_numerator->Rebin(19, "", ptbins);          
                          data_temp_numerator=data_temp_binned;
                          //data_temp_numerator   ->Rebin(var->rebin());
                        }
                      
                      data_temp_numerator->SetMarkerColor(colour);
                      data_temp_numerator->SetLineColor(colour);
                      data_temp_numerator->SetLineWidth(2);
                      data_temp_numerator->SetMarkerStyle(dataMarker);
                      data_temp_numerator->SetMarkerSize(1.5);

                      data_numerator.push_back(data_temp_numerator);
                    }
                }
              
          
              if(!denominator) cout << "Denominator is NULL" << endl; else denominator->Sumw2();
              
              if(doData)
                {
                  if(!data_denominator)
                    cout << "Denominator is NULL" << endl;
                  else
                    data_denominator->Sumw2();
                }

              if(var->rebin())
                {
                  TH1* binned = denominator->Rebin(19, "", ptbins);
                  denominator=binned;
                  //denominator->Rebin(var->rebin());
                  
                  TH1* data_binned = NULL;
                  if(doData)
                    {
                      data_binned = data_denominator->Rebin(19, "", ptbins);
                      data_denominator=data_binned;
                      //data_denominator->Rebin(var->rebin());
                    }
                }
              for(size_t l=0; l<discr->nwp(); ++l) // Loop on working points   
                {
                  numerator[l]->Divide(denominator);
                  numerator[l]->GetYaxis()->SetTitle("Fake rate");
              
                  numerator[l]->Sumw2();
                  numerator[l]->SetMaximum(1.);
                  numerator[l]->SetMinimum(0.0001);
                  
                  if(doData)
                    data_numerator[l]->Divide(data_denominator);
                }
              gStyle->SetLegendBorderSize(0);
              TLegend* leg = new TLegend(0.5, 0.75, 0.89, 0.9);
              TLegendEntry* le = NULL;
              for(size_t l=0; l<discr->nwp(); ++l)
                {
                  if(false && discr->wp(l).Contains("Medium"))
                    {
                      le = leg->AddEntry(numerator[l], discr->fancy(l), "pl");
                      le->SetTextColor(kRed);
                      le->SetTextSize(0.04);
                    }
                  else
                    leg->AddEntry(numerator[l], discr->fancy(l), "pl");
                }
              leg->SetHeader("");
              
              TText* t = new TText(0.45, 0.7, "Full(empty) markers: data(MonteCarlo)"); t->SetNDC();
              t->SetTextSize(0.03);
              
              for(int i=0; i<2; ++i)
                {
                  TCanvas* c = new TCanvas("fakerate", "fakerate", 800, 800);
                  c->cd();
                  gPad->SetLogy();
                  for(size_t l=0; l<discr->nwp(); ++l)
                    {
                      numerator[l]->GetXaxis()->SetNdivisions(509,true);
                      if(
                         var->name() == "pt"      ||
                         var->name() == "recomet" ||
                         var->name() == "met"
                         )
                        {
                          numerator[l]->GetXaxis()->SetRangeUser(20.,499.9);
                          if(doData) data_numerator[l]->GetXaxis()->SetRangeUser(20.,499.9);
                        }
                      if(var->name() == "radius")
                        {
                          numerator[l]->GetXaxis()->SetRangeUser(0.,0.3);
                          if(doData) data_numerator[l]->GetXaxis()->SetRangeUser(0.,0.3);
                        }
                      l==0 ? numerator[l]->Draw("") : numerator[l]->Draw("same");
                      if(i>0 && doData)
                        {
                          data_numerator[l]->Draw("same");
                          t->Draw("");
                        }
                    }
                  leg->Draw("");
                  c->Modified();
                  c->Update();
                  gSystem->Exec(TString("mkdir -p devel/")+discr->out());
                  gSystem->Exec(TString("cp ${HOME}/www/HIG-13-026/index.php devel/")+discr->out()+TString("/"));
                  c->Print(TString("devel/")+discr->out()+TString("/fakes_")+var->name()+TString("_")+(i>0?TString("data_"):TString(""))+anal->name()+TString(".pdf"));
                  c->Print(TString("devel/")+discr->out()+TString("/fakes_")+var->name()+TString("_")+(i>0?TString("data_"):TString(""))+anal->name()+TString(".png"));
                } 
            } // End loop on FakesVariableCollection
        } // End loop on TauDiscriminatorSetCollection
    } // End loop on FakeRateAnalysisCollection
  gSystem->Exec(TString("mkdir -p ")+inDir);
  gSystem->Exec(TString("rm -r ")+inDir+TString("/")+outDir);
  gSystem->Exec("cp ${HOME}/www/HIG-13-026/index.php devel/");
  gSystem->Exec(TString("mv devel/ ")+inDir+TString("/")+outDir);
  
  exit(0);  
}
