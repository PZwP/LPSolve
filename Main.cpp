#include "lpsolve/lp_lib.h"
#include<iostream>
#include<fstream>
#include<sstream>
#include<conio.h>

bool check_user_input() noexcept
{
	if(_kbhit())
		return true;
	return false;
}

extern "C" int __WINAPI abort_on_user_input(lprec*, void*)
{
	if(check_user_input())
		return TRUE;
	return FALSE;
}

void replace_all(const std::string& filename, char what, char with)
{
	std::fstream f;
	f.open(filename);
	std::stringstream ss;
	ss << f.rdbuf();
	std::string s = ss.str();
	for(unsigned int i = 0; i < s.length(); i++)
		if(s[i] == what)
			s[i] = with;
	f.clear();
	f.seekg(0, std::ios::beg);
	f << s;
	f.close();
}

void set_variables(lprec* lp, const std::string& x, unsigned int n)
{
	std::ifstream file_x;
	file_x.open(x);
	for(unsigned int i = 0; i < n && !file_x.eof(); i++)
	{
		std::string tmp;
		std::string name;
		std::string type;
		REAL low;
		REAL up;
		getline(file_x, tmp);
		std::stringstream ss(tmp);
		ss >> name;
		if(ss >> type)
		{
			if(type == "int")
				set_int(lp, static_cast<int>(i) + 1, TRUE);
			if(type == "bin" || type == "binary")
				set_binary(lp, static_cast<int>(i) + 1, TRUE);
			if(type == "sec")
				set_semicont(lp, static_cast<int>(i) + 1, TRUE);
			if(type == "free")
				set_unbounded(lp, static_cast<int>(i) + 1);
		}
		if(ss >> low)
		{
			set_lowbo(lp, static_cast<int>(i) + 1, low);
		}
		if(ss >> up)
		{
			set_upbo(lp, static_cast<int>(i) + 1, up);
		}
		set_col_name(lp, static_cast<int>(i) + 1, const_cast<char*>(name.c_str()));
	}
	file_x.close();
}

lprec* create_model(const std::string& a, const std::string& b, const std::string& c, const std::string& x)
{
	// sanitize input files: change all commas to points in case the files are copied from spreadsheets
	replace_all(a, ',', '.');
	replace_all(b, ',', '.');
	replace_all(c, ',', '.');
	replace_all(x, ',', '.');

	// get matrices B and C as they will determine the size of a model
	REAL temp;
	unsigned int n;
	unsigned int m;
	REAL* row;

	std::ifstream file_a;
	std::ifstream file_b;
	std::ifstream file_c;
	file_b.open(b);
	file_c.open(c);

	/* get number of variables determined by a matrix C
	   and number of constraints determined by a matrix B */
	n = 0;
	m = 0;
	while(file_c >> temp)
		n++;
	while(file_b >> temp)
		m++;

	//check if matrix A is of correct dimension
	file_a.open(a);
	unsigned int size_a = 0;
	while(file_a >> temp)
		size_a++;
	if(size_a != n * m)
		std::cerr << "Warning: size of matrix A (" << size_a << ") is not equal to expected (" << n * m << ")\n";

	// rewind files a, b, and cc
	file_c.clear();
	file_c.seekg(0, std::ios::beg);
	file_b.clear();
	file_b.seekg(0, std::ios::beg);
	file_a.clear();
	file_a.seekg(0, std::ios::beg);

	row = new REAL[n + 1];  // values start at element 1

	//create and initialase a new model
	lprec* lp;
	lp = make_lp(0, static_cast<int>(n));
	if(lp == nullptr || row == nullptr)
	{
		std::cerr << "Unable to create new model\n";
		if(row)
			delete[] row;
		return nullptr;
	}

	//name variables (names may not contain spaces)
	set_variables(lp, x, n);

	set_add_rowmode(lp, TRUE); //optimization for constructing the model row by row

	// set the objective function
	for(unsigned int i = 1; i <= n; i++)
		file_c >> row[i];
	if(!set_obj_fn(lp, row))
	{
		std::cerr << "Unable to set objective function\n";
		return nullptr;
	}

	// create the constraints
	for(unsigned int i = 0; i < m; i++)
	{
		for(unsigned int j = 1; j <= n; j++)
			file_a >> row[j];
		file_b >> temp;
		if(!add_constraint(lp, row, LE, temp))
		{
			std::cerr << "Unable to add constraint\n";
			return nullptr;
		}
	}

	set_maxim(lp);
	set_add_rowmode(lp, FALSE);

	// close files
	file_a.close();
	file_b.close();
	file_c.close();

	// free allocated memory
	delete[] row;

	return lp;
}

int my_solve(lprec* model, const std::string& r)
{
	// solve model
	const int ret = solve(model);
	std::cout << '\n';
	switch(ret)
	{
		case -2:
			std::cout << "Out of memory\n";
			break;
		case 0:
			std::cout << "An optimal solution was obtained\n";
			break;
		case 1:
			std::cout << "An integer solution was found. The solution is not guaranteed the most optimal one\n";
			break;
		case 2:
			std::cout << "The model is infeasible\n";
			break;
		case 3:
			std::cout << "The model is unbounded\n";
			break;
		case 4:
			std::cout << "The model is degenerative\n";
			break;
		case 5:
			std::cout << "Numerical failure encountered\n";
			break;
		case 6:
			std::cout << "The abort routine returned TRUE\n";
			break;
		case 7:
			std::cout << "A timeout occurred\n";
			break;
		case 9:
			std::cout << "The model could be solved by presolve\n";
			break;
		case 25:
			std::cout << "Accuracy error encountered\n";
			break;
		default:
			std::cerr << "Error: solve() returned an unknown value\n";
	}

	if(ret == 0 || ret == 1 || ret == 6)
	{
		// print results
		std::cout << "Objective value: " << get_objective(model) << '\n';

		REAL* row;
		const auto number_of_columns = static_cast<unsigned int>(get_Ncolumns(model));
		row = new REAL[number_of_columns];
		get_variables(model, row);

		// open file for result writing
		std::fstream result;
		result.open(r, std::fstream::in | std::fstream::out | std::fstream::trunc);
		if(!result.is_open())
			return 2;

		// write results to screen and file
		for(unsigned int i = 0; i < number_of_columns; i++)
		{
			if(row[i] != 0)
				std::cout << get_col_name(model, static_cast<int>(i) + 1) << ": " << row[i] << '\n';
			result << row[i] << '\n';
		}
		std::cout << "\n\n";

		// free allocated memory
		delete[] row;
	}
	return ret;
}

void my_solve_value(lprec* model, const std::string& r)
{
	set_break_at_first(model, TRUE);
	int ret = my_solve(model, r);
	set_break_at_first(model, FALSE);
	REAL interval = 0.001;
	REAL last_objective = get_objective(model);
	while(ret == 1)
	{
		set_break_at_value(model, last_objective + interval);
		ret = my_solve(model, r);
		if(ret == 1)
		{
			last_objective = get_objective(model);
		}
		else if(ret > 1)
		{
			if(interval < 1000)
			{
				interval *= 10;
				std::cerr << "Interval increased to " << interval << '\n';
				ret = 1;
			}
			else
			{
				set_break_at_value(model, INFINITY);
				ret = my_solve(model, r);
			}
		}
	}
}

void my_solve_time(lprec* model, const std::string& r, long timeout)
{
	set_timeout(model, timeout);
	my_solve(model, r);
	set_timeout(model, 0);
}

int main(int argc, const char* argv[])
{
	bool model_ready = false;

	// allow using nondefault filenames
	std::string a = "input/A.txt";
	std::string b = "input/B.txt";
	std::string c = "input/C.txt";
	std::string x = "input/X.txt";
	std::string r = "result.txt";
	if(argc > 6)
		argc = 6;
	if(argc >= 2)
	{
		a = argv[1];
		if(a[a.length() - 2] == 'l' && a[a.length() - 1] == 'p')
		{
			model_ready = true;
			if(argc >= 3)
				r = argv[2];
		}
		else
		{
			switch(argc)
			{
				case 6:
					r = argv[5];
					[[fallthrough]];
				case 5:
					x = argv[4];
					[[fallthrough]];
				case 4:
					c = argv[3];
					[[fallthrough]];
				case 3:
					b = argv[2];
					break;
				default:
					break;
			}
		}
	}

	// create model
	lprec* model;
	if(model_ready)
		model = read_LP(const_cast<char*>(a.c_str()), NORMAL, nullptr);
	else
		model = create_model(a, b, c, x);

	if(model == nullptr)
	{
		std::cerr << "Model not created. Exiting program.\n";
		return 1;
	}

	// print model to screen
	set_verbose(model, NORMAL);
	put_abortfunc(model, abort_on_user_input, nullptr);   //allow user to gracefully abort the program
	write_lp(model, nullptr);   // write model to console
	//write_lp(model, "model.lp");  //write model to file
	std::cout << "\n\n\n";

	//read config
	long timeout = 0;//timeout in seconds, 0 if no timeout, -1 if save at each solution found
	std::ifstream config;
	config.open("config.txt");
	if(config.is_open())
	{
		config >> timeout;
		config.close();
	}

	// solve model
	if(timeout == 0)
		my_solve(model, r);
	else if(timeout < 0)
	{
		set_verbose(model, IMPORTANT);
		my_solve_value(model, r);
	}
	else
		my_solve_time(model, r, timeout);

	// free allocated memory
	delete_lp(model);

	// sanitize output for easier pasting into a spreadsheet
	replace_all(r, '.', ',');

	return 0;
}