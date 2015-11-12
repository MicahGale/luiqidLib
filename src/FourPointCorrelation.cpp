//
//  FourPointCorrelation.cpp
//
//  Copyright (c) 2015 Zhang-Group. All rights reserved.
//  This software is distributed under the MIT license
//  -----------------------------------------------------
//  Contributing authors: Zhikun Cai,
//                        Abhishek Jaiswal,
//                        Nathan Walter
//  -----------------------------------------------------
//
#include "FourPointCorrelation.hpp"

#ifdef OMP
#include "omp.h"
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cassert>
#include <vector>
#include <iomanip>
#include <cstring>

#include "Trajectory.hpp"

using namespace std;

FourPointCorrelation::FourPointCorrelation() :
    number_of_time_points_(0),
    number_of_frames_to_average_(1),
    frame_interval_(1.0),
    overlap_length_(1.0),
    time_scale_type_("linear"),
    output_file_name_("chi4_t.txt"),
    input_file_name_("chi4_t.in"),
    atom_type_("all"),
    atom_group_("system")
{
}


FourPointCorrelation::~FourPointCorrelation()
{
}


void FourPointCorrelation::read_command_inputs(int argc, char * argv[])
{
    for (int input = 1; input < argc; ++input) {
        if (strcmp(argv[input], "-i") == 0) {
            input_file_name_ = argv[++input];
            continue;
        }
        if (strcmp(argv[input], "-o") == 0) {
            output_file_name_ = argv[++input];
            continue;
        }
        if (strcmp(argv[input], "-t") == 0) {
            trajectory_file_name_ = argv[++input];
            continue;
        }
        if (strcmp(argv[input], "-v") == 0) {
            is_run_mode_verbose_ = 1;
            continue;
        }
        
        cerr << "\nERROR: Unrecognized flag '" << argv[input] << "' from command inputs.\n";
        exit(1);
    }
}


void FourPointCorrelation::read_input_file()
{
    ifstream input_file(input_file_name_);
    
    if (!input_file) {
        cerr << "ERROR: Input file location, ";
        cerr << "\033[1;25m";
        cerr << input_file_name_;
        cerr << "\033[0m";
        cerr << ", does not exist, please check input";
        cerr << endl;
        exit(1);
    }
    
    
    string input_word;
    
    while (input_file >> input_word) {
        //check for comment
        if (input_word[0] == '#') {
            getline(input_file, input_word);
            continue;
        }
        
        //check for member bools
        if (input_word == "is_run_mode_verbose") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            if (input_word == "true" || input_word == "yes") {
                is_run_mode_verbose_ = true;
            }
            else if(input_word == "false" || input_word == "no") {
                is_run_mode_verbose_ = false;
            }
            else {
                is_run_mode_verbose_ = stoi(input_word);
            }
            continue;
        }
        if (input_word == "is_wrapped") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            if (input_word == "true" || input_word == "yes") {
                is_wrapped_ = true;
            }
            else if(input_word == "false" || input_word == "no") {
                is_wrapped_ = false;
            }
            else {
                is_wrapped_ = stoi(input_word);
            }
            continue;
        }
        
        //check if equal to member ints
        if (input_word ==  "number_of_time_points") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            number_of_time_points_ = stoi(input_word);
            continue;
        }
        if (input_word ==  "number_of_frames_to_average") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            number_of_frames_to_average_ = stoi(input_word);
            continue;
        }
        if (input_word == "start_frame") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            start_frame_ = stoi(input_word);
            continue;
        }
        if (input_word == "end_frame") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            end_frame_ = stoi(input_word);
            continue;
        }
        if (input_word == "dimension") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            dimension_ = stoi(input_word);
            continue;
        }
        
        //check if equal to member doubles
        if (input_word == "frame_interval") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            frame_interval_ = stod(input_word);
            continue;
        }
        if (input_word == "overlap_length") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            overlap_length_ = stod(input_word);
            continue;
        }
        if (input_word == "trajectory_delta_time") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            trajectory_delta_time_ = stod(input_word);
            continue;
        }
        
        //check if equal to member strings
        if (input_word == "time_scale_type") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            time_scale_type_ = input_word;
            continue;
        }
        if (input_word == "output_file_name") {
            if (output_file_name_ != "chi4_t.txt") {
                cerr << "ERROR: Please do not set output file by command line and input file,\n";
                cerr << "     : we are unsure on which to prioritize.";
                cerr << endl;
                exit(1);
            }
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            output_file_name_ = input_word;
            continue;
        }
        if (input_word == "trajectory_file_name") {
            if (trajectory_file_name_ != "") {
                cerr << "ERROR: Please do not set trajectory file by command line and input file,\n";
                cerr << "     : we are unsure on which to prioritize.";
                cerr << endl;
                exit(1);
            }
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            trajectory_file_name_ = input_word;
            continue;
        }
#ifdef GROMACS
        if (input_word == "gro_file_name") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            gro_file_name_ = input_word;
            continue;
        }
#else
        if (input_word == "gro_file_name") {
            cerr << "gro files cannot be used in non gromacs";
            cerr << "compatible version of LiquidLib\n";
            cerr << endl;
        }
#endif
        if (input_word == "atom_type") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            atom_type_ = input_word;
            continue;
        }
        if (input_word == "atom_group") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            atom_group_ = input_word;
            continue;
        }
        if (input_word == "trajectory_data_type") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            trajectory_data_type_ = input_word;
            continue;
        }
        if (input_word == "output_precision") {
            input_file >> input_word;
            if (input_word[0] == '=') {
                input_file >> input_word;
            }
            output_precision_ = stod(input_word);
            continue;
        }
        
        //check for everything else
        
        cerr << "WARNING: no matching input type for: ";
        cerr << "\033[1;33m";
        cerr << input_word;
        cerr << "\033[0m";
        cerr << " disregarding this variable and continueing to next line\n";
        getline(input_file, input_word);
    }
    check_parameters();
    
    input_file.close();
}


void FourPointCorrelation::compute_chi4_t()
{
    if (is_wrapped_) {
        unwrap_coordinates();
    }
    
    chi4_t_.resize(number_of_time_points_, vector< double > (2, 0.0));
    
    // Form Array of time index values for a given type of timescale computation
    compute_time_array();
    
    vector< unsigned int > atom_type_indexes;
    select_atoms(atom_type_indexes, atom_type_, atom_group_);

    // Normalization factor
    double const normalization_factor = 1.0/(atom_type_indexes.size() * number_of_frames_to_average_);
    double system_volume = 1.0;
    for (size_t i_dimension = 0; i_dimension < dimension_; ++i_dimension) {
        system_volume *= average_box_length_[i_dimension];
    }
    
    cout << setiosflags(ios::fixed);
    cout << setprecision(4);
    
    int status = 0;
    cout << "Computing ..." << endl;
    
    // Perform time averaging of Four Point Correlation Function
#pragma omp parallel for
    for (size_t time_point = 0; time_point < number_of_time_points_; ++time_point) {
        double q_self = 0.0;
        double q_self2 = 0.0;
        for (size_t initial_frame = 0; initial_frame <  number_of_frames_to_average_; ++initial_frame) {
            size_t current_frame = initial_frame + time_array_indexes_[time_point];
            double q_self_tmp = 0.0;
            for (size_t i_atom = 0; i_atom < atom_type_indexes.size(); ++i_atom) {
                size_t atom_index = atom_type_indexes[i_atom];
                double r_squared = 0.0;
                for (size_t i_dimension = 0; i_dimension < dimension_; ++i_dimension) {
                    double delta_r = trajectory_[current_frame][atom_index][i_dimension] - trajectory_[initial_frame][atom_index][i_dimension];
                    r_squared += delta_r * delta_r;
                }
                if (sqrt(r_squared) <= overlap_length_) {
                    q_self_tmp += 1.0;
                }
            }
            q_self += q_self_tmp;
            q_self2 += q_self_tmp * q_self_tmp;
        }
        
        // Normalization
        chi4_t_[time_point][0] = q_self * normalization_factor;
        chi4_t_[time_point][1] = system_volume * (q_self2 * normalization_factor / atom_type_indexes.size() - chi4_t_[time_point][0] * chi4_t_[time_point][0]);

        if (is_run_mode_verbose_) {
#pragma omp critical
            {
                ++status;
                cout << "\rcurrent progress of calculating chi4 is: ";
                cout << status * 100.0/number_of_time_points_;
                cout << " \%";
                cout << flush;
            }
        }
    }
    cout << endl;
}


void FourPointCorrelation::write_chi4_t()
{
    if (trajectory_delta_time_ == 0.0) {
        cerr << "WARNING: time step of simulation could not be derived from trajectory,\n";
        cerr << "       : and was not provided by input, will use time step of: ";
        cerr << "\033[1;25m" << "1 (step/a.u.)" << "\033[0m\n\n";
        trajectory_delta_time_ = 1.0;
    }
    
    ofstream output_chi4_t_file(output_file_name_);
    
    if (!output_chi4_t_file) {
        cerr << "ERROR: Output file:" << "\033[1;25m" << output_file_name_ << "\033[0m";
        cerr << ", could not be opened." << endl;
        exit(1);
    }
    output_chi4_t_file << setiosflags(ios::scientific) << setprecision(output_precision_);
    output_chi4_t_file << "#Four point correlation function for ";
    output_chi4_t_file << atom_type_ << " atoms of group " << atom_group_ << "\n";
    output_chi4_t_file << "#using " << time_scale_type_ << "scale\n";
    output_chi4_t_file << "#time        q_self_t        chi4_t \n";
    
    for (size_t time_point = 0; time_point < number_of_time_points_; ++time_point) {
        output_chi4_t_file << time_array_indexes_[time_point] * trajectory_delta_time_ << "        ";
        output_chi4_t_file << chi4_t_[time_point][0] << "        ";
        output_chi4_t_file << chi4_t_[time_point][1] << "\n";
    }
    
    output_chi4_t_file.close();
}


void FourPointCorrelation::check_parameters() throw()
{
    if (end_frame_ == 0 && number_of_time_points_ == 0) {
        cerr << "ERROR: We require more information to proceed, either frameend or numberoftimepoints\n";
        cerr << "       must be povided for us to continue.";
        cerr << endl;
        exit(1);
    }
    
    if (time_scale_type_ == "linear") {
        if (end_frame_ == 0) {
            end_frame_ = start_frame_ + number_of_time_points_*frame_interval_ + number_of_frames_to_average_;
        }
        if (number_of_time_points_ == 0) {
            number_of_time_points_ = (end_frame_ - start_frame_ - number_of_frames_to_average_)/frame_interval_;
        }
        if (number_of_time_points_*frame_interval_ + number_of_frames_to_average_ > end_frame_ - start_frame_) {
            end_frame_ = start_frame_ + number_of_time_points_*frame_interval_ + number_of_frames_to_average_;
            cerr << "WARNING: the number of frames required is greater then the number supplied\n";
            cerr << "       : setting end frame to minimum value allowed: ";
            cerr << end_frame_;
            cerr << endl;
        }
        if (frame_interval_ < 1) {
            cerr << "ERROR: frame_interval must be an integer greater than 0 for linear scale\n" << endl;
            exit(1);
        }
    }
    else if (time_scale_type_ == "log") {
        if (end_frame_ == 0) {
            end_frame_ = start_frame_ + pow(frame_interval_,number_of_time_points_)  + number_of_frames_to_average_;
        }
        if (static_cast<unsigned int>(pow(frame_interval_, number_of_time_points_) + 0.5) + number_of_frames_to_average_ > end_frame_ - start_frame_) {
            end_frame_ = start_frame_ + static_cast<unsigned int>(pow(frame_interval_, number_of_time_points_) + 0.5)  + number_of_frames_to_average_;
            cerr << "WARNING: the number of frames required is greater then the number supplied\n";
            cerr << "         setting end frame to minimum value allowed: ";
            cerr << end_frame_;
            cerr << endl;
        }
        if (frame_interval_ <= 1) {
            cerr << "ERROR: frame_interval must be greater than 1.0 for logscale\n" << endl;
            exit(1);
        }
    }
    else {
        cerr << "ERROR: Illegal time scale specified. Must be one of (linear/log)\n" << endl;
        exit(1);
    }
    
    if (is_wrapped_) {
        cerr << "WARNING: the trajectory provided is not unwrapped\n";
        cerr << "       : We will unwrapp it for you, but user discretion\n";
        cerr << "       : is advised";
        cerr << endl;
    }
}


void FourPointCorrelation::compute_time_array()
{
    time_array_indexes_.resize(number_of_time_points_);
    time_array_indexes_[0] = 0;
    
    double       total_time     = frame_interval_;
    unsigned int frame_previous = 0;
    
    for (size_t time_point = 1; time_point < number_of_time_points_; ++time_point) {
        if (time_scale_type_ == "linear") {
            time_array_indexes_[time_point] = static_cast<unsigned int>(total_time);
            total_time += frame_interval_;
        }
        else {
            time_array_indexes_[time_point] = time_array_indexes_[time_point - 1];
            while (time_array_indexes_[time_point] == frame_previous) {
                time_array_indexes_[time_point] = static_cast<unsigned int>(total_time + 0.5);
                total_time *= frame_interval_;
            }
            frame_previous = time_array_indexes_[time_point];
        }
        
        assert(time_array_indexes_[time_point] + number_of_frames_to_average_ < end_frame_ - start_frame_ && "Error: Not eneough frames for calculation on log time scale");
    }
}


