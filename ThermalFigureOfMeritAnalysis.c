#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "TGraph.h"
#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include <string>
#include <sstream>
#include <fstream>
#include "TMultiGraph.h"
#include "TPad.h"
#include "TLegend.h"
#include "THStack.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TColor.h"

double dayTime_in_sec(std::string dayTime);

unsigned long int date_in_days(std::string date);

void fill_temp_map(TH2D* map, double temp, std::string module, bool module_check = true);
void fill_power_map(TH2D* map, double power, std::string module);

void FicoPlot();

struct module_data{
	std::string module;
	std::vector<double> temps;
	std::vector<double> temp_time;
	std::vector<double> powers;
	std::vector<double> power_time;
};

struct cooling_data{
	std::string half_stave;
	std::vector<double> temps;
	std::vector<double> temp_time;
};




void FicoPlot(){

	const int NRGBs=5;
	const int NCont =255;
	double stops[NRGBs] = {0.00,0.34,0.61,1.00,1.00};
	double red[NRGBs] = {0.00,0.00,0.87,1.00,1.00};
	double green[NRGBs] = {0.00,1.00,0.80,0.20,0.00};
	double blue[NRGBs] ={1.00,1.00,0.20,0.00,0.00};
 
	TColor::CreateGradientColorTable(NRGBs,stops,red,green,blue,NCont);
	gStyle->SetNumberContours(NCont);
	//gPad->SetLogz(0);
	gStyle->SetOptStat(0);
//	gPad->SetRightMargin(0.15);
  gStyle->SetPaintTextFormat("2.2f");

}
void fill_power_map(TH2D* map, double power, std::string module){
	int stave_id = atoi(module.substr(4,2).c_str());
	if(module.find("C_M4") != std::string::npos) map->SetBinContent(1,stave_id,power);
	if(module.find("C_M3") != std::string::npos) map->SetBinContent(2,stave_id,power);
	if(module.find("C_M2") != std::string::npos) map->SetBinContent(3,stave_id,power);
	if(module.find("C_M1") != std::string::npos) map->SetBinContent(4,stave_id,power);
	if(module.find("A_M1") != std::string::npos) map->SetBinContent(5,stave_id,power);
	if(module.find("A_M2") != std::string::npos) map->SetBinContent(6,stave_id,power);
	if(module.find("A_M3") != std::string::npos) map->SetBinContent(7,stave_id,power);
	if(module.find("A_M4") != std::string::npos) map->SetBinContent(8,stave_id,power);
}

void fill_temp_map(TH2D* map, double temp, std::string module, bool module_check){
	int stave_id = atoi(module.substr(4,2).c_str());
	//std::cout<<"MODULE "<<module<<std::endl;
	if(module.find("_C") != std::string::npos && !module_check) map->SetBinContent(1,stave_id, temp);	
	if(module.find("C_M4") != std::string::npos && module_check) map->SetBinContent(2,stave_id,temp);
	if(module.find("C_M3") != std::string::npos && module_check) map->SetBinContent(3,stave_id,temp);
	if(module.find("C_M2") != std::string::npos && module_check) map->SetBinContent(4,stave_id,temp);
	if(module.find("C_M1") != std::string::npos && module_check) map->SetBinContent(5,stave_id,temp);
	if(module.find("A_M1") != std::string::npos && module_check) map->SetBinContent(6,stave_id,temp);
	if(module.find("A_M2") != std::string::npos && module_check) map->SetBinContent(7,stave_id,temp);
	if(module.find("A_M3") != std::string::npos && module_check) map->SetBinContent(8,stave_id,temp);
	if(module.find("A_M4") != std::string::npos && module_check) map->SetBinContent(9,stave_id,temp);
	if(module.find("_A") != std::string::npos && !module_check) map->SetBinContent(10,stave_id, temp);	

}



void TFoM(){


	TMultiGraph* graphs = new TMultiGraph();
	//TMultiGraph* module_temp_graphs = new TMultiGraph();

	TH1D* power_distro = new TH1D("powerDistro","powerDistro", 80,0,10);
	THStack* temp_distro = new THStack();
	TH1D* temp_distro_M1 = new TH1D("tempDistro_M1","tempDistro_M1", 180,-30,30);
	TH1D* temp_distro_M2 = new TH1D("tempDistro_M2","tempDistro_M2", 180,-30,30);
	TH1D* temp_distro_M3 = new TH1D("tempDistro_M3","tempDistro_M3", 180,-30,30);
	TH1D* temp_distro_M4 = new TH1D("tempDistro_M4","tempDistro_M4", 180,-30,30);
	TH2D* temp_map = new TH2D("tempMap", "tempMap", 10, 0, 10, 14, 0.5, 14.5);
	TH2D* cool_temp_map = new TH2D("cool_tempMap", "cool_tempMap", 10, 0, 10, 14, 0.5, 14.5);
	TH2D* power_map = new TH2D("powerMap", "powerMap", 8, 0, 8, 14, 0.5, 14.5);
	TH2D* tfom_map = new TH2D("tfomMap", "tfomMap", 8, 0, 8, 14, 0.5, 14.5);

	TH1D* cool_distro = new TH1D("coolDistro", "coolDistro", 180,-30,30);
	TH1D* tfom = new TH1D("tfom","tfom", 30, 0,30);	

	module_data MD_sectors[112];
	cooling_data CD_halfstave[28];

	// timestamp: M6 = 166550 | M7 = 232000 | M8 = 342000 | M9 = 378800 
	int timestamp = 384000;
	ifstream ddv_data_temp;
	// time_interval M6 = "1310-1710" | M7 = "2011-1612" | M8 = "2301-2402" "2402-0103" | M9 = "1103-1203" "0703-1403" "0703-1603"
	std::string time_interval = "0703-1603";
	std::string temp_file_name = "IBL_temperature_"+time_interval+".txt";
	std::string power_file_name = "IBL_power_"+time_interval+".txt";
	std::string cool_file_name = "IBL_coolingPipe_"+time_interval+".txt";

	std::string output_file_name = time_interval+".root";
	TFile *output_file = new TFile(output_file_name.c_str(), "RECREATE");
	ddv_data_temp.open(temp_file_name.c_str());

	if(ddv_data_temp.is_open()){
		std::stringstream buffer_temp;
		buffer_temp << ddv_data_temp.rdbuf();

		std::string module, date, hours;
		double temperature, minutes;
		int stave_id, module_id, stave_side_offset;
		int buffer_counter = 0;
		int m_index;
		while(buffer_temp){
			buffer_counter++;
			buffer_temp>>module;	
			stave_id = atoi(module.substr(4,2).c_str());
			if(module.substr(7,1)=="A")stave_side_offset=0;
			else stave_side_offset=4;	
			module_id = atoi(module.substr(10,1).c_str());
			m_index = (stave_id-1)*8+stave_side_offset+(module_id-1);
			buffer_temp>>date;
			buffer_temp>>hours;	
			buffer_temp>>temperature;
			minutes = (double) date_in_days(date)*1440+(double)dayTime_in_sec(hours);	
			//std::cout<<date<<"\t"<<hours<<"\t"<<minutes<<std::endl;
			MD_sectors[m_index].temps.push_back(temperature);
			MD_sectors[m_index].temp_time.push_back(minutes);
			MD_sectors[m_index].module = module;
		}
	}

	ifstream ddv_data_power;
	ddv_data_power.open(power_file_name.c_str());

	if(ddv_data_power.is_open()){
		std::stringstream buffer_power;
		buffer_power << ddv_data_power.rdbuf();

		std::string module, date, hours;
		double power, minutes;
		int stave_id, module_id, stave_side_offset;
		int buffer_counter = 0;
		int m_index;
		while(buffer_power){
			buffer_counter++;
			buffer_power>>module;	
			stave_id = atoi(module.substr(4,2).c_str());
			if(module.substr(7,1)=="A")stave_side_offset=0;
			else stave_side_offset=4;	
			module_id = atoi(module.substr(10,1).c_str());
			m_index = (stave_id-1)*8+stave_side_offset+(module_id-1);
			buffer_power>>date;
			buffer_power>>hours;	
			buffer_power>>power;
			minutes = (double) date_in_days(date)*1440+(double)dayTime_in_sec(hours);	
			int t_inf= timestamp-100;
			int t_sup= timestamp+100;
			if(t_inf <=minutes && t_sup>=minutes)std::cout<<minutes<<"\t"<<date<<"\t"<<hours<<std::endl;
			MD_sectors[m_index].powers.push_back(power);
			MD_sectors[m_index].power_time.push_back(minutes);
		}
	}

	ifstream ddv_data_cooling;
	ddv_data_cooling.open(cool_file_name.c_str());

	if(ddv_data_cooling.is_open()){
		std::stringstream buffer_cooling;
		buffer_cooling << ddv_data_cooling.rdbuf();

		std::string half_stave, date, hours, trash;
		double temp, minutes;
		int stave_id, stave_side_offset;
		int buffer_counter = 0;
		int m_index;
		while(buffer_cooling){
			buffer_counter++;
			buffer_cooling >> trash;
			buffer_cooling >> trash;
			buffer_cooling >> trash;
			buffer_cooling >> half_stave;
			stave_id = atoi(half_stave.substr(4,2).c_str());
			if(half_stave.substr(7,1)=="A")stave_side_offset=0;
			else stave_side_offset=1;	
			m_index = (stave_id-1)*2+stave_side_offset;	
			buffer_cooling >> date;
			buffer_cooling >> hours;
			buffer_cooling >> temp;
			minutes = (double)date_in_days(date)*1440+(double)dayTime_in_sec(hours);
			CD_halfstave[m_index].temps.push_back(temp);
			CD_halfstave[m_index].temp_time.push_back(minutes);	
			CD_halfstave[m_index].half_stave = half_stave;
//			std::cout<<"ECCOMI"<<std::endl;
		}
	}

	double cool_temp, cool_step, cool_temp_A, cool_temp_C, mod_temp, mod_power, tfom_value;

	for(int i=0; i<112; i++){
		int s_id = (i/8)+1;
		int counter = (s_id-1)*2;
		int m_id;
		if(MD_sectors[i].module.find("_C_")!=std::string::npos)m_id = i%8;
		else m_id = 3-(i%8);
		//if(s_id%2==0) m_id = (i%8)+1;
		//else m_id = 8 -(i%8);

		int j=i/4;
		int k=i%4;
	
		if(i%8==0){
			std::string A_string = CD_halfstave[counter].half_stave+".cool";
			std::string C_string = CD_halfstave[counter+1].half_stave+".cool";

			TGraph* graph_cool_A = new TGraph(CD_halfstave[counter].temps.size()-1, &(CD_halfstave[counter].temp_time[0]), &(CD_halfstave[counter].temps[0]));
			TGraph* graph_cool_C = new TGraph(CD_halfstave[counter+1].temps.size()-1, &(CD_halfstave[counter+1].temp_time[0]), &(CD_halfstave[counter+1].temps[0]));
			graph_cool_C->SetTitle(C_string.c_str());
			graph_cool_C->SetLineColor(kOrange+8);
			graph_cool_A->SetTitle(A_string.c_str());
			graph_cool_A->SetLineColor(kOrange+8);
			graphs->Add(graph_cool_C);
			graphs->Add(graph_cool_A);
			cool_temp_A = graph_cool_A->Eval(timestamp);
			cool_temp_C = graph_cool_C->Eval(timestamp);
			cool_distro->Fill(cool_temp_A);
			cool_distro->Fill(cool_temp_C);
			fill_temp_map(temp_map, cool_temp_A, CD_halfstave[counter].half_stave, false);
			fill_temp_map(temp_map, cool_temp_C, CD_halfstave[counter+1].half_stave, false);
			fill_temp_map(cool_temp_map, cool_temp_A, CD_halfstave[counter].half_stave, false);
			fill_temp_map(cool_temp_map, cool_temp_C, CD_halfstave[counter+1].half_stave, false);
			//std::cout<<"Cool A: "<<cool_temp_A<<"\tCool C: "<<cool_temp_C<<"Stave: "<<s_id<<std::endl;		
		}

		cool_step = (cool_temp_A-cool_temp_C)/9;
		cool_temp = cool_temp_C + cool_step*(9-(m_id+1));
		//std::cout<<"STAVE ID: "<<s_id<<"\tModule id"<<m_id<<"\tSECTOR:"<<MD_sectors[i].module<<"Cooling pipe A: "<<cool_temp_A<<" | Estimation: "<<cool_temp<<" | Cooling pipe C: "<<cool_temp_C<<std::endl;
		//std::cout<<"COOL TEMP = "<<cool_temp<<"\tSTAVE ID:"<<s_id<<std::endl;
		std::string t_string = MD_sectors[i].module+".temp";
		std::string p_string = MD_sectors[i].module+".power";

		TGraph* graph_t = new TGraph(MD_sectors[i].temps.size()-1, &(MD_sectors[i].temp_time[0]), &(MD_sectors[i].temps[0]));			
		graph_t->SetTitle(t_string.c_str());
		TGraph* graph_p = new TGraph(MD_sectors[i].powers.size()-1, &(MD_sectors[i].power_time[0]), &(MD_sectors[i].powers[0]));			
		graph_p->SetTitle(p_string.c_str());
		graph_p->SetLineColor(kRed);
		graphs->Add(graph_t);
		graphs->Add(graph_p);

		mod_temp = graph_t->Eval(timestamp);
		mod_power = graph_p->Eval(timestamp);

		// FOOT print of each FE is 3.15cm^2
		tfom_value = (mod_temp-cool_temp)/(mod_power/(12.6));
		//std::cout<<mod_temp<<"\t"<<cool_temp<<"\t"<<mod_power<<"\t"<<tfom_value<<std::endl;
		tfom->Fill(tfom_value);
		if(MD_sectors[i].module.find("M1") != std::string::npos)	temp_distro_M1->Fill(mod_temp);
		if(MD_sectors[i].module.find("M2") != std::string::npos)	temp_distro_M2->Fill(mod_temp);
		if(MD_sectors[i].module.find("M3") != std::string::npos)	temp_distro_M3->Fill(mod_temp);
		if(MD_sectors[i].module.find("M4") != std::string::npos)	temp_distro_M4->Fill(mod_temp);
		fill_temp_map(temp_map,mod_temp,MD_sectors[i].module);
		fill_temp_map(cool_temp_map,cool_temp,MD_sectors[i].module);
		fill_power_map(power_map, mod_power, MD_sectors[i].module);
		fill_power_map(tfom_map, tfom_value, MD_sectors[i].module);
		power_distro->Fill(mod_power);

	}


	
	TCanvas *c1 = new TCanvas("canvas_1","canvas_1",1600,800);
	
	c1->Divide(2,2);
	c1->cd(1);
	graphs->Draw("AL");
	graphs->GetYaxis()->SetRangeUser(-20,20);
	//graphs->GetXaxis()->SetRangeUser(330000,355000);
	graphs->Draw("AL");

	c1->cd(2);
	tfom->Draw();
	tfom->GetXaxis()->SetTitle("Thermal Figure of Merit [C*cm^2/W]");
	tfom->Write();

	c1->cd(3);
	temp_distro_M1->SetFillColor(kRed);
	temp_distro_M2->SetFillColor(kOrange+2);
	temp_distro_M3->SetFillColor(kGreen+2);
	temp_distro_M4->SetFillColor(kBlue);

	temp_distro_M1->SetLineColor(kRed);
	temp_distro_M2->SetLineColor(kOrange+2);
	temp_distro_M3->SetLineColor(kGreen+2);
	temp_distro_M4->SetLineColor(kBlue);

	temp_distro_M1->SetFillStyle(3004);
	temp_distro_M2->SetFillStyle(3004);
	temp_distro_M3->SetFillStyle(3004);
	temp_distro_M4->SetFillStyle(3004);

	temp_distro->Add(temp_distro_M1);
	temp_distro->Add(temp_distro_M2);
	temp_distro->Add(temp_distro_M3);
	temp_distro->Add(temp_distro_M4);
	temp_distro->Draw();
	temp_distro->GetXaxis()->SetTitle("Temperature [C]");

	cool_distro->SetLineColor(kBlack);
	cool_distro->SetFillColor(kBlack);
	cool_distro->SetFillStyle(3001);
	cool_distro->Draw("same");

	std::cout<<"MEAN COOLING PIPE TEMPERATURE: "<<cool_distro->GetMean()<<"\tRMS COOLING PIPE TEMPERATURE: "<<cool_distro->GetRMS()<<std::endl;
	//std::cout<<"MEAN M1 TEMPERATURE: "<<temp_distro_M1->GetMean()<<"\tRMS M1 TEMPERATURE: "<<temp_distro_M1->GetRMS()<<std::endl;
	//std::cout<<"MEAN M2 TEMPERATURE: "<<temp_distro_M2->GetMean()<<"\tRMS M2 TEMPERATURE: "<<temp_distro_M2->GetRMS()<<std::endl;
	//std::cout<<"MEAN M3 TEMPERATURE: "<<temp_distro_M3->GetMean()<<"\tRMS M4 TEMPERATURE: "<<temp_distro_M3->GetRMS()<<std::endl;
	//std::cout<<"MEAN M4 TEMPERATURE: "<<temp_distro_M4->GetMean()<<"\tRMS M4 TEMPERATURE: "<<temp_distro_M4->GetRMS()<<std::endl;
	double temp_mean = (temp_distro_M1->GetMean() + temp_distro_M2->GetMean() + temp_distro_M3->GetMean() + temp_distro_M4->GetMean() ) / 4;
	double temp_rms = sqrt( (pow(temp_distro_M1->GetRMS(),2) + pow(temp_distro_M2->GetRMS(),2) + pow(temp_distro_M3->GetRMS(),2) + pow(temp_distro_M4->GetRMS(),2))/4 );
	std::cout<<"MEAN MODULES TEMPERATURE: "<<temp_mean<<"\tRMS MODULES TEMPERATURE: "<<temp_rms<<std::endl;
	


	TLegend *distro_leg = new TLegend(0.7,0.7,0.9,0.9);
	distro_leg->AddEntry(temp_distro_M1,"M1 temp","F");	
	distro_leg->AddEntry(temp_distro_M2,"M2 temp","F");	
	distro_leg->AddEntry(temp_distro_M3,"M3 temp","F");	
	distro_leg->AddEntry(temp_distro_M4,"M4 temp","F");	
	distro_leg->AddEntry(cool_distro,"Cooling temp","F");	
	distro_leg->Draw();

	c1->cd(4);
	power_distro->Draw();
	power_distro->GetXaxis()->SetTitle("Power distro [W]");
//	c1->cd(3);

	TCanvas *c2 = new TCanvas("canvas","canvas", 1200, 800);
	FicoPlot();
	c2->Divide(2,2);
	c2->cd(1);
	temp_map->SetMarkerColor(kWhite);
	temp_map->SetMarkerSize(0.0000000005);
	temp_map->Draw("colztext");
	std::string labels[10] = {"CoolPipe C","M4C","M3C","M2C","M1C","M1A","M2A","M3A","M4A","CoolPipe A"};
	for(int i=1; i<=10; i++)temp_map->GetXaxis()->SetBinLabel(i,labels[i-1].c_str());
	temp_map->GetXaxis()->SetTitle("Sector");
	temp_map->GetYaxis()->SetTitle("Stave");
	temp_map->GetXaxis()->SetLabelSize(0.060);
	temp_map->GetYaxis()->SetLabelSize(0.060);
	temp_map->GetXaxis()->SetTitleSize(0.060);
	temp_map->GetYaxis()->SetTitleSize(0.060);
	temp_map->GetXaxis()->SetTitleOffset(0.8);
	temp_map->GetYaxis()->SetTitleOffset(0.8);
	//temp_map->GetZaxis()->SetRangeUser(-20,-10);
	c2->cd(2);
	power_map->SetMarkerColor(kWhite);
	power_map->SetMarkerSize(0.0000000005);
	power_map->Draw("colztext");
	power_map->GetXaxis()->SetTitle("Sector");
	power_map->GetYaxis()->SetTitle("Stave");
	power_map->GetZaxis()->SetRangeUser(2.5,4);
	for(int i=1; i<=8; i++)power_map->GetXaxis()->SetBinLabel(i, labels[i].c_str());
	power_map->GetXaxis()->SetLabelSize(0.060);
	power_map->GetYaxis()->SetLabelSize(0.060);
	power_map->GetXaxis()->SetTitleSize(0.060);
	power_map->GetYaxis()->SetTitleSize(0.060);
	power_map->GetXaxis()->SetTitleOffset(0.8);
	power_map->GetYaxis()->SetTitleOffset(0.8);
	c2->cd(3);
	tfom_map->SetMarkerColor(kWhite);
	tfom_map->SetMarkerSize(0.0000000005);
	tfom_map->Draw("colztext");
	for(int i=1; i<=8; i++)tfom_map->GetXaxis()->SetBinLabel(i, labels[i].c_str());
	tfom_map->GetXaxis()->SetTitle("Sector");
	tfom_map->GetYaxis()->SetTitle("Stave");
	tfom_map->GetXaxis()->SetLabelSize(0.060);
	tfom_map->GetYaxis()->SetLabelSize(0.060);
	tfom_map->GetXaxis()->SetTitleSize(0.060);
	tfom_map->GetYaxis()->SetTitleSize(0.060);
	tfom_map->GetXaxis()->SetTitleOffset(0.8);
	tfom_map->GetYaxis()->SetTitleOffset(0.8);
	tfom_map->GetZaxis()->SetRangeUser(0,40);
	c2->cd(4);
	TMultiGraph *slopes = new TMultiGraph();
	TLegend *leg = new TLegend(0.85,0.2,1,0.9);
	//std::vector<TGraph*> s_graph;
	for(int i=0; i<14; i++){
		std::vector<double> tt, hh;
		stringstream ss;
		ss<<"stave_id_"<<i+1;
		std::string s = ss.str();
		for(int j=0; j<8; j++){
			//temp_map->GetBinContent(j+2,i+1);
			tt.push_back(temp_map->GetBinContent(j+2, i+1));
			if(i%2==0) hh.push_back(7-j);
			else hh.push_back(j);
		}
		TGraph* gg = new TGraph(8,&hh[0],&tt[0]);
		gg->SetLineColor((i%9)+1);
		slopes->Add(gg);
		leg->AddEntry(gg,s.c_str(),"l");
	}
//	slopes->SetLineStyle(3);
	slopes->Draw("ALM");
	slopes->GetYaxis()->SetTitle("Temperature");
	slopes->GetXaxis()->SetTitle("Distance from Inlet Cool Pipe");
	slopes->Draw("ALM");
	leg->Draw();

//	cool_temp_map->Draw("colztext");	
//	c2->cd(4);

//	THStack *slope_pari = new THStack();
//	for(int i=0; i<7; i++){
//		stringstream ss;
//		ss<<i;
//		std::string s = ss.str();
//		slope_pari->Add(temp_map->ProjectionX(s.c_str(), (i*2), (i*2)));
//	}
//	slope_pari->Draw("Lnostack");
}




double dayTime_in_sec(std::string dayTime){

//	std::cout<<dayTime<<std::endl;
	double hour =(double) atoi(dayTime.substr(0,2).c_str());
	double min =(double) atoi(dayTime.substr(3,2).c_str());
	double sec =(double) atoi(dayTime.substr(6,2).c_str());
	double msec =(double) atoi(dayTime.substr(9,3).c_str());
	double buff =(double) hour*60 + min + sec/60 + msec/60000;
//	std::cout<<hour<<"\t"<<min<<"\t"<<sec<<"\tbuff: "<<buff<<std::endl;
	return buff;


}

unsigned long int date_in_days(std::string date){

	int day = atoi((date.substr(0,2)).c_str());
	int month = atoi((date.substr(3,2)).c_str());
	int year = atoi((date.substr(6,4)).c_str());

	int day_in_month[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
	int start_activities[3] = {22,6,2014};
	if(month==0)return 0;	


	unsigned long int SA_in_days = 22+day_in_month[5]+2014*365;

	unsigned long int tot_days = day + day_in_month[month-1] + year*365;
//	std::cout<<"********    "<<tot_days-SA_in_days<<std::endl;
	return tot_days-SA_in_days;
}



