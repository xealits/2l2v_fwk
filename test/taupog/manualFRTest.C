void manualFRTest()
{

  TFile* f = TFile::Open("/eos/user/v/vischia/www/taufakes_new/plotter_wjet.root", "READ");
  
  TString
    data("data (single mu)"),
    ttbar("TTbar"),
    wjets("W#rightarrow l#nu inclusive"),
    qcd("QCD_EMEnr_Incl");
  
  
  TString
    numerator("/wjet_step5byLooseCombinedIsolationDeltaBetaCorr3Hitseta_numerator"),
    denominator("/wjet_step5eta_denominator");

  

  TH1F* data_num = (TH1F*) f->Get(data+numerator);
  TH1F* data_den = (TH1F*) f->Get(data+denominator);

  TH1F* ttbar_num = (TH1F*) f->Get(ttbar+numerator);
  TH1F* ttbar_den = (TH1F*) f->Get(ttbar+denominator);

  TH1F* wjets_num = (TH1F*) f->Get(wjets+numerator);
  TH1F* wjets_den = (TH1F*) f->Get(wjets+denominator);

  TH1F* qcd_num = (TH1F*) f->Get(qcd+numerator);
  TH1F* qcd_den = (TH1F*) f->Get(qcd+denominator);
  

  TH1F* mc_num = (TH1F*) ttbar_num->Clone("mc_num");
  mc_num->Add(wjets_num);
  //mc_num->Add(qcd_num);

  TH1F* mc_den = (TH1F*) ttbar_den->Clone("mc_den");
  mc_den->Add(wjets_den);
  //mc_den->Add(qcd_den);

  data_num->SetMarkerColor(kBlack);
  mc_num->SetMarkerColor(kRed);

  data_den->SetMarkerColor(kBlack);
  mc_den->SetMarkerColor(kRed);

  TCanvas* d = new TCanvas("d", "d", 600, 1200);
  d->Divide(1,2);
  d->cd(1);
  gPad->SetTitle("numerator");
  data_num->DrawCopy("pe");
  mc_num->DrawCopy("pesame");
  d->cd(2);
  gPad->SetTitle("denominator");
  data_den->DrawCopy("pe");
  mc_den->DrawCopy("pesame");
  


  TH1F* data_fr = (TH1F*) data_num->Clone("data_fr");
  data_fr->Divide(data_den);
  
  TH1F* mc_fr = (TH1F*) mc_num->Clone("mc_fr");
  mc_fr->Divide(mc_den);
  
  data_fr->SetMarkerColor(kBlack);
  mc_fr->SetMarkerColor(kRed);
  
  data_fr->SetMarkerSize(2);
  mc_fr->SetMarkerSize(2);

  TCanvas* c = new TCanvas("c", "c", 600, 600);
  c->cd();

  data_fr->Draw("pe");
  mc_fr->Draw("pesame");
  
  

              


}
