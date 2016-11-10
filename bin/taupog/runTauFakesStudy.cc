//
// Pietro Vischia, <pietro.vischia@gmail.com>
//
// Charged Higgs analysis
//

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/ChainEvent.h"
#include "DataFormats/Common/interface/MergeableCounter.h"

//Load here all the dataformat that we will need
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "GeneratorInterface/LHEInterface/interface/LHEEvent.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/PatCandidates/interface/GenericParticle.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"
//#include "TauAnalysis/SVfitStandalone/interface/SVfitStandaloneAlgorithm.h" //for svfit

#include "UserCode/llvv_fwk/interface/MacroUtils.h"
#include "UserCode/llvv_fwk/interface/HiggsUtils.h"
#include "UserCode/llvv_fwk/interface/SmartSelectionMonitor.h"
#include "UserCode/llvv_fwk/interface/TMVAUtils.h"
#include "UserCode/llvv_fwk/interface/LeptonEfficiencySF.h"
#include "UserCode/llvv_fwk/interface/PDFInfo.h"
//#include "TauAnalysis/JetToTauFakeRate/interface/MuScleFitCorrector.h"
#include "UserCode/llvv_fwk/interface/rochcor2016.h"
#include "UserCode/llvv_fwk/interface/BTagCalibrationStandalone.h"
#include "UserCode/llvv_fwk/interface/BtagUncertaintyComputer.h"
#include "UserCode/llvv_fwk/interface/GammaWeightsHandler.h"

#include "UserCode/llvv_fwk/interface/PatUtils.h"


#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TEventList.h"
#include "TROOT.h"
#include "TNtuple.h"
#include <Math/VectorUtil.h>

using namespace std;



bool hasStableLeptonAsDaughter(const reco::GenParticle p)
{
  bool foundL(false);
  if(p.numberOfDaughters()==0) return foundL;

  // cout << "Particle " << p.pdgId() << " with status " << p.status() << " and " << p.numberOfDaughters() << endl;
  const reco::Candidate *part = &p;
  // loop on the daughter particles to check if it has an e/mu as daughter
  while ((part->numberOfDaughters()>0)) {
    const reco::Candidate* DaughterPart = part->daughter(0);
    // cout << "\t\t Daughter: " << DaughterPart->pdgId() << " with status " << DaughterPart->status() << endl;
    if (fabs(DaughterPart->pdgId()) == 11 || fabs(DaughterPart->pdgId() == 13)){
      if(DaughterPart->status() == 1 || DaughterPart->status() == 2 ){
        foundL = true;
        break;
      }
    }
    part=DaughterPart;
  }
  return foundL;
}


bool hasWasMother(const reco::GenParticle  p)
{
  bool foundW(false);
  if(p.numberOfMothers()==0) return foundW;
  const reco::Candidate* part =&p; // (p.mother());
  // loop on the mother particles to check if it has a W as mother
  while ((part->numberOfMothers()>0)) {
    const reco::Candidate* MomPart =part->mother();
    if (fabs(MomPart->pdgId())==24){
      foundW = true;
      break;
    }
    part = MomPart;
  }
  return foundW;
}

bool hasTauAsMother(const reco::GenParticle  p)
{
  bool foundTau(false);
  if(p.numberOfMothers()==0) return foundTau;
  const reco::Candidate* part = &p; //(p.mother());
  // loop on the mother particles to check if it has a tau as mother
  while ((part->numberOfMothers()>0)) {
    const reco::Candidate* MomPart =part->mother();
    if (fabs(MomPart->pdgId())==15)// && MomPart->status() == 2) // Not sure the status check is needed.
      {
        foundTau = true;
        break;
      }
    part = MomPart;
  }
  return foundTau;
}

int main (int argc, char *argv[])
{
  //##############################################
  //########    GLOBAL INITIALIZATION     ########
  //##############################################

  // check arguments
  if (argc < 2)
    {
      std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl;
      exit (0);
    }
  
  // load framework libraries
  gSystem->Load ("libFWCoreFWLite");
  AutoLibraryLoader::enable ();
  
  // configure the process
  const edm::ParameterSet & runProcess = edm::readPSetsFrom (argv[1])->getParameter < edm::ParameterSet > ("runProcess");

  bool debug      = runProcess.getParameter<bool>  ("debug");
  bool isMC       = runProcess.getParameter<bool>  ("isMC");
  double xsec     = runProcess.getParameter<double>("xsec");
  int mctruthmode = runProcess.getParameter<int>   ("mctruthmode");
  TString dtag    = runProcess.getParameter<std::string>("dtag");

  TString suffix = runProcess.getParameter < std::string > ("suffix");
  std::vector < std::string > urls = runProcess.getUntrackedParameter < std::vector < std::string > >("input");
//  TString baseDir = runProcess.getParameter < std::string > ("dirName");
//  TString url = TString (argv[1]);
//  TString outFileUrl (gSystem->BaseName (url));
//  outFileUrl.ReplaceAll ("_cfg.py", "");
//  if (mctruthmode != 0)
//    {
//      outFileUrl += "_filt";
//      outFileUrl += mctruthmode;
//    }
  TString outUrl = runProcess.getParameter<std::string> ("outfile");

  // Good lumi mask
  lumiUtils::GoodLumiFilter goodLumiFilter(runProcess.getUntrackedParameter<std::vector<edm::LuminosityBlockRange> >("lumisToProcess", std::vector<edm::LuminosityBlockRange>()));

  
  bool
    filterOnlyJETHT    (false),
    filterOnlySINGLEMU (false);
  if (!isMC)
    {
      if (dtag.Contains ("JetHT"))       filterOnlyJETHT    = true;
      if (dtag.Contains ("SingleMuon"))  filterOnlySINGLEMU = true;
    }
  
  bool isSingleMuPD (!isMC && dtag.Contains ("SingleMu")); // Do I really need this?
  bool isV0JetsMC   (isMC && (dtag.Contains ("DYJetsToLL_50toInf") || dtag.Contains ("WJets")));
  bool isControlWJets (isMC && dtag.Contains("W_Jets"));
  bool isPromptReco (!isMC && dtag.Contains("Run2015B-PromptReco"));
  bool isNLOMC      (isMC && (TString(urls[0]).Contains("amcatnlo") || TString(urls[0]).Contains("powheg")) );

  if(debug) cout << "isNLOMC: " << isNLOMC << ", for dtag: " << dtag << endl;

  TString outTxtUrl = outUrl + ".txt";
  FILE *outTxtFile = NULL;
  if (!isMC) outTxtFile = fopen (outTxtUrl.Data (), "w");
  printf ("TextFile URL = %s\n", outTxtUrl.Data ());
  
  //tree info
  TString dirname = runProcess.getParameter < std::string > ("dirName");
  
  //systematics
  bool runSystematics = runProcess.getParameter < bool > ("runSystematics");
  std::vector < TString > varNames (1, "");
  if (runSystematics)
    {
      varNames.push_back ("_jerup" ); varNames.push_back ("_jerdown" );
      varNames.push_back ("_jesup" ); varNames.push_back ("_jesdown" );
      varNames.push_back ("_umetup"); varNames.push_back ("_umetdown");
      varNames.push_back ("_lesup" ); varNames.push_back ("_lesdown" );
      varNames.push_back ("_puup"  ); varNames.push_back ("_pudown"  );
      varNames.push_back ("_btagup"); varNames.push_back ("_btagdown");
    }
  
  size_t nvarsToInclude = varNames.size ();

  std::vector < std::string > allWeightsURL = runProcess.getParameter < std::vector < std::string > >("weightsFile");
  std::string weightsDir (allWeightsURL.size ()? allWeightsURL[0] : "");
  
  //##############################################
  //########    INITIATING HISTOGRAMS     ########
  //##############################################
  SmartSelectionMonitor mon;

  // ensure proper normalization                                                                                                        
  TH1D* normhist = (TH1D*) mon.addHistogram(new TH1D("initNorm", ";;Nev", 5,0.,5.));
  normhist->GetXaxis()->SetBinLabel (1, "Gen. Events");
  normhist->GetXaxis()->SetBinLabel (2, "Events");
  normhist->GetXaxis()->SetBinLabel (3, "PU central");
  normhist->GetXaxis()->SetBinLabel (4, "PU up");
  normhist->GetXaxis()->SetBinLabel (5, "PU down");

  //event selection
  TH1D* h = (TH1D*) mon.addHistogram (new TH1D ("wjet_eventflow", ";;Events", 8, 0., 8.));
  h->GetXaxis()->SetBinLabel(1, "-");
  h->GetXaxis()->SetBinLabel(2, "#geq 1 vertex");
  h->GetXaxis()->SetBinLabel(3, "1 lepton");
  h->GetXaxis()->SetBinLabel(4, "M_{T}>50~GeV");
  h->GetXaxis()->SetBinLabel(5, "#geq 1 jet");
  h->GetXaxis()->SetBinLabel(6, "#leq 1 b-tags");
  h->GetXaxis()->SetBinLabel(7, "leptons veto");
  h->GetXaxis()->SetBinLabel(8, "#leq 1 hard jets");
  h = (TH1D*) mon.addHistogram (new TH1D ("qcd_eventflow", ";;Events", 6, 0., 6.));
  h->GetXaxis()->SetBinLabel (1, "-");
  h->GetXaxis()->SetBinLabel (2, "#geq 1 vertex");
  h->GetXaxis()->SetBinLabel (3, "#geq 2 jets");
  h->GetXaxis()->SetBinLabel (4, "Lepton veto");
  h->GetXaxis()->SetBinLabel (5, "(Unused)");
  h->GetXaxis()->SetBinLabel (6, "(Unused)");

  mon.addHistogram(new TH1D("lheHt", ";;Events", 50, 0., 1000.));

  // Setting up control categories
  std::vector < TString > controlCats;
  controlCats.clear ();
  controlCats.push_back("step1");
  controlCats.push_back("step2");
  controlCats.push_back("step3");
  controlCats.push_back("step4");
  controlCats.push_back("step5");
  controlCats.push_back("step6");
  controlCats.push_back("step7");
  controlCats.push_back("step8");
  
  std::vector<TString> tauDiscriminators;
  tauDiscriminators.clear();
  
  tauDiscriminators.push_back("byLooseCombinedIsolationDeltaBetaCorr3Hits");
  tauDiscriminators.push_back("byMediumCombinedIsolationDeltaBetaCorr3Hits");
  tauDiscriminators.push_back("byTightCombinedIsolationDeltaBetaCorr3Hits");

  tauDiscriminators.push_back("byVLooseIsolationMVArun2v1DBdR03oldDMwLT");
  tauDiscriminators.push_back("byLooseIsolationMVArun2v1DBdR03oldDMwLT");
  tauDiscriminators.push_back("byMediumIsolationMVArun2v1DBdR03oldDMwLT");
  tauDiscriminators.push_back("byTightIsolationMVArun2v1DBdR03oldDMwLT");
  tauDiscriminators.push_back("byVTightIsolationMVArun2v1DBdR03oldDMwLT");
  tauDiscriminators.push_back("byVVTightIsolationMVArun2v1DBdR03oldDMwLT");

  tauDiscriminators.push_back("byVLooseIsolationMVArun2v1DBnewDMwLT");
  tauDiscriminators.push_back("byLooseIsolationMVArun2v1DBnewDMwLT");
  tauDiscriminators.push_back("byMediumIsolationMVArun2v1DBnewDMwLT");
  tauDiscriminators.push_back("byTightIsolationMVArun2v1DBnewDMwLT");
  tauDiscriminators.push_back("byVTightIsolationMVArun2v1DBnewDMwLT");
  tauDiscriminators.push_back("byVVTightIsolationMVArun2v1DBnewDMwLT");

  tauDiscriminators.push_back("byVLooseIsolationMVArun2v1DBoldDMwLT");
  tauDiscriminators.push_back("byLooseIsolationMVArun2v1DBoldDMwLT");
  tauDiscriminators.push_back("byMediumIsolationMVArun2v1DBoldDMwLT");
  tauDiscriminators.push_back("byTightIsolationMVArun2v1DBoldDMwLT");
  tauDiscriminators.push_back("byVTightIsolationMVArun2v1DBoldDMwLT");
  tauDiscriminators.push_back("byVVTightIsolationMVArun2v1DBoldDMwLT");

  tauDiscriminators.push_back("byVLooseIsolationMVArun2v1PWdR03oldDMwLT");
  tauDiscriminators.push_back("byLooseIsolationMVArun2v1PWdR03oldDMwLT");
  tauDiscriminators.push_back("byMediumIsolationMVArun2v1PWdR03oldDMwLT");
  tauDiscriminators.push_back("byTightIsolationMVArun2v1PWdR03oldDMwLT");
  tauDiscriminators.push_back("byVTightIsolationMVArun2v1PWdR03oldDMwLT");
  tauDiscriminators.push_back("byVVTightIsolationMVArun2v1PWdR03oldDMwLT");

  tauDiscriminators.push_back("byVLooseIsolationMVArun2v1PWnewDMwLT");
  tauDiscriminators.push_back("byLooseIsolationMVArun2v1PWnewDMwLT");
  tauDiscriminators.push_back("byMediumIsolationMVArun2v1PWnewDMwLT");
  tauDiscriminators.push_back("byTightIsolationMVArun2v1PWnewDMwLT");
  tauDiscriminators.push_back("byVTightIsolationMVArun2v1PWnewDMwLT");
  tauDiscriminators.push_back("byVVTightIsolationMVArun2v1PWnewDMwLT");

  tauDiscriminators.push_back("byVLooseIsolationMVArun2v1PWoldDMwLT");
  tauDiscriminators.push_back("byLooseIsolationMVArun2v1PWoldDMwLT");
  tauDiscriminators.push_back("byMediumIsolationMVArun2v1PWoldDMwLT");
  tauDiscriminators.push_back("byTightIsolationMVArun2v1PWoldDMwLT");
  tauDiscriminators.push_back("byVTightIsolationMVArun2v1PWoldDMwLT");
  tauDiscriminators.push_back("byVVTightIsolationMVArun2v1PWoldDMwLT");


  
  for (size_t k = 0; k < controlCats.size (); ++k)
    {
      TString icat(controlCats[k]);

      mon.addHistogram(new TH1D(icat+"pt_denominator", ";p_{T}^{jet};Events", 50, 0., 500.)); // Variable number of bins to be implemented
      mon.addHistogram(new TH1D(icat+"met_denominator", ";E_{T}^{miss};Events", 50, 0., 500.)); // Variable number of bins to be implemented
      mon.addHistogram(new TH1D(icat+"recomet_denominator", ";E_{T}^{miss};Events", 50, 0., 500.)); // Variable number of bins to be implemented
      mon.addHistogram(new TH1D(icat+"eta_denominator", ";#eta_{jet};Events", 25, -2.5, 2.5));
      mon.addHistogram(new TH1D(icat+"radius_denominator", ";R_{jet};Events", 20, 0., 1.));
      mon.addHistogram(new TH1D(icat+"nvtx_denominator", ";N_{vtx};Events", 30, 0., 60.));        
      mon.addHistogram(new TH1D(icat+"nbtags_denominator", ";N_{b-tags};Events", 6, 0., 6.));        
      for(size_t l=0; l<tauDiscriminators.size(); ++l){
        TString tcat(tauDiscriminators[l]);
        mon.addHistogram(new TH1D(icat+tcat+"pt_numerator",   ";p_{T}^{jet};Events", 50, 0., 500.)); // Variable number of bins to be implemented
        mon.addHistogram(new TH1D(icat+tcat+"met_numerator",   ";E_{T}^{miss};Events", 50, 0., 500.)); // Variable number of bins to be implemented
        mon.addHistogram(new TH1D(icat+tcat+"recomet_numerator",   ";E_{T}^{miss};Events", 50, 0., 500.)); // Variable number of bins to be implemented
        mon.addHistogram(new TH1D(icat+tcat+"eta_numerator",   ";#eta_{jet};Events", 25, -2.5, 2.5));
        mon.addHistogram(new TH1D(icat+tcat+"radius_numerator",   ";R_{jet};Events", 20, 0., 1.));
        mon.addHistogram(new TH1D(icat+tcat+"nvtx_numerator",   ";N_{vtx};Events", 30, 0., 60.));
        mon.addHistogram(new TH1D(icat+tcat+"nbtags_numerator", ";N_{b-tags};Events", 6, 0., 6.));        
      }
      
      // Some control plots, mostly on event selection
      mon.addHistogram(new TH1D(icat+"nvtx",    ";Vertices;Events",                        50, 0.,   50.));
      mon.addHistogram(new TH1D(icat+"ptmu",    ";Muon transverse momentum [GeV];Events",  50, 0.,  500.));
      mon.addHistogram(new TH1D(icat+"jetpt",   ";Transverse momentum [GeV];Events",       50, 0., 1000.));
      mon.addHistogram(new TH1D(icat+"met",     ";Missing transverse energy [GeV];Events", 50, 0., 1000.));
      mon.addHistogram(new TH1D(icat+"recoMet", ";Missing transverse energy [GeV];Events", 50, 0., 1000.));
      mon.addHistogram(new TH1D(icat+"mt",      ";Transverse mass;Events",                 50, 0.,  500.));
      mon.addHistogram(new TH1D(icat+"nbtags",  ";N_{b-tags};Events",                       6, 0.,    6.));        


    }                           // End of loop on controlCats

  //
  // STATISTICAL ANALYSIS
  //
  TH1D *Hoptim_systs = (TH1D *) mon.addHistogram (new TH1D ("optim_systs", ";syst;", nvarsToInclude, 0, nvarsToInclude));
  for (size_t ivar = 0; ivar < nvarsToInclude; ivar++) Hoptim_systs->GetXaxis ()->SetBinLabel (ivar + 1, varNames[ivar]);



  //##############################################
  //######## GET READY FOR THE EVENT LOOP ########
  //##############################################

  //fwlite::ChainEvent ev (urls);
  size_t totalEntries(0);// = ev.size ();

  
  //jet energy scale and uncertainties 
  TString jecDir = runProcess.getParameter < std::string > ("jecDir");
  gSystem->ExpandPathName (jecDir);
  FactorizedJetCorrector *jesCor = utils::cmssw::getJetCorrector (jecDir, isMC);
  if(debug) cout << "Jet Corrector created for directory " << jecDir << endl;
  TString pf(isMC ? "MC" : "DATA");
  JetCorrectionUncertainty *totalJESUnc = new JetCorrectionUncertainty ((jecDir + "/"+pf+"_Uncertainty_AK4PFchs.txt").Data());
  if(debug) cout << "Jet CorrectionUncertainty created for directory " << jecDir << endl;
  //muon energy scale and uncertainties
  //MuScleFitCorrector *muCor = getMuonCorrector (jecDir, url);
  rochcor2016* muCor = new rochcor2016(); // Replaces the RunI MuScle fit
  //lepton efficiencies
  LeptonEfficiencySF lepEff;

  //b-tagging: beff and leff must be derived from the MC sample using the discriminator vs flavor
  //the scale factors are taken as average numbers from the pT dependent curves see:
  //https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagPOG#2012_Data_and_MC_EPS13_prescript
  BTagSFUtil btsfutil;
  double beff (0.68), sfb (0.99), sfbunc (0.015);
  double leff (0.13), sfl (1.05), sflunc (0.12);

  // b-tagging working points
  // TODO: in 74X switch to pfCombined.... (based on pf candidates instead of tracks) (recommended)
  // Apparently this V2 has the following preliminary operating points:
  // These preliminary operating points were derived from ttbar events:
  //   - Loose : 0.423 (corresponding to 10.1716% DUSG mistag efficiency)
  //   - Medium : 0.814 (corresponding to 1.0623% DUSG mistag efficiency)
  //   - Tight : 0.941 (corresponding to 0.1144% DUSG mistag efficiency)
  
  // New recommendations for 50ns https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation74X50ns
  //   (pfC|c)ombinedInclusiveSecondaryVertexV2BJetTags
  //      v2CSVv2L 0.605
  //      v2CSVv2M 0.890
  //      v2CSVv2T 0.970
  double
    btagLoose(0.605),
    btagMedium(0.890),
    btagTight(0.970);
  
    BTagCalibration btagCalib("CSVv2", string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/weights/btagSF_CSVv2.csv");
  BTagCalibrationReader btagCal   (&btagCalib, BTagEntry::OP_MEDIUM, "mujets", "central");  // calibration instance, operating point, measurement type, systematics type
  BTagCalibrationReader btagCalUp (&btagCalib, BTagEntry::OP_MEDIUM, "mujets", "up"     );  // sys up
  BTagCalibrationReader btagCalDn (&btagCalib, BTagEntry::OP_MEDIUM, "mujets", "down"   );  // sys down
  BTagCalibrationReader btagCalL  (&btagCalib, BTagEntry::OP_MEDIUM, "comb", "central");  // calibration instance, operating point, measurement type, systematics type
  BTagCalibrationReader btagCalLUp(&btagCalib, BTagEntry::OP_MEDIUM, "comb", "up"     );  // sys up
  BTagCalibrationReader btagCalLDn(&btagCalib, BTagEntry::OP_MEDIUM, "comb", "down"   );  // sys down

  // from Btag SF and eff from https://indico.cern.ch/event/437675/#preview:1629681
  //beff = 0.747; sfb = 0.899; //for Loose WP  //sfb is not actually used as it's taken from btagCal



  //MC normalization (to 1/pb)
  double xsecWeight = 1.0;

  //pileup weighting
  edm::LumiReWeighting * LumiWeights = NULL;
  utils::cmssw::PuShifter_t PuShifters;
  double PUNorm[] = { 1, 1, 1 };
  if (isMC)
    {
      std::vector < double >dataPileupDistributionDouble = runProcess.getParameter < std::vector < double >>("datapileup");
      std::vector < float >dataPileupDistribution;
      for (unsigned int i = 0; i < dataPileupDistributionDouble.size (); i++)
        {
          dataPileupDistribution.push_back (dataPileupDistributionDouble[i]);
        }
      std::vector < float >mcPileupDistribution;

      double totalNumEvent = utils::getMCPileupDistributionAndTotalEventFromMiniAOD(urls, dataPileupDistribution.size(), mcPileupDistribution);
      xsecWeight=xsec/totalNumEvent;

      //utils::getMCPileupDistributionFromMiniAOD(urls, dataPileupDistribution.size (), mcPileupDistribution);
      while (mcPileupDistribution.size () < dataPileupDistribution.size ()) mcPileupDistribution.push_back (0.0);
      while (mcPileupDistribution.size () > dataPileupDistribution.size ()) dataPileupDistribution.push_back (0.0);
      gROOT->cd ();             //THIS LINE IS NEEDED TO MAKE SURE THAT HISTOGRAM INTERNALLY PRODUCED IN LumiReWeighting ARE NOT DESTROYED WHEN CLOSING THE FILE
      LumiWeights = new edm::LumiReWeighting (mcPileupDistribution, dataPileupDistribution);
      PuShifters = utils::cmssw::getPUshifters (dataPileupDistribution, 0.05);
      utils::getPileupNormalization (mcPileupDistribution, PUNorm, LumiWeights, PuShifters);
    }

  if(debug){
    cout << "DEBUG: xsec: " << xsec << endl;
    cout << "DEBUG: xsecWeight: " << xsecWeight << endl;
    cout << "DEBUG: totalEntries: " << totalEntries << endl;
  }

  gROOT->cd ();                 //THIS LINE IS NEEDED TO MAKE SURE THAT HISTOGRAM INTERNALLY PRODUCED IN LumiReWeighting ARE NOT DESTROYED WHEN CLOSING THE FILE
  
  //higgs::utils::EventCategory eventCategoryInst(higgs::utils::EventCategory::EXCLUSIVE2JETSVBF); //jet(0,>=1)+vbf binning
  
  patUtils::MetFilter metFilter;
  if(!isMC)
    {
      // no 2016 yet // 	metFiler.FillBadEvents(string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/MetFilter/DoubleEG_RunD/DoubleEG_csc2015.txt");
      // no 2016 yet // 	metFiler.FillBadEvents(string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/MetFilter/DoubleEG_RunD/DoubleEG_ecalscn1043093.txt"); 
      // no 2016 yet // 	metFiler.FillBadEvents(string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/MetFilter/DoubleMuon_RunD/DoubleMuon_csc2015.txt");
      // no 2016 yet // 	metFiler.FillBadEvents(string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/MetFilter/DoubleMuon_RunD/DoubleMuon_ecalscn1043093.txt"); 
      // no 2016 yet // 	metFiler.FillBadEvents(string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/MetFilter/MuonEG_RunD/MuonEG_csc2015.txt");
      // no 2016 yet // 	metFiler.FillBadEvents(string(std::getenv("CMSSW_BASE"))+"/src/UserCode/llvv_fwk/data/MetFilter/MuonEG_RunD/MuonEG_ecalscn1043093.txt"); 
      
      //FIXME, we need to add here the single mu, single el, and gamma path
    }
  




  //##############################################
  //########           EVENT LOOP         ########
  //##############################################
  //loop on all the events
  printf ("Progressing Bar     :0%%       20%%       40%%       60%%       80%%       100%%\n");

  int nSkipped(0);
  int nMultiChannel(0);
  int nNoChannel(0);
  
  for(size_t f=0; f<urls.size();++f){
    TFile* file = TFile::Open(urls[f].c_str());
    fwlite::Event ev(file);
    printf ("Scanning the ntuple %2lu/%2lu : ", f+1, urls.size());
    int iev(0);
    int treeStep (ev.size() / 50);
    //DuplicatesChecker duplicatesChecker;
    //int nDuplicates(0);
    
    for (ev.toBegin(); !ev.atEnd(); ++ev)
    {
      iev++;
      totalEntries++;
      if (iev % treeStep == 0)
        {
          printf (".");
          fflush (stdout);
        }
      
      double weightGen(1.);
      if(isNLOMC)
        {
          //double weightGen(0.);                                                                                                                                                             
          //double weightLhe(0.);                                                                                                                                                             
          
          fwlite::Handle<GenEventInfoProduct> evt;
          evt.getByLabel(ev, "generator");
          if(evt.isValid())
            {
              //weightGen = (evt->weight() > 0 ) ? 1. : -1. ;
              weightGen = evt->weight();
              if(debug) cout << "NNLO gen weight: " << evt->weight() << endl;
            }
          else
            {
              if(debug) cout << "Event is not valid" << endl;
            }

        }
      
      //if(debug) cout << "Gen weight: " << weightGen << endl;

      std::vector < TString > tags (1, "all");

      //##############################################   EVENT LOOP STARTS   ##############################################
      // Not needed anymore with the current way of looping ev.to (iev);              //load the event content from the EDM file
      //if(!isMC && duplicatesChecker.isDuplicate( ev.run, ev.lumi, ev.event) ) { nDuplicates++; continue; }

      ///if(!patUtils::exclusiveDataEventFilter(ev.eventAuxiliary().run(), isMC, isPromptReco ) ) continue;

      // Skip bad lumi                                                                                                                              
      if(!goodLumiFilter.isGoodLumi(ev.eventAuxiliary().run(),ev.eventAuxiliary().luminosityBlock())) continue;

            
      //
      // DERIVE WEIGHTS TO APPLY TO SAMPLE
      //

      //pileup weight
      double weight(weightGen);
      double TotalWeight_plus(1.0);
      double TotalWeight_minus(1.0);
      double puWeight(1.0);

      if(isMC)
        {
          int ngenITpu(0);
          
          fwlite::Handle < std::vector < PileupSummaryInfo > >puInfoH;
          puInfoH.getByLabel (ev, "slimmedAddPileupInfo");
          for (std::vector < PileupSummaryInfo >::const_iterator it = puInfoH->begin (); it != puInfoH->end (); it++)
            {
              if (it->getBunchCrossing () == 0) ngenITpu += it->getPU_NumInteractions ();
            }
          //ngenITpu=nGoodPV; // based on nvtx
          puWeight = LumiWeights->weight (ngenITpu) * PUNorm[0];
          weight *= puWeight;
          TotalWeight_plus =  PuShifters[utils::cmssw::PUUP]  ->Eval (ngenITpu) * (PUNorm[2]/PUNorm[0]);
          TotalWeight_minus = PuShifters[utils::cmssw::PUDOWN]->Eval (ngenITpu) * (PUNorm[1]/PUNorm[0]);
        }
      
      
      mon.fillHisto("initNorm", tags, 0., weightGen); // Should be all 1, but for NNLO samples there are events weighting -1     
      mon.fillHisto("initNorm", tags, 1., weightGen); // Should be all 1, but for NNLO samples there are events weighting -1                                                            
      mon.fillHisto("initNorm", tags, 2, puWeight);
      mon.fillHisto("initNorm", tags, 3, TotalWeight_plus);
      mon.fillHisto("initNorm", tags, 4, TotalWeight_minus);


      //apply trigger and require compatibilitiy of the event with the PD
      edm::TriggerResultsByName tr = ev.triggerResultsByName ("HLT");
      if (!tr.isValid ())
        return false;

      
      if(debug && iev <100 && iev >95)
        {
        cout << "AVAILABLE TRIGGER BITS" << endl;
        //std::vector< std::vector<std::string>::const_iterator > matches = edm::regexMatch(tr.triggerNames(), "HLT_*Jet*");
        std::vector< std::vector<std::string>::const_iterator > matches = edm::regexMatch(tr.triggerNames(), "HLT_PFJet*");
          for(size_t t=0;t<matches.size();t++)
          cout << "\t\t\t" << matches[t]->c_str() << endl;
        }
        
      bool jetTrigger(true);
      bool muTrigger(true);

      if(!isMC)
        {
          jetTrigger = utils::passTriggerPatterns(tr, "HLT_PFJet450_v*");
          //bool muTrigger   (utils::passTriggerPatterns (tr, "HLT_IsoMu20_v*", "HLT_IsoTkMu20_v*"));
          muTrigger = utils::passTriggerPatterns(tr, "HLT_IsoMu22_v*","HLT_IsoTkMu22_v*");
          /* Available in miniaod v2      
             HLT_PFJet40_v1
             HLT_PFJet60_v1
             HLT_PFJet80_v1
             HLT_PFJet140_v1
             HLT_PFJet200_v1
             HLT_PFJet260_v1
             HLT_PFJet320_v1
             HLT_PFJet400_v1
             HLT_PFJet450_v1
             HLT_PFJet500_v1
          */
          if (filterOnlyJETHT)    {                     muTrigger = false; }
          if (filterOnlySINGLEMU) { jetTrigger = false;                    }
        }
      
      int metFilterValue(0);
      bool filterbadPFMuon = true; 
      bool filterbadChCandidate = true;
          
      metFilterValue = metFilter.passMetFilterInt( ev, true ); // true is is2016data                   // Apply Bad Charged Hadron and Bad Muon Filters from MiniAOD (for Run II 2016 only ) 
      filterbadChCandidate = metFilter.passBadChargedCandidateFilter(ev); if (!filterbadChCandidate) {  metFilterValue=9; } 
      filterbadPFMuon = metFilter.passBadPFMuonFilter(ev); if (!filterbadPFMuon) { metFilterValue=8; }
      
      
      if(debug) cout << "Triggers: jet " << jetTrigger << ", muon " << muTrigger << endl;
      
      if (!jetTrigger && !muTrigger){ nSkipped++; continue;}         //ONLY RUN ON THE EVENTS THAT PASS OUR TRIGGERS
      //##############################################   EVENT PASSED THE TRIGGER   #######################################

      if(debug) cout << "Event passed at least one trigger: jet " << jetTrigger << ", muon " << muTrigger << endl;

      // // -------------- Apply MET filters -------------------
      if( metFilterValue !=0 ) continue; // This is for MC as well.


      //load all the objects we will need to access
      reco::VertexCollection vtx;
      reco::Vertex goodPV;
      unsigned int nGoodPV(0);
      fwlite::Handle < reco::VertexCollection > vtxHandle;
      vtxHandle.getByLabel (ev, "offlineSlimmedPrimaryVertices");
      if (vtxHandle.isValid() ) vtx = *vtxHandle;
      for(size_t ivtx=0; ivtx<vtx.size(); ++ivtx)
        {
          if(utils::isGoodVertex(vtx[ivtx]))
            {
              if(nGoodPV==0) goodPV=vtx[ivtx];
              nGoodPV++;
            }
        }
      
      double rho = 0;
      fwlite::Handle < double >rhoHandle;
      rhoHandle.getByLabel (ev, "fixedGridRhoFastjetAll");
      if (rhoHandle.isValid() ) rho = *rhoHandle;
      
      if(isMC && mctruthmode!=0)
        {
          reco::GenParticleCollection gen;
          fwlite::Handle < reco::GenParticleCollection > genHandle;
          genHandle.getByLabel (ev, "prunedGenParticles");
          if (genHandle.isValid() ) gen = *genHandle;
          
          // Save time and don't load the rest of the objects when selecting by mctruthmode :)
          bool hasTop(false);
          int
            ngenLeptonsStatus3(0),
            ngenLeptonsNonTauSonsStatus3(0),
            ngenTausStatus3(0),
            ngenQuarksStatus3(0);
          //double tPt(0.), tbarPt(0.); // top pt reweighting - dummy value results in weight equal to 1 if not set in loop
          //float wgtTopPt(1.0), wgtTopPtUp(1.0), wgtTopPtDown(1.0);
          //if(iev != 500) continue;
          for(size_t igen=0; igen<gen.size(); igen++){
            // Following the new status scheme from: https://github.com/cms-sw/cmssw/pull/7791
            
            //cout << "Particle " << igen << " has " << gen[igen].numberOfDaughters() << ", pdgId " << gen[igen].pdgId() << " and status " << gen[igen].status() << ", pt " << gen[igen].pt() << ", eta " << gen[igen].eta() << ", phi " << gen[igen].phi() << ". isHardProcess is " << gen[igen].isHardProcess() << ", and isPromptFinalState is " << gen[igen].isPromptFinalState() << endl;
            //if(!gen[igen].isHardProcess() && !gen[igen].isPromptFinalState()) continue;
            if(gen[igen].status() != 1 &&  gen[igen].status() !=2 && gen[igen].status() !=62 ) continue;
            int absid=abs(gen[igen].pdgId());
            if(absid==6 && gen[igen].status()==62){ // particles of the hardest subprocess 22 : intermediate (intended to have preserved mass)
              hasTop=true;
              //if(isTTbarMC){
              //  if(gen[igen].get("id") > 0) tPt=gen[igen].pt();
              //  else                        tbarPt=gen[igen].pt();
              //}
            }
            
            //if(!gen[igen].isPromptFinalState() ) continue;
            if( (gen[igen].status() != 1 && gen[igen].status()!= 2 ) || !hasWasMother(gen[igen])) continue;
            if(absid==11 || absid==13){
              ngenLeptonsStatus3++;
              if(!hasTauAsMother(gen[igen]))
                ngenLeptonsNonTauSonsStatus3++;
            }
            if(absid==15 && !hasStableLeptonAsDaughter(gen[igen])      ) ngenTausStatus3++; // This should be summed to ngenLeptonsStatus3 for the dilepton final states, not summed for the single lepton final states.
            if(absid<=5              ) ngenQuarksStatus3++;
          }

          if(mctruthmode==1 && (ngenTausStatus3==0 || !hasTop)) continue;
          if(mctruthmode==2 && (ngenTausStatus3!=0 || !hasTop)) continue;
          
        }
      
      
      
      
      
      //      if(tPt>0 && tbarPt>0 && topPtWgt)
      //        {
      //          topPtWgt->computeWeight(tPt,tbarPt);
      //          topPtWgt->getEventWeight(wgtTopPt, wgtTopPtUp, wgtTopPtDown);
      //          wgtTopPtUp /= wgtTopPt;
      //          wgtTopPtDown /= wgtTopPt;
      //        }

      pat::MuonCollection muons;
      fwlite::Handle < pat::MuonCollection > muonsHandle;
      muonsHandle.getByLabel (ev, "slimmedMuons");
      if (muonsHandle.isValid() ) muons = *muonsHandle;

      pat::ElectronCollection electrons;
      fwlite::Handle < pat::ElectronCollection > electronsHandle;
      electronsHandle.getByLabel (ev, "slimmedElectrons");
      if (electronsHandle.isValid() ) electrons = *electronsHandle;

      pat::JetCollection jets;
      fwlite::Handle < pat::JetCollection > jetsHandle;
      jetsHandle.getByLabel (ev, "slimmedJets");
      if (jetsHandle.isValid() ) jets = *jetsHandle;

      /*
        pat::PhotonCollection photons;
        fwlite::Handle < pat::PhotonCollection > photonsHandle;
        photonsHandle.getByLabel (ev, "slimmedPhotons");
        if (photonsHandle.isValid() ) photons = *photonsHandle;
      */

      pat::METCollection mets;
      fwlite::Handle < pat::METCollection > metsHandle;
      metsHandle.getByLabel (ev, "slimmedMETs");
      if (metsHandle.isValid() ) mets = *metsHandle;
      LorentzVector met = mets[0].p4 ();

      if(debug ){
        // MET try:
        double mypt = mets[0].shiftedPt(pat::MET::METUncertainty::JetEnUp);
        cout << "MET = " << mets[0].pt() << ", JetEnUp: " << mypt << endl;
        LorentzVector myshiftedMet = mets[0].shiftedP4(pat::MET::METUncertainty::JetEnUp);
        cout << "MET = " << mets[0].pt() << ", JetEnUp: " << myshiftedMet.pt() << endl;
      }

      pat::TauCollection taus;
      fwlite::Handle < pat::TauCollection > tausHandle;
      tausHandle.getByLabel (ev, "slimmedTaus");
      if (tausHandle.isValid() ) taus = *tausHandle;

      // Old stitching for exclusive jets samplesif (isV0JetsMC)
      // Old stitching for exclusive jets samples  {
      // Old stitching for exclusive jets samples    fwlite::Handle < LHEEventProduct > lheEPHandle;
      // Old stitching for exclusive jets samples    lheEPHandle.getByLabel (ev, "externalLHEProducer");
      // Old stitching for exclusive jets samples    mon.fillHisto ("nup", "", lheEPHandle->hepeup ().NUP, 1);
      // Old stitching for exclusive jets samples    if (lheEPHandle->hepeup ().NUP > 5)  continue;
      // Old stitching for exclusive jets samples    mon.fillHisto ("nupfilt", "", lheEPHandle->hepeup ().NUP, 1);
      // Old stitching for exclusive jets samples  }
      
     
      // HT-binned samples stitching: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsToTauTauWorking2015#MC_and_data_samples
      if(isV0JetsMC || isControlWJets)
        {
          // access generator level HT
          fwlite::Handle<LHEEventProduct> lheEventProduct;
          lheEventProduct.getByLabel(ev, "externalLHEProducer");
          //edm::Handle<LHEEventProduct> lheEventProduct;  
          //ev.getByLabel( 'externalLHEProducer', lheEventProduct);
          const lhef::HEPEUP& lheEvent = lheEventProduct->hepeup();
          std::vector<lhef::HEPEUP::FiveVector> lheParticles = lheEvent.PUP;
          double lheHt = 0.;
          size_t numParticles = lheParticles.size();
          for ( size_t idxParticle = 0; idxParticle < numParticles; ++idxParticle ) {
            int absPdgId = TMath::Abs(lheEvent.IDUP[idxParticle]);
            int status = lheEvent.ISTUP[idxParticle];
            if ( status == 1 && ((absPdgId >= 1 && absPdgId <= 6) || absPdgId == 21) ) { // quarks and gluons
          lheHt += TMath::Sqrt(TMath::Power(lheParticles[idxParticle][0], 2.) + TMath::Power(lheParticles[idxParticle][1], 2.)); // first entry is px, second py
            } 
          }
          if(debug) cout << "Sample: " << dtag << " has isV0JetsMC " << isV0JetsMC << ", lheHt: " << lheHt << ", scale factor from spreadsheet: " << patUtils::getHTScaleFactor(dtag, lheHt) << endl; 
          if(isV0JetsMC) weightGen *=   patUtils::getHTScaleFactor(dtag, lheHt);
          if(debug) cout << "WeightGen is consequently " << weightGen << endl;
          mon.fillHisto("lheHt", tags, lheHt, weightGen);
        }


      //
      //
      // BELOW FOLLOWS THE ANALYSIS OF THE MAIN SELECTION WITH N-1 PLOTS
      //
      //


      
      //
      // LEPTON ANALYSIS
      //
      
      //start by merging electrons and muons
      std::vector < patUtils::GenericLepton > leptons;
      for(size_t l = 0; l < electrons.size (); l++) leptons.push_back (patUtils::GenericLepton (electrons[l] ));
      for(size_t l = 0; l < muons.size (); l++)     leptons.push_back (patUtils::GenericLepton (muons[l]     ));
      std::sort (leptons.begin (), leptons.end (), utils::sort_CandidatesByPt);

      LorentzVector muDiff (0, 0, 0, 0);
      std::vector < patUtils::GenericLepton > selLeptons; // Different main lepton definitions
      double nVetoLeptons(0);
      for (size_t ilep = 0; ilep < leptons.size (); ilep++)
        {
          bool 
            passKin(true),             passId(true),              passIso(true),
            passVetoKin(true),          passVetoId(true),          passVetoIso(true);
          int lid = leptons[ilep].pdgId();
          
          //apply muon corrections (MuScle fit - RunI)
          // if (abs (lid) == 13)
          //   {
          //     if (muCor)
          //       {
          //         TLorentzVector p4 (leptons[ilep].px(), leptons[ilep].py(), leptons[ilep].pz(), leptons[ilep].energy());
          //         muCor->applyPtCorrection (p4, lid < 0 ? -1 : 1);
          //         if (isMC) muCor->applyPtSmearing (p4, lid < 0 ? -1 : 1, false);
          //         muDiff -= leptons[ilep].p4();
          //         leptons[ilep].setP4(LorentzVector(p4.Px(), p4.Py(), p4.Pz(), p4.E()));
          //         muDiff += leptons[ilep].p4();
          //       }
          //   }

          // apply muon corrections (Rochester - RunII)
          if(abs(lid)==13){
            if(muCor){         
              float qter;
              TLorentzVector p4(leptons[ilep].px(),leptons[ilep].py(),leptons[ilep].pz(),leptons[ilep].energy());
              if(isMC)
                muCor->momcor_mc  (p4, lid<0 ? -1 :1, 0, qter);
              else
                muCor->momcor_data(p4, lid<0 ? -1 :1, 0, qter);
              muDiff -= leptons[ilep].p4();
              leptons[ilep].setP4(LorentzVector(p4.Px(),p4.Py(),p4.Pz(),p4.E() ) );
              muDiff += leptons[ilep].p4();
            }                  
          }
          //no need for charge info any longer
          lid = abs (lid);
          TString lepStr(lid == 13 ? "mu" : "e");
          
          // don't want to mess with photon ID // //veto nearby photon (loose electrons are many times photons...)
          // don't want to mess with photon ID // double minDRlg(9999.);
          // don't want to mess with photon ID // for(size_t ipho=0; ipho<selPhotons.size(); ipho++)
          // don't want to mess with photon ID //   minDRlg=TMath::Min(minDRlg,deltaR(leptons[ilep].p4(),selPhotons[ipho].p4()));
          // don't want to mess with photon ID // if(minDRlg<0.1) continue;
          
          //kinematics
          double leta = fabs (lid == 11 ? leptons[ilep].el.superCluster ()->eta() : leptons[ilep].eta());
          
          // Single lepton main + veto kin
          if (leptons[ilep].pt () < (lid==11 ? 30. : 25.))     passKin = false;
          if (leta > (lid == 11 ? 2.5 : 2.1))                { passKin = false; passVetoKin = false; }
          if (lid == 11 && (leta > 1.4442 && leta < 1.5660)) { passKin = false; passVetoKin = false; } // Crack veto
          
          // Single lepton veto kin
          if (leptons[ilep].pt () < (lid==11 ? 20. : 10.))   passVetoKin = false;

          //Cut based identification 
          passId      = lid==11 ? patUtils::passId(leptons[ilep].el, vtx[0], patUtils::llvvElecId::Tight, patUtils::CutVersion::ICHEP16Cut) : patUtils::passId(leptons[ilep].mu, vtx[0], patUtils::llvvMuonId::Tight, patUtils::CutVersion::ICHEP16Cut);
          passVetoId = lid==11 ? patUtils::passId(leptons[ilep].el, vtx[0], patUtils::llvvElecId::Loose, patUtils::CutVersion::ICHEP16Cut) : patUtils::passId(leptons[ilep].mu, vtx[0], patUtils::llvvMuonId::Loose, patUtils::CutVersion::ICHEP16Cut);
          //passId = lid == 11 ? patUtils::passId(leptons[ilep].el, goodPV, patUtils::llvvElecId::Tight) : patUtils::passId (leptons[ilep].mu, goodPV, patUtils::llvvMuonId::Tight);
          //passVetoId = passId;
          
          //isolation
          passIso     = lid == 11 ? patUtils::passIso(leptons[ilep].el, patUtils::llvvElecIso::Tight, patUtils::CutVersion::ICHEP16Cut, 0.) : patUtils::passIso(leptons[ilep].mu, patUtils::llvvMuonIso::Tight, patUtils::CutVersion::ICHEP16Cut);
          passVetoIso = lid == 11 ? patUtils::passIso(leptons[ilep].el, patUtils::llvvElecIso::Loose, patUtils::CutVersion::ICHEP16Cut, 0.) : patUtils::passIso(leptons[ilep].mu, patUtils::llvvMuonIso::Loose, patUtils::CutVersion::ICHEP16Cut);
          
          //passIso = lid == 11 ? patUtils::passIso (leptons[ilep].el, patUtils::llvvElecIso::Tight) : patUtils::passIso (leptons[ilep].mu, patUtils::llvvMuonIso::Tight); // Try tight iso for dilepton
          //passVetoIso = passIso;
          
          if (passKin && passId && passIso) selLeptons.push_back(leptons[ilep]);
          else if(passVetoKin && passVetoId && passVetoIso) nVetoLeptons++;// "else if" is for having nVetoLeptons to be the number of *additional* leptons in the event.
          
        }
      std::sort(selLeptons.begin(),   selLeptons.end(),   utils::sort_CandidatesByPt);
      LorentzVector recoMET = met - muDiff;
      
      //pre-select the taus
      pat::TauCollection selTaus;
      for (size_t itau = 0; itau < taus.size(); ++itau)
        {
          pat::Tau& tau = taus[itau];
          if(tau.pt() < 20. || fabs (tau.eta()) > 2.3) continue;
          
          bool overlapWithLepton(false);
          for(int l1=0; l1<(int)selLeptons.size();++l1){
            if(deltaR(tau, selLeptons[l1])<0.4){overlapWithLepton=true;}
          }
          if(overlapWithLepton) continue;
          
          //      if(!tau.isPFTau()) continue; // Only PFTaus // It should be false for slimmedTaus
          //      if(tau.emFraction() >=2.) continue;
          
          //if(!tau.tauID("decayModeFindingNewDMs")) continue; // High pt tau. Otherwise, OldDMs (but it not included anymore in miniAOD)
          if(!tau.tauID("decayModeFinding")) continue;
          // Anyways, the collection of taus from miniAOD should be already afer decayModeFinding cut (the tag - Old or New - is unspecified in the twiki, though).
          
          //if (!tau.tauID ("byMediumCombinedIsolationDeltaBetaCorr3Hits")) continue;
          // Independent from lepton rejection algos performance
          //if (!tau.tauID ("againstMuonTight3"))                           continue;
          //if (!tau.tauID ("againstElectronMediumMVA5"))                   continue;
          
          // Pixel hits cut (will be available out of the box in new MINIAOD production)
          int nChHadPixelHits = 0;
          reco::CandidatePtrVector chCands = tau.signalChargedHadrCands();
          for(reco::CandidatePtrVector::const_iterator iter = chCands.begin(); iter != chCands.end(); iter++){
            pat::PackedCandidate const* packedCand = dynamic_cast<pat::PackedCandidate const*>(iter->get());
            int pixelHits = packedCand->numberOfPixelHits();
            if(pixelHits > nChHadPixelHits) nChHadPixelHits = pixelHits;
          }
          if(nChHadPixelHits==0) continue;
          //

          selTaus.push_back(tau);
        }
      std::sort (selTaus.begin(), selTaus.end(), utils::sort_CandidatesByPt);

      // Tau IDs
      vector<pat::TauCollection> selIdTaus;
      selIdTaus.clear();
      for(size_t l=0; l<tauDiscriminators.size(); ++l){
        TString tcat(tauDiscriminators[l]);
        pat::TauCollection temp;
        for(pat::TauCollection::iterator tau=selTaus.begin(); tau!=selTaus.end(); ++tau){
          if(!tau->tauID(tcat)) continue;
          temp.push_back(*tau);
        }
        std::sort(temp.begin(), temp.end(), utils::sort_CandidatesByPt);
        selIdTaus.push_back(temp);
      }
      
      //
      //JET/MET ANALYSIS
      //
      //add scale/resolution uncertainties and propagate to the MET      
      utils::cmssw::updateJEC(jets,jesCor,totalJESUnc,rho,nGoodPV,isMC);  //FIXME if still needed
      //std::vector<LorentzVector> met=utils::cmssw::getMETvariations(recoMet,jets,selLeptons,isMC); //FIXME if still needed
      
      //select the jets

      //utils::cmssw::updateJEC(jets, jesCor, totalJESUnc, rho, nGoodPV, isMC);

      pat::JetCollection
        selWJetsJets, selQCDJets,
        selHardJets,
        selWJetsBJets, selQCDBJets;
      for (size_t ijet = 0; ijet < jets.size(); ijet++)
        {
          // if (jets[ijet].pt() < 15 || fabs (jets[ijet].eta()) > 4.7) continue; // Reactivate this for jecs
          if(jets[ijet].pt() < 20 || fabs(jets[ijet].eta()) > 2.3) continue;
          
          //mc truth for this jet
          const reco::GenJet * genJet = jets[ijet].genJet();
          TString jetType (genJet && genJet->pt() > 0 ? "truejetsid" : "pujetsid");
          
          //jet id
          bool passPFloose = patUtils::passPFJetID("Loose", jets[ijet]);
          //float PUDiscriminant = jets[ijet].userFloat ("pileupJetId:fullDiscriminant");
          bool passLooseSimplePuId = true;//patUtils::passPUJetID(jets[ijet]); //FIXME Broken in miniAOD V2 : waiting for JetMET fix.
          if (!passPFloose || !passLooseSimplePuId || jets[ijet].pt() <20 || fabs(jets[ijet].eta()) > 2.3) continue;

          //cross-clean with selected leptons and photons
          double minDRlj(9999.);
          for (size_t ilep = 0; ilep < selLeptons.size(); ilep++)
            minDRlj = TMath::Min(minDRlj, deltaR (jets[ijet], selLeptons[ilep]));
          
          bool hasCSVtag( jets[ijet].bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags") > btagMedium );
          bool hasCSVtagUp = hasCSVtag;  
          bool hasCSVtagDown = hasCSVtag;
          //update according to the SF measured by BTV
          if(isMC){
            int flavId=jets[ijet].partonFlavour();  double eta=jets[ijet].eta();
            btsfutil.SetSeed(ev.eventAuxiliary().event()*10 + ijet*10000);
            if      (abs(flavId)==5){  btsfutil.modifyBTagsWithSF(hasCSVtag    , btagCal   .eval(BTagEntry::FLAV_B   , eta, jets[ijet].pt()), beff);
              btsfutil.modifyBTagsWithSF(hasCSVtagUp  , btagCalUp .eval(BTagEntry::FLAV_B   , eta, jets[ijet].pt()), beff);
              btsfutil.modifyBTagsWithSF(hasCSVtagDown, btagCalDn .eval(BTagEntry::FLAV_B   , eta, jets[ijet].pt()), beff);
            }else if(abs(flavId)==4){  btsfutil.modifyBTagsWithSF(hasCSVtag    , btagCal   .eval(BTagEntry::FLAV_C   , eta, jets[ijet].pt()), beff);
              btsfutil.modifyBTagsWithSF(hasCSVtagUp  , btagCalUp .eval(BTagEntry::FLAV_C   , eta, jets[ijet].pt()), beff);
              btsfutil.modifyBTagsWithSF(hasCSVtagDown, btagCalDn .eval(BTagEntry::FLAV_C   , eta, jets[ijet].pt()), beff);
            }else{                     btsfutil.modifyBTagsWithSF(hasCSVtag    , btagCalL  .eval(BTagEntry::FLAV_UDSG, eta, jets[ijet].pt()), leff);
              btsfutil.modifyBTagsWithSF(hasCSVtagUp  , btagCalLUp.eval(BTagEntry::FLAV_UDSG, eta, jets[ijet].pt()), leff);
              btsfutil.modifyBTagsWithSF(hasCSVtagDown, btagCalLDn.eval(BTagEntry::FLAV_UDSG, eta, jets[ijet].pt()), leff);
            }
          }

          selQCDJets.push_back(jets[ijet]);
          if(hasCSVtag)
            selQCDBJets.push_back(jets[ijet]);

          if(minDRlj < 0.4) continue;

          selWJetsJets.push_back(jets[ijet]);
          if(jets[ijet].pt()>40)
            selHardJets.push_back(jets[ijet]);
          if(hasCSVtag)
            selWJetsBJets.push_back(jets[ijet]);
        }
      std::sort (selWJetsJets.begin(), selWJetsJets.end(), utils::sort_CandidatesByPt);
      std::sort (selQCDJets.begin()  , selQCDJets.end()  , utils::sort_CandidatesByPt);
      std::sort (selHardJets.begin() , selHardJets.end() , utils::sort_CandidatesByPt);
      
      // Event classification and analyses
      //if(muTrigger)  tags.push_back("wjets");
      //if(jetTrigger) tags.push_back("qcd"  );
      if(muTrigger && jetTrigger) nMultiChannel++;
      if(!muTrigger && !jetTrigger) nNoChannel++;
      
      // W+jet full analysis
      if(muTrigger){
        // Selection mimicking AN2014_008_v11
        // - HLT_IsoMu24 trigger
        // - An event vertex with Ndof>=4, |Zvtx|<24cm, |r|<2cm
        // - A muon Pt>25GeV, |eta|<2.1, passing deltabeta-corrected isolation I<0.06ptmu  (Tight muon)
        // - Transverse mass Mt>50GeV (muon+MET)
        // - At least one jet pt>20GeV, |eta|<2.5 not overlapping with the muon withing DeltaR<0.7
        
        vector<TString> wjetTags(1, "all");
        wjetTags.push_back("wjet");

        // At least one event vertex
        bool passVtx(nGoodPV);
        // One lepton
        bool passLepton(selLeptons.size()==1);
        bool passLeptonVeto(nVetoLeptons==0);
        if(passLepton) passLepton = (passLepton && (abs(selLeptons[0].pdgId()) == 13) );
        if(passLepton) // Updated lepton selection
          {
            int id(abs(selLeptons[0].pdgId()));                                                                                                                               
            weight *= isMC ? lepEff.getLeptonEfficiency( selLeptons[0].pt(), selLeptons[0].eta(), id,  id ==11 ? "tight"    : "tight ", patUtils::CutVersion::ICHEP16Cut   ).first : 1.0; //ID               
          }
        
        // Transverse mass
        double mt(0.);
        if(selLeptons.size()>0)
          {
            LorentzVector leptonformt = selLeptons[0].p4();
            mt =utils::cmssw::getMT<const LorentzVector,LorentzVector>(leptonformt,recoMET);

            // Apply lepton efficiency

          }
        bool passMt(mt>50 );
        // At least one jet not overlapping with the muon
        bool passJet(selWJetsJets.size()>0);
        bool passBtag(selWJetsBJets.size()==0); // Kill ttbar component with btag veto
        bool passHardJets(selHardJets.size()<2);
        
        
        // Setting up control categories and fill up event flow histo
        std::vector < TString > ctrlCats; ctrlCats.clear ();
                                                                                                     { ctrlCats.push_back("step1"); mon.fillHisto("wjet_eventflow", wjetTags, 0, weight);}
        if(passVtx)                                                                                  { ctrlCats.push_back("step2"); mon.fillHisto("wjet_eventflow", wjetTags, 1, weight);}
        if(passVtx && passLepton)                                                                    { ctrlCats.push_back("step3"); mon.fillHisto("wjet_eventflow", wjetTags, 2, weight);}
        if(passVtx && passLepton && passMt )                                                         { ctrlCats.push_back("step4"); mon.fillHisto("wjet_eventflow", wjetTags, 3, weight);}
        if(passVtx && passLepton && passMt && passJet)                                               { ctrlCats.push_back("step5"); mon.fillHisto("wjet_eventflow", wjetTags, 4, weight);}
        if(passVtx && passLepton && passMt && passJet && passBtag)                                   { ctrlCats.push_back("step6"); mon.fillHisto("wjet_eventflow", wjetTags, 5, weight);} 
        if(passVtx && passLepton && passMt && passJet && passBtag && passLeptonVeto)                 { ctrlCats.push_back("step7"); mon.fillHisto("wjet_eventflow", wjetTags, 6, weight);} 
        if(passVtx && passLepton && passMt && passJet && passBtag && passLeptonVeto && passHardJets) { ctrlCats.push_back("step8"); mon.fillHisto("wjet_eventflow", wjetTags, 7, weight);} 
        
        // Fill the control plots
        for(size_t k=0; k<ctrlCats.size(); ++k){
          
          TString icat(ctrlCats[k]);
          //if(icat!="step5" && icat!="step6" && icat!="step7" && icat!="step8") continue; // Only for final selection step, for a quick test
          
          
          // Fake rate: 
          // fr = (pt_jet>20 && |eta_jet| <2.3 && pt_tau>20 && |eta_tau|<2.3 && DM-finding && tauID) / (pt_jet>20 && |eta_jet| <2.3)
          for(pat::JetCollection::iterator jet=selWJetsJets.begin(); jet!=selWJetsJets.end(); ++jet)
            {
              if(jet->pt()<20.0 || abs(jet->eta())>2.3) continue;
              
              double jetWidth( ((jet->etaetaMoment()+jet->phiphiMoment())> 0) ? sqrt(jet->etaetaMoment()+jet->phiphiMoment()) : 0.);
              
              mon.fillHisto(icat+"pt_denominator",      wjetTags, jet->pt()   , weight); // Variable number of bins to be implemented
              mon.fillHisto(icat+"met_denominator",     wjetTags, met.pt()    , weight); // Variable number of bins to be implemented
              mon.fillHisto(icat+"recomet_denominator", wjetTags, recoMET.pt(), weight); // Variable number of bins to be implemented
              mon.fillHisto(icat+"eta_denominator",     wjetTags, jet->eta()  , weight);
              mon.fillHisto(icat+"radius_denominator",  wjetTags, jetWidth    , weight);
              mon.fillHisto(icat+"nvtx_denominator",    wjetTags, nGoodPV  , weight);

              // This must be repeated for each discriminator
              for(size_t l=0; l<tauDiscriminators.size(); ++l){
                TString tcat(tauDiscriminators[l]);
                
                // Match taus
                double minDRtj(9999.);
                pat::Tau theTau;
                for (pat::TauCollection::iterator tau=selIdTaus[l].begin(); tau!=selIdTaus[l].end(); ++tau)
                  {
                    if(deltaR(*jet, *tau) < minDRtj) theTau=*tau;
                    minDRtj = TMath::Min(minDRtj, deltaR(*jet, *tau));
                  }
                if(minDRtj>0.4) continue;
                if(theTau.pt()<20. || theTau.eta()>2.3) continue; // Numerator has both requirements (jet and tau) for pt and eta
                mon.fillHisto(icat+tcat+"pt_numerator",       wjetTags, jet->pt()   , weight); // Variable number of bins to be implemented
                mon.fillHisto(icat+tcat+"met_numerator",      wjetTags, met.pt()    , weight); // Variable number of bins to be implemented
                mon.fillHisto(icat+tcat+"recomet_numerator",  wjetTags, recoMET.pt(), weight); // Variable number of bins to be implemented
                mon.fillHisto(icat+tcat+"eta_numerator",      wjetTags, jet->eta()  , weight);
                mon.fillHisto(icat+tcat+"radius_numerator",   wjetTags, jetWidth    , weight);
                mon.fillHisto(icat+tcat+"nvtx_numerator",     wjetTags, nGoodPV  , weight);
              }
            }

          
          // Some control plots, mostly on event selection
          mon.fillHisto(icat+"nvtx",    wjetTags,  nGoodPV, weight);
          if(selLeptons.size()>0)
            mon.fillHisto(icat+"ptmu",    wjetTags,  selLeptons[0].pt(), weight);
          for(pat::JetCollection::iterator jet=selWJetsJets.begin(); jet!=selWJetsJets.end(); ++jet)
            mon.fillHisto(icat+"jetpt",   wjetTags, jet->pt(), weight);
          mon.fillHisto(icat+"met",     wjetTags,  met.pt(),     weight);
          mon.fillHisto(icat+"recoMet", wjetTags,  recoMET.pt(), weight);
          mon.fillHisto(icat+"mt",      wjetTags,  mt, weight);
        }
        
        
      } // End WJets full analysis
      
      // QCD full analysis
      if(jetTrigger){
        vector<TString> qcdTags(1, "all");
        qcdTags.push_back("qcd");
        
        // - HLT_PfJet320 (450)
        // - An event vertex with Ndof>=4, |Zvtx|<24cm, |r|<2cm
        // - At least two jets pt>20GeV, |eta|<2.5. At least one of the jets is required to be matched within DeltaR<0.3 to the jet firing the jet trigger.
        // - If there is only one jet matching the trigger requirement in the event, the jet is excluded from the fake-rate computation


        // At least one event vertex
        bool passVtx(nGoodPV); // Ask someone about the offlineSkimmedPrimaryVertices collection
        // At least two jets
        // Insert trigger matching here, which will fix the incomplete jet selection step
        bool passJet(selQCDJets.size()>1);

        // Check trigger firing:
        int jetsFiring(0);
        double fireEta(0), firePhi(0);
        //load all the objects we will need to access
        edm::TriggerResults triggerBits;
        fwlite::Handle<edm::TriggerResults> triggerBitsHandle;
        triggerBitsHandle.getByLabel(ev, "TriggerResults","","HLT"); // ?
        if(triggerBitsHandle.isValid()) triggerBits = *triggerBitsHandle;
        const edm::TriggerNames& trigNames = ev.triggerNames(triggerBits);

        pat::TriggerObjectStandAloneCollection triggerObjects;
        fwlite::Handle<pat::TriggerObjectStandAloneCollection> triggerObjectsHandle;
        triggerObjectsHandle.getByLabel(ev, "selectedPatTrigger" );
        if(triggerObjectsHandle.isValid()) triggerObjects = *triggerObjectsHandle;

        for(pat::TriggerObjectStandAlone obj : triggerObjects){
          obj.unpackPathNames(trigNames);
          // I already know that (if(jetTrigger)) if(utils::passTriggerPatterns(tr, "HLT_PFJet260_v*"))
          // I guess it's "both L3 and LF", that is "true true". L3 only is "false true", LF only is "true false", none is "false false"
          if(obj.hasPathName("HLT_PFJet450_v*", true, true)){
            for(pat::JetCollection::iterator jet=selQCDJets.begin(); jet!=selQCDJets.end(); ++jet){
              if( deltaR(obj.eta(), obj.phi(), jet->eta(), jet->phi()) < 0.3 ){
                if(jetsFiring==0)
                  {
                    fireEta=jet->eta();
                    firePhi=jet->phi();
                  }
                jetsFiring++;
              }
            }
          }
        }
        // At least one of the jets is required to be matched with the one firing HLT
        if(jetsFiring==0) passJet=false;
        
        // Lepton veto, like a boss
        bool passLepton(selLeptons.size()==0 && nVetoLeptons==0); 
        
        

        // Setting up control categories and fill up event flow histo
        std::vector < TString > ctrlCats;
        ctrlCats.clear ();
                                             { ctrlCats.push_back("step1"); mon.fillHisto("qcd_eventflow", qcdTags, 0, weight);}
        if(passVtx   )                       { ctrlCats.push_back("step2"); mon.fillHisto("qcd_eventflow", qcdTags, 1, weight);}
        if(passVtx && passJet)               { ctrlCats.push_back("step3"); mon.fillHisto("qcd_eventflow", qcdTags, 2, weight);}
        if(passVtx && passJet && passLepton) { ctrlCats.push_back("step4"); mon.fillHisto("qcd_eventflow", qcdTags, 3, weight);}
        
        // Fill the control plots
        for(size_t k=0; k<ctrlCats.size(); ++k){
          
          TString icat(ctrlCats[k]);
          //if(icat!="step3" && icat!="step4") continue; // Only for final selection step, for a quick test
          
          // Fake rate: 
          // fr = (pt_jet>20 && |eta_jet| <2.3 && pt_tau>20 && |eta_tau|<2.3 && DM-finding && tauID) / (pt_jet>20 && |eta_jet| <2.3)
          for(pat::JetCollection::iterator jet=selQCDJets.begin(); jet!=selQCDJets.end(); ++jet)
            {
              if(jet->pt() < 20 || abs(jet->eta())>2.3) continue;
              
              // Multi-jet event: the trigger jet is excluded from the fake rate computation if there is only one jet that passes the trigger requirement
              // In case more than one jet passes the requirement, all are used
              if(jetsFiring==1)
                {
                  if(jet->eta() == fireEta && jet->phi() == firePhi) continue; // Skip the firing jet if it is the only one passing the trigger requirement
                }
        
              
              double jetWidth( ((jet->etaetaMoment()+jet->phiphiMoment())> 0) ? sqrt(jet->etaetaMoment()+jet->phiphiMoment()) : 0.);
              
              mon.fillHisto(icat+"pt_denominator",      qcdTags, jet->pt()   , weight); // Variable number of bins to be implemented
              mon.fillHisto(icat+"met_denominator",     qcdTags, met.pt()    , weight); // Variable number of bins to be implemented
              mon.fillHisto(icat+"recomet_denominator", qcdTags, recoMET.pt(), weight); // Variable number of bins to be implemented
              mon.fillHisto(icat+"eta_denominator",     qcdTags, jet->eta()  , weight);
              mon.fillHisto(icat+"radius_denominator",  qcdTags, jetWidth    , weight);
              mon.fillHisto(icat+"nvtx_denominator",    qcdTags, nGoodPV     , weight);
              
              // This must be repeated for each discriminator
              for(size_t l=0; l<tauDiscriminators.size(); ++l){
                TString tcat(tauDiscriminators[l]);
                // Match taus
                //cross-clean with selected leptons and photons
                double minDRtj(9999.);
                pat::Tau theTau;
                for (pat::TauCollection::iterator tau=selIdTaus[l].begin(); tau!=selIdTaus[l].end(); ++tau)
                  {
                    if(deltaR(*jet, *tau) < minDRtj) theTau=*tau;
                    minDRtj = TMath::Min(minDRtj, deltaR(*jet, *tau));
                  }
                if(minDRtj>0.4) continue;
                if(theTau.pt()<20. || theTau.eta()>2.3) continue; // Numerator has both requirements (jet and tau) for pt and eta
                mon.fillHisto(icat+tcat+"pt_numerator",      qcdTags, jet->pt()    , weight); // Variable number of bins to be implemented
                mon.fillHisto(icat+tcat+"met_numerator",     qcdTags, met.pt()    , weight); // Variable number of bins to be implemented
                mon.fillHisto(icat+tcat+"recomet_numerator", qcdTags, recoMET.pt(), weight); // Variable number of bins to be implemented
                mon.fillHisto(icat+tcat+"eta_numerator",     qcdTags, jet->eta()   , weight);
                mon.fillHisto(icat+tcat+"radius_numerator",  qcdTags, jetWidth     , weight);
                mon.fillHisto(icat+tcat+"nvtx_numerator",    qcdTags, nGoodPV     , weight);
              }
            }
          // Some control plots, mostly on event selection
          mon.fillHisto(icat+"nvtx",    qcdTags,  nGoodPV, weight);
          if(selLeptons.size()>0)
            mon.fillHisto(icat+"ptmu",    qcdTags,  selLeptons[0].pt(), weight);
          for(pat::JetCollection::iterator jet=selQCDJets.begin(); jet!=selQCDJets.end(); ++jet)
            mon.fillHisto(icat+"jetpt",   qcdTags, jet->pt(), weight);
          mon.fillHisto(icat+"met",     qcdTags,  met.pt(),     weight);
          mon.fillHisto(icat+"recoMet", qcdTags,  recoMET.pt(), weight);
          // mon.fillHisto(icat+"mt",      tags,  mt, weight); // No mt for QCD selection (not requiring any lepton)
        }
        
      } // End single lepton full analysis
      
      continue; // Quick break (statistical analysis will come later)
      
      //
      // HISTOS FOR STATISTICAL ANALYSIS (include systematic variations)
      //
      //Fill histogram for posterior optimization, or for control regions
      for (size_t ivar = 0; ivar < nvarsToInclude; ivar++)
        {
          double iweight = weight;       //nominal

          //energy scale/resolution
          bool varyJesUp (varNames[ivar] == "_jesup");
          bool varyJesDown (varNames[ivar] == "_jesdown");
          bool varyJerUp (varNames[ivar] == "_jerup");
          bool varyJerDown (varNames[ivar] == "_jerdown");
          bool varyUmetUp (varNames[ivar] == "_umetup");
          bool varyUmetDown (varNames[ivar] == "_umetdown");
          bool varyLesUp (varNames[ivar] == "_lesup");
          bool varyLesDown (varNames[ivar] == "_lesdown");

          //pileup variations
          if (varNames[ivar] == "_puup")   iweight *= TotalWeight_plus;
          if (varNames[ivar] == "_pudown") iweight *= TotalWeight_minus;

          //btag
          bool varyBtagUp (varNames[ivar] == "_btagup");
          bool varyBtagDown (varNames[ivar] == "_btagdown");

          //Here were the Q^2 variations on VV pT spectum


          //recompute MET/MT if JES/JER was varied
          LorentzVector zvv = mets[0].p4();
          //FIXME
          //      if(varyJesUp)    zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetEnUp);
          //      if(varyJesDown)  zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetEnDown);
          //      if(varyJerUp)    zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetResUp);
          //      if(varyJerDown)  zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetResDown);
          //      if(varyUmetUp)   zvv = mets[0].shiftedP4(pat::MET::METUncertainty::UnclusteredEnUp);
          //      if(varyUmetDown) zvv = mets[0].shiftedP4(pat::MET::METUncertainty::UnclusteredEnDown);
          //      if(varyLesUp)    zvv = met[utils::cmssw::LESUP]; //FIXME  must vary all leptons separately: MuonEnUp/MuonEnDown/ElectronEnUp/ElectronEnDown/TauEnUp/TauEnDown
          //      if(varyLesDown)  zvv = met[utils::cmssw::LESDOWN];

          pat::JetCollection tightVarJets;
          bool passLocalBveto (true);///passBtags);
          for (size_t ijet = 0; ijet < jets.size(); ijet++)
            {

              double eta = jets[ijet].eta();
              if (fabs (eta) > 4.7) continue;
              double pt = jets[ijet].pt();
              //FIXME
              //        if(varyJesUp)    pt=jets[ijet].getVal("jesup");
              //        if(varyJesDown)  pt=jets[ijet].getVal("jesdown");
              //        if(varyJerUp)    pt=jets[ijet].getVal("jerup");
              //        if(varyJerDown)  pt=jets[ijet].getVal("jerdown");
              if (pt < 30) continue;

              //cross-clean with selected leptons and photons
              double minDRlj (9999.), minDRlg (9999.);
              for (size_t ilep = 0; ilep < selLeptons.size(); ilep++)
                minDRlj = TMath::Min (minDRlj, double(deltaR(jets[ijet].p4(), selLeptons[ilep].p4())));
              // don't want to mess with photon ID // for(size_t ipho=0; ipho<selPhotons.size(); ipho++)
              // don't want to mess with photon ID //   minDRlg = TMath::Min( minDRlg, deltaR(jets[ijet].p4(),selPhotons[ipho].p4()) );
              if (minDRlj < 0.4 /*|| minDRlg<0.4 */ ) continue;

              //jet id
              bool passPFloose = patUtils::passPFJetID("Loose", jets[ijet]);      //FIXME --> Need to be updated according to te latest recipe;
              int simplePuId = true;    //FIXME
              bool passLooseSimplePuId = true;//patUtils::passPUJetID(jets[ijet]); //FIXME Broken in miniAOD V2 : waiting for JetMET fix.
              if (!passPFloose || !passLooseSimplePuId) continue;

              //jet is selected
              tightVarJets.push_back (jets[ijet]);

              //check b-tag
              if (pt < 30 || fabs (eta) > 2.5) continue;
              if (!isMC) continue;
              if (!varyBtagUp && !varyBtagDown) continue;
              int flavId = jets[ijet].partonFlavour();
              bool hasCSVtag (jets[ijet].bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags") > btagMedium);
              if (varyBtagUp)
                {
                  if (abs (flavId) == 5)      btsfutil.modifyBTagsWithSF(hasCSVtag, sfb + sfbunc,     beff);
                  else if (abs (flavId) == 4) btsfutil.modifyBTagsWithSF(hasCSVtag, sfb/5 + 2*sfbunc, beff);
                  else                        btsfutil.modifyBTagsWithSF(hasCSVtag, sfl + sflunc,     leff);
                }
              else if (varyBtagDown)
                {
                  if (abs (flavId) == 5)      btsfutil.modifyBTagsWithSF(hasCSVtag, sfb - sfbunc,     beff);
                  else if (abs (flavId) == 4) btsfutil.modifyBTagsWithSF(hasCSVtag, sfb/5 - 2*sfbunc, beff);
                  else                        btsfutil.modifyBTagsWithSF(hasCSVtag, sfl - sflunc,     leff);
                }
              passLocalBveto |= hasCSVtag;
            }


          //re-assign the event category to take migrations into account
          //      TString evCat  = eventCategoryInst.GetCategory(tightVarJets,dileptonSystem);
          //for (size_t ich = 0; ich < chTags.size(); ich++)
          //  {
          //    
          //    //TString tags_full = chTags[ich];  //+evCat;
          //    //double chWeight (iweight);
          //    
          //    //update weight and mass for photons
          //    //LorentzVector idileptonSystem (dileptonSystem);
          //    
          //    //updet the transverse mass
          //    //double mt = higgs::utils::transverseMass (idileptonSystem, zvv, true);
          //    
          //  }
        } // End stat analysis

    } // End single file event loop
    printf("\n");
    delete file;
  } // End loop on files

  if(nSkipped>0) cout << "Warning! There were " << nSkipped << " skipped because of trigger events out of " << totalEntries << " events!" << endl;
  if(nMultiChannel>0) cout << "Warning! There were " << nMultiChannel << " multi-channel events out of " << totalEntries << " events!" << endl;
  if(nNoChannel>0) cout << "Warning! There were " << nNoChannel << " no-channel events out of " << totalEntries << " events!" << endl;
  printf ("\n");

  //##############################################
  //########     SAVING HISTO TO FILE     ########
  //##############################################
  //save control plots to file
  printf ("Results save in %s\n", outUrl.Data());

  //save all to the file
  TFile *ofile = TFile::Open (outUrl, "recreate");
  mon.Write();
  ofile->Close();

  if (outTxtFile)
    fclose (outTxtFile);

  // Now that everything is done, dump the list of lumiBlock that we processed in this job
  if(!isMC){
    goodLumiFilter.FindLumiInFiles(urls);
    goodLumiFilter.DumpToJson(((outUrl.ReplaceAll(".root",""))+".json").Data());
  }


}
