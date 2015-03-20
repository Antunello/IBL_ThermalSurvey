#include <string>
#include <stdio.h>
//#include <stdlib.h>
#include "TH1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"

void PlotRuns(){
std::string run_list[4] = {"255313.root","255788.root", "255988.root", "256147.root"};
double setPoint_list[4] = {15.,7.,0.,-10.};
//std::vector<TFile*> file_list;
double mean_temp[4], mean_rms[4], delta_temp[4], rms_rms[4];


TFile* file;

	for(int i=0; i<4; i++){
		file = new TFile(run_list[i].c_str());
		TH1D* temp_mean_distro = (TH1D*)(file->Get("temp_mean_distro"));
		mean_temp[i] = temp_mean_distro->GetMean();


		TH1D* temp_rms_distro = (TH1D*)(file->Get("temp_rms_distro"));
		mean_rms[i] = temp_rms_distro->GetMean();
		rms_rms[i] = temp_rms_distro->GetRMS();
		delta_temp[i] = mean_temp[i]-setPoint_list[i];
		file->Close();
	}

TGraph *mTemp = new TGraph(4,setPoint_list, mean_temp);
TGraph *dTemp = new TGraph(4,setPoint_list, delta_temp);
TGraphErrors *rTemp = new TGraphErrors(4,setPoint_list, mean_rms,0, rms_rms);

TCanvas *c1 = new TCanvas("c1","c1", 1200, 600);
c1->Divide(2,1);
c1->cd(1);
mTemp->SetMarkerStyle(20);
mTemp->Draw("APL");
mTemp->GetXaxis()->SetTitle("Set Point temperature (C)");
mTemp->GetYaxis()->SetTitle("Temperature (C)");
dTemp->SetLineColor(kRed);
dTemp->SetMarkerStyle(21);
dTemp->SetMarkerColor(kRed);
dTemp->Draw("PL");
TLegend *leg = new TLegend(0.3,0.85,0.9,0.9);
leg->AddEntry(mTemp,"Module Mean Temperature", "lp");
leg->AddEntry(dTemp,"Module Mean Temperature - Set Point", "lp");
leg->Draw();

c1->cd(2);
rTemp->SetMarkerStyle(22);
rTemp->Draw("AP");
rTemp->GetXaxis()->SetTitle("Set Point temperature (C)");
rTemp->GetYaxis()->SetTitle("Module Mean RMS (C)");
rTemp->GetYaxis()->SetRangeUser(0.,0.1);
rTemp->GetYaxis()->SetTitleOffset(1.7);
}
