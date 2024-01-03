// C++ includes
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>

using namespace std;

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TBranch.h"
#include "TSystem.h"

struct Para
{
  char field[20];
  double l;
  double h;
  double* field_value;
};

struct Sel_type
{
  const char* sel_name;
  const char* eff_name;
  bool calced=false;
  float* sel_value;
  double* eff_value;
  Sel_type() {}
  Sel_type(const char* sn, const char* en, bool c, float* sv, double* ev)
  :sel_name(sn),eff_name(en),calced(c),sel_value(sv),eff_value(ev) {}
};

float muon_cont, muon_tra, muon_sel, hadr, comb;
double muon_cont_eff, muon_tra_eff, muon_sel_eff, hadr_eff, comb_eff;
double x_pos, y_pos, z_pos, XLepMom, YLepMom, ZLepMom;
double TotalMom, cos_angle, LongitudinalMom;
double E_vis_true, ev;
const char* list_of_directories[40]={"0mgsimple","0m","1.75m","2m","4m","5.75m","8m","9.75m","12m","13.75m","16m","17.75m","20m","21.75m","24m","25.75m","26.75m","28m",
"28.25m","28.5m","0mgsimpleRHC","0mRHC","1.75mRHC","2mRHC","4mRHC","5.75mRHC","8mRHC","9.75mRHC","12mRHC","13.75mRHC","16mRHC","17.75mRHC","20mRHC","21.75mRHC","24mRHC",
"25.75mRHC","26.75mRHC","28mRHC","28.25mRHC","28.5mRHC"};
const int NUM_FIELDS=4;

Para pr[]= //position is in units of cm, momentum is in units of GeV/c, angle is in units of rad, and energy is in  units of GeV
{
  {"LepMomTot", 0., 10.,&TotalMom},
  {"cos_LepNuAngle", 0., 1.,&cos_angle},
  {"Ev", 0., 10., &ev},
  {"E_vis_true", 0., 10., &E_vis_true}
};

vector<Sel_type> br=
{
  Sel_type("muon_contained", "muon_contained_eff", false, &muon_cont, &muon_cont_eff),
  Sel_type("muon_tracker", "muon_tracker_eff", false, &muon_tra, &muon_tra_eff),
  Sel_type("muon_selected", "muon_sel_eff", true, &muon_sel, &muon_sel_eff),
  Sel_type("hadron_selected", "hadron_selected_eff", false, &hadr, &hadr_eff ),
  Sel_type("combined", "combined_eff", false, &comb, &comb_eff)
};

void histogram_files_ND_FNAL()
{
  // vector<double> geoeff_cut_threshold = {0.0, 0.005, 0.01, 0.1};
  vector<double> geoeff_cut_threshold = {0.1, 0.08, 0.06, 0.04, 0.02, 0.01};

  for (double geoeff_cut:geoeff_cut_threshold)
  {

    cout << "geoeff_cut: " << geoeff_cut << endl;

    // setup plots
    vector<TH1D*> histograms1, histograms2, histograms3;
    histograms1.clear();
    histograms2.clear();
    histograms3.clear();
    TH1::AddDirectory(false);

    br[0].sel_name = "muon_contained"; // Have to initialize the br first!!! Not sure the reason, but keep it!!!

    int first_pass=0;
    for(auto sel:br)
    {
      const char* dt=sel.sel_name;
      // cout << "sel_name: " << dt << ", sel_val: " << sel.sel_value << ", eff_name: " << sel.eff_name << ", eff_val: " << sel.eff_value << endl;
      for(auto item:pr)
      {
        char *fd=item.field;
        double l=item.l;
        double h=item.h; //insert 11 check
        if (first_pass<NUM_FIELDS) histograms1.push_back(new TH1D(Form("raw_%s", fd), Form("raw %s", fd), 200, l, h)); //remove dt from name
        histograms2.push_back(new TH1D(Form("selection-cut_%s_%s", dt, fd), Form("selected %s %s", dt, fd), 200, l, h));
        histograms3.push_back(new TH1D(Form("geo-corrected_%s_%s", dt, fd), Form("geo corrected %s %s", dt, fd), 200, l, h));
      }
      first_pass=1;
    }

    // Generate the required root files
    // Input FDroot file
    // TString FileIn = "/pnfs/dune/persistent/users/flynnguo/NDeff_muon/0mgsimple/NDGeoEff.root";
    TString FileIn = "/pnfs/dune/persistent/users/flynnguo/NDeff_muon/0mgsimple/NDGeoEff_51583010.root";
    // Read
    TChain *event_data = new TChain("event_data");
    event_data->Add(FileIn.Data());
    Long64_t nentries=event_data->GetEntries();
    cout << "nentries: " << nentries << endl;

    // Read four invariant variables
    int i_pr=0;
    for (auto item:pr)
    {
      event_data->SetBranchAddress(item.field, item.field_value);
      i_pr++;
    }

    for(auto sel:br)
    {
      // cout << "sel_name: " << sel.sel_name << ", sel_val: " << sel.sel_value << ", eff_name: " << sel.eff_name << ", eff_val: " << sel.eff_value << endl;

      if(sel.calced) continue;
      event_data->SetBranchAddress(sel.sel_name, sel.sel_value);
      event_data->SetBranchAddress(sel.eff_name, sel.eff_value);
    }

    event_data->SetBranchAddress("vtx_x", &x_pos);
    // Fill plots
    // Long64_t nentries=event_data->GetEntries();
    // cout << "nentries: " << nentries << endl;
    for (int i=0;i<nentries;i++) {
    // for (int i=0;i<10000;i++) {

      event_data->GetEntry(i);

      // // only pick center region
      // if (x_pos <= -50 || x_pos >= 50) continue;  // Skip values outside the (-50, 50) range
      // // cout << "x_pos: " << x_pos << endl;
      //
      // cout << "i_entry: " << i << endl;
      // cout << "LepE: " << LepE << ", eP: " << eP << ", ePip: " << ePip << ", ePim: " << ePim << ", ePi0: " << ePi0 << ", eOther: " << eOther << ", nipi0: " << nipi0 << endl;
      // cout << "E_vis_true_int: " << E_vis_true << endl;

      //calculation for the muon-selected cut
      muon_sel=muon_cont+muon_tra;
      cout<<"muon_sel: " << muon_cont + muon_tra << endl;
      muon_sel_eff=muon_cont_eff+muon_tra_eff;
      // Chek if muon_sel is neither 0 nor 1.
      // if (muon_sel!=0&&muon_sel!=1)
      // // if (muon_sel > 1 || muon_sel < 0)
      // {
      //   cout<<"bad val for muon-selected check! "<<muon_sel<<endl<<". Event # "<<i<<", contained: "<<muon_cont<<", tracker-matched: "<<muon_tra<<endl;
      //   continue;
      // }

      cout << "geoeff_cut: " << geoeff_cut << ", ientry: " << i << ", muon_sel: " << muon_sel << ", combined: " << comb << endl;
      int n=0;
      for (auto sel:br)
      {
        for (auto item:pr)
        {
          const char *fd=item.field;
          double geo_eff=*sel.eff_value;

          TH1D* hist1;
          if (n<NUM_FIELDS) hist1=histograms1.at(n);
          TH1D* hist2=histograms2.at(n);
          TH1D* hist3=histograms3.at(n);
          if (n<NUM_FIELDS) hist1->Fill(*item.field_value); // Raw
          n++;

          if (geo_eff<=geoeff_cut) {
            continue;
          } else {
            hist2->Fill(*item.field_value,*sel.sel_value); // Sel
            hist3->Fill(*item.field_value,*sel.sel_value/geo_eff); // Geo-corrected
          }
        }// end pr loop
      }// end br loop
    }


    TFile *raw_files[NUM_FIELDS];
    TFile *sel_files[int(NUM_FIELDS*5)];
    TFile *geo_files[int(NUM_FIELDS*5)];
    int index=0;
    for (auto sel:br) {
      const char *dt=sel.sel_name;
      for (Para item:pr) {
        const char *fd=item.field;

        // Create a folder before writting root file
        // gSystem->mkdir(TString::Format("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_center/%.3f_eff_veto_cut_ND/%s", geoeff_cut,fd), kTRUE); //  means only choose events w/ geoeff >=0
        gSystem->mkdir(TString::Format("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_test/%.3f_eff_veto_cut_ND/%s", geoeff_cut,fd), kTRUE); //  means only choose events w/ geoeff >=0

        if (index<NUM_FIELDS) {
          // raw_files[index]=new TFile(Form("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_center/%.3f_eff_veto_cut_ND/%s/raw_%s.root",geoeff_cut, fd,fd),"recreate");
          raw_files[index]=new TFile(Form("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_test/%.3f_eff_veto_cut_ND/%s/raw_%s.root",geoeff_cut, fd,fd),"recreate");
          TH1D* raw_hist=histograms1.at(index);
          raw_hist->Write();
          raw_files[index]->Close();
        }

        // sel_files[index]=new TFile(Form("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_center/%.3f_eff_veto_cut_ND/%s/selection-cut_%s_%s.root",geoeff_cut,fd,dt,fd),"recreate");
        sel_files[index]=new TFile(Form("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_test/%.3f_eff_veto_cut_ND/%s/selection-cut_%s_%s.root",geoeff_cut,fd,dt,fd),"recreate");
        TH1D* sel_hist=histograms2.at(index);
        sel_hist->Write();
        sel_files[index]->Close();

        // geo_files[index]=new TFile(Form("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_center/%.3f_eff_veto_cut_ND/%s/geo-corrected_%s_%s.root",geoeff_cut,fd,dt,fd),"recreate");
        geo_files[index]=new TFile(Form("/dune/app/users/flynnguo/NDeff_muon_Plots/0mgsimple_test/%.3f_eff_veto_cut_ND/%s/geo-corrected_%s_%s.root",geoeff_cut,fd,dt,fd),"recreate");
        TH1D* geo_hist=histograms3.at(index);
        geo_hist->Write();
        geo_files[index]->Close();
        index++;
      }
    }// end for (auto sel:br)

    histograms1.clear();
    histograms2.clear();
    histograms3.clear();

  }// end geoeff_cut loop

}
