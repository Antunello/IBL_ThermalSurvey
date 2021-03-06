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
#include <time.h>


struct module_data{
	std::string module;
	std::vector<double> temps;
	std::vector<double> temp_time;
	std::vector<double> powers;
	std::vector<double> power_time;
	TH1D* temp_distro;
};

struct run_time_interval{
	int run_number = 0;
	double start_time = 0;
	double stop_time = 0;

};

void fill_temp_map(TH2D* map, double temp, std::string module);
void draw_option_temp_map(TH2D* map);
double dayTime_in_sec(std::string dayTime);
unsigned long int date_in_days(std::string date);
void fill_time_interval(int run_number, run_time_interval RTI, std::string run_list_filname);
void select_data(run_time_interval RTI, std::stringstream& buffer, std::stringstream& selection);

void TemperatureStabilityForRun(int runNumber, std::string temp_file_name, std::string run_list);
void sort_module_data(std::stringstream& buffer, module_data* MD);


void TempStability(std::string temp_file_name, std::string run_list){
	std::vector<int> runs;
	std::string trash;	
	int run;

	ifstream runListFile;
	runListFile.open(run_list.c_str());
	std::stringstream buffer;
	buffer << runListFile.rdbuf();
	while(buffer){
		buffer>>run;
		buffer>>trash;
		buffer>>trash;
		runs.push_back(run);
		std::cout<<run<<std::endl;
	} 
	runListFile.close();
	
	for(std::vector<int>::iterator it = runs.begin(); it != runs.end(); ++it) {
//     std::cout << *it; ... 
		TemperatureStabilityForRun(*it, temp_file_name, run_list);
	}	
}


void draw_option_temp_map(TH2D* temp_map){
	temp_map->Draw();

	std::string labels[8] = {"M4C","M3C","M2C","M1C","M1A","M2A","M3A","M4A"};
	for(int i=1; i<=8; i++)temp_map->GetXaxis()->SetBinLabel(i,labels[i-1].c_str());
	temp_map->GetXaxis()->SetTitle("Sector");
	temp_map->GetYaxis()->SetTitle("Stave");
	temp_map->GetXaxis()->SetLabelSize(0.060);
	temp_map->GetYaxis()->SetLabelSize(0.060);
	temp_map->GetXaxis()->SetTitleSize(0.060);
	temp_map->GetYaxis()->SetTitleSize(0.060);
	temp_map->GetXaxis()->SetTitleOffset(0.8);

}

void fill_temp_map(TH2D* map, double temp, std::string module){
	int stave_id = atoi(module.substr(4,2).c_str());
	//std::cout<<"MODULE "<<module<<std::endl;
//	if(module.find("_C") != std::string::npos && !module_check) map->SetBinContent(1,stave_id, temp);	
	if(module.find("C_M4") != std::string::npos) map->SetBinContent(1,stave_id,temp);
	if(module.find("C_M3") != std::string::npos) map->SetBinContent(2,stave_id,temp);
	if(module.find("C_M2") != std::string::npos) map->SetBinContent(3,stave_id,temp);
	if(module.find("C_M1") != std::string::npos) map->SetBinContent(4,stave_id,temp);
	if(module.find("A_M1") != std::string::npos) map->SetBinContent(5,stave_id,temp);
	if(module.find("A_M2") != std::string::npos) map->SetBinContent(6,stave_id,temp);
	if(module.find("A_M3") != std::string::npos) map->SetBinContent(7,stave_id,temp);
	if(module.find("A_M4") != std::string::npos) map->SetBinContent(8,stave_id,temp);
//	if(module.find("_A") != std::string::npos && !module_check) map->SetBinContent(10,stave_id, temp);	

}

void fill_time_interval(int run_number, run_time_interval* RTI, std::string run_list_filename){
	//RTI.run_number = run_number;
	std::string rNumber, rStop, rStart;

	ifstream run_list;
	run_list.open(run_list_filename.c_str());

	if(run_list.is_open()){
		std::stringstream buffer;
		buffer << run_list.rdbuf();
			
		while(buffer){
		//	std::getline(buffer, rNumber, ',');
		//	std::getline(buffer, rStart, ',');
		//	std::getline(buffer, rStop);
		buffer>>rNumber;
		buffer>>rStart;
		buffer>>rStop;
			if(atoi(rNumber.c_str()) == run_number){
				RTI->run_number = atoi(rNumber.c_str());
				RTI->start_time = atof(rStart.c_str());
				RTI->stop_time = atof(rStop.c_str());
				std::cout<<run_number<<"\t"<<RTI->run_number<<"\t"<<RTI->start_time<<"\t"<<RTI->stop_time<<std::endl;
			} 
		}
	}
}

void select_data(run_time_interval RTI, std::stringstream& buffer, std::stringstream& selection){
//	std::stringstream selection;
	selection.str(std::string());
	std::string module, date, hours, timestamp;
	double data;
	while(buffer){
		buffer>>module;
		buffer>>data;		
		buffer>>date;
		buffer>>hours;
		//buffer>>data;		

		timestamp = date +"_" +hours.substr(0,8);

		struct tm tm;
		time_t epoch;
		if ( strptime(timestamp.c_str(), "%d-%m-%Y_%H:%M:%S", &tm) != NULL ){
			epoch = mktime(&tm);
			if(epoch>=RTI.start_time && epoch<=RTI.stop_time){
				selection<<module<<"\t"<<epoch<<"\t"<<data<<std::endl;
			}
		}
	}
}


void sort_module_data(std::stringstream& buffer, module_data* MD){
	int buffer_counter = 0;
	std::string module;
	int time;
	double temperature;

	int stave_id, stave_side_offset, module_id, m_index;
	
	//std::cout<<buffer.str();
	//buffer.clear();
	buffer.seekg(0,ios::beg);



	while(!buffer.eof()){
		buffer_counter++;
		buffer>>module;	
		//std::cout<<module<<std::endl;
		stave_id = atoi(module.substr(4,2).c_str());
		if(module.substr(7,1)=="A")stave_side_offset=0;
		else stave_side_offset=4;	
		module_id = atoi(module.substr(10,1).c_str());
		m_index = (stave_id-1)*8+stave_side_offset+(module_id-1);
		buffer>>time;
		buffer>>temperature;
		//std::cout<<time<<"\t"<<temperature<<std::endl;
		if(MD[m_index].temps.size()==0)MD[m_index].temp_distro = new TH1D(module.c_str(), module.c_str(), 1000, -30, 30);
		MD[m_index].temps.push_back(temperature);
		MD[m_index].temp_time.push_back(time);
		MD[m_index].module = module;
		MD[m_index].temp_distro->Fill(temperature);
		//std::cout<<MD[m_index].module<<std::endl;	
	}
}

void TemperatureStabilityForRun(int runNumber, std::string temp_file_name, std::string run_list){

//	std::string sel_module = "LI_S01_A_M1_TModule";
	
	module_data MD_sectors[112];

	//std::string time_interval = "0703-1603";
	//std::string temp_file_name = "M7_mod_temperature.txt";
	//std::string run_list = "M7_runQuery.txt";
	ifstream ddv_data_temp;
	TH2D* temp_mean_map = new TH2D("temp_mean_map", "temp_mean_map", 8, 0, 8, 14, 0.5, 14.5);
	TH2D* temp_rms_map = new TH2D("temp_RMS_map", "temp_RMS_map", 8, 0, 8, 14, 0.5, 14.5);
	TH1D* temp_mean_distro = new TH1D("temp_mean_distro", "temp_mean_distro", 180, -30, 30);
	TH1D* temp_rms_distro = new TH1D("temp_rms_distro", "temp_rms_distro", 180, 0, 0.5);


	std::stringstream output_file_name;
	output_file_name<<runNumber<<".root";

  std::string ofn = output_file_name.str();
  TFile *output_file = new TFile(ofn.c_str(), "RECREATE");
	run_time_interval rti;

	fill_time_interval(runNumber, &rti, run_list);
	std::cout<<"RUN NUMBER: "<<rti.run_number<<std::endl;
	ifstream temp_file;
	temp_file.open(temp_file_name.c_str());
	if(temp_file.is_open()){
		std::stringstream temp_buffer;
		temp_buffer << temp_file.rdbuf();
		std::stringstream temp_selection;
		select_data(rti, temp_buffer, temp_selection);
		//std::cout<<temp_selection.str();


		sort_module_data(temp_selection, MD_sectors);
		for(int i=0; i < 112; i++){
			MD_sectors[i].temp_distro->Write();
			temp_mean_distro->Fill(MD_sectors[i].temp_distro->GetMean());
			temp_rms_distro->Fill(MD_sectors[i].temp_distro->GetRMS());

			fill_temp_map(temp_mean_map, MD_sectors[i].temp_distro->GetMean(), MD_sectors[i].module);
			fill_temp_map(temp_rms_map, MD_sectors[i].temp_distro->GetRMS(), MD_sectors[i].module);
		}//	std::cout<<MD_sectors[i].module<<std::endl;

	}

	draw_option_temp_map(temp_mean_map);
	draw_option_temp_map(temp_rms_map);
	temp_mean_distro->Write();
	temp_rms_distro->Write();
	temp_mean_map->Write();
	temp_rms_map->Write();
	output_file->Close();


/*	ddv_data_temp.open(temp_file_name.c_str());
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
			if(module == sel_module && minutes>383800){

				std::cout<<module<<"\t"<<date<<"\t"<<hours<<"\t"<<temperature<<std::endl;
				temp_distro->Fill(temperature);
			}
		}
	}
	TCanvas *c1 = new TCanvas();
	temp_distro->Draw();

*/
}


unsigned long int date_in_days(std::string date){

	int day = atoi((date.substr(0,2)).c_str());
	int month = atoi((date.substr(3,2)).c_str());
	int year = atoi((date.substr(6,4)).c_str());

	int day_in_month[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
	int start_activities[3] = {1,6,2014};
	if(month==0)return 0;	


	unsigned long int SA_in_days = 22+day_in_month[5]+2014*365;

	unsigned long int tot_days = day + day_in_month[month-1] + year*365;
//	std::cout<<"********    "<<tot_days-SA_in_days<<std::endl;
	return tot_days-SA_in_days;

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
