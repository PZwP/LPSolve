#include "lp_lib.h"
#include<iostream>
#include<fstream>
#include<sstream>

lprec* create_model(std::string a, std::string b, std::string c, std::string x)
{
    // get matrices B and C as they will determine the size of a model
    REAL temp;
    int n, m;
    REAL* row;

    std::ifstream file_a, file_b, file_c, file_x;
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
    int size_a = 0;
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
    lp = make_lp(0, n);
    if(lp == NULL || row == NULL)
    {
        std::cerr << "Unable to create new model\n";
        return NULL;
    }

    //name variables (names may not contain spaces)
    file_x.open(x);
    for(int i = 0; i < n && !file_x.eof(); i++)
    {
        std::string tmp, name, type;
        REAL low, up;
        getline(file_x, tmp);
        std::stringstream ss(tmp);
        ss >> name;
        if(ss >> type)
        {
            if(type == "int")
                set_int(lp, i + 1, TRUE);
            if(type == "bin" || type == "binary")
                set_binary(lp, i + 1, TRUE);
            if(type == "sec")
                set_semicont(lp, i + 1, TRUE);
            if(type == "free")
                set_unbounded(lp, i + 1);
        }
        if(ss >> low)
        {
            set_lowbo(lp, i + 1, low);
        }
        if(ss >> up)
        {
            set_upbo(lp, i + 1, up);
        }
        set_col_name(lp, i + 1, const_cast<char*>(name.c_str()));
    }

    set_add_rowmode(lp, TRUE); //optimization for constructing the model row by row

    // set the objective function
    for(int i = 1; i <= n; i++)
        file_c >> row[i];
    if(!set_obj_fn(lp, row))
    {
        std::cerr << "Unable to set objective function\n";
        return NULL;
    }

    // create the constraints
    for(int i = 0; i < m; i++)
    {
        for(int j = 1; j <= n; j++)
            file_a >> row[j];
        file_b >> temp;
        if(!add_constraint(lp, row, LE, temp))
        {
            std::cerr << "Unable to add constraint\n";
            return NULL;
        }
    }

    set_maxim(lp);
    set_add_rowmode(lp, FALSE);

    // close files
    file_a.close();
    file_b.close();
    file_c.close();
    file_x.close();

    // free allocated memory
    delete[] row;

    return lp;
}

int main(int argc, char* argv[])
{
    bool model_ready = false;

    // allow using nondefault filenames
    std::string a, b, c, x, r;
    a = "input/A.txt";
    b = "input/B.txt";
    c = "input/C.txt";
    x = "input/X.txt";
    r = "result.txt";
    if(argc >= 2)
    {
        a = argv[1];
        if(a[a.length() - 2] == 'l' && a[a.length() - 1] == 'p')
            model_ready = true;
        if(argc >= 3)
        {
            if(model_ready)
                r = argv[2];
            else
            {
                b = argv[2];
                if(argc >= 4)
                {
                    c = argv[3];
                    if(argc >= 5)
                    {
                        x = argv[4];
                        if(argc >= 6)
                            r = argv[5];
                    }
                }
            }
        }
    }
    
    // create model
    lprec* model;
    if(model_ready)
        model = read_LP(const_cast<char*>(a.c_str()), NORMAL, NULL);
    else
        model = create_model(a, b, c, x);

    if(model == NULL)
    {
        std::cerr << "Model not created. Exiting program.\n";
        return 1;
    }

    // print model to screen
    set_verbose(model, IMPORTANT);
    write_lp(model, NULL);
    std::cout << "\n\n\n";

    // solve model
    int ret = solve(model);
    switch(ret)
    {
        case -2:
        {
            std::cout << "Out of memory\n";
            break;
        }
        case 0:
        {
            std::cout << "An optimal solution was obtained\n";
            break;
        }
        case 1:
        {
            std::cout << "An integer solution was found. The solution is not guaranteed the most optimal one\n";
            break;
        }
        case 2:
        {
            std::cout << "The model is infeasible\n";
            break;
        }
        case 3:
        {
            std::cout << "The model is unbounded\n";
            break;
        }
        case 4:
        {
            std::cout << "The model is degenerative\n";
            break;
        }
        case 5:
        {
            std::cout << "Numerical failure encountered\n";
            break;
        }
        case 6:
        {
            std::cout << "The abort routine returned TRUE\n";
            break;
        }
        case 7:
        {
            std::cout << "A timeout occurred\n";
            break;
        }
        case 9:
        {
            std::cout << "The model could be solved by presolve\n";
            break;
        }
        case 25:
        {
            std::cout << "Accuracy error encountered\n";
            break;
        }
        default:
            std::cerr << "Error: solve() returned an unknown value\n";
    }

    // print results
    std::cout << "Objective value: " << get_objective(model) << '\n';

    REAL* row;
    row = new REAL[get_Ncolumns(model)];
    get_variables(model, row);

    // open file for result writing
    std::fstream result;
    result.open(r, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if(!result.is_open())
        return 2;
    
    // write results to screen and file
    for(int i = 0; i < get_Ncolumns(model); i++)
    {
        std::cout << get_col_name(model, i + 1) << ": " << row[i] << '\n';
        result << row[i] << '\n';
    }

    // free allocated memory
    delete[] row;
    delete_lp(model);
    
    return 0;
}