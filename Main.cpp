#include "lp_lib.h"
#include<iostream>
#include<fstream>

int demo()
{
    lprec* lp;
    int Ncol, * colno = NULL, j, ret = 0;
    REAL* row = NULL;

    /* We will build the model row by row
       So we start with creating a model with 0 rows and 2 columns */
    Ncol = 2; /* there are two variables in the model */
    lp = make_lp(0, Ncol);
    if(lp == NULL)
        ret = 1; /* couldn't construct a new model... */

    if(ret == 0)
    {
        /* let us name our variables. Not required, but can be useful for debugging */
        set_col_name(lp, 1, "x");
        set_col_name(lp, 2, "y");

        /* create space large enough for one row */
        colno = (int*) malloc(Ncol * sizeof(*colno));
        row = (REAL*) malloc(Ncol * sizeof(*row));
        if((colno == NULL) || (row == NULL))
            ret = 2;
    }

    if(ret == 0)
    {
        set_add_rowmode(lp, TRUE);  /* makes building the model faster if it is done rows by row */

        /* construct first row (120 x + 210 y <= 15000) */
        j = 0;

        colno[j] = 1; /* first column */
        row[j++] = 120;

        colno[j] = 2; /* second column */
        row[j++] = 210;

        /* add the row to lpsolve */
        if(!add_constraintex(lp, j, row, colno, LE, 15000))
            ret = 3;
    }

    if(ret == 0)
    {
        /* construct second row (110 x + 30 y <= 4000) */
        j = 0;

        colno[j] = 1; /* first column */
        row[j++] = 110;

        colno[j] = 2; /* second column */
        row[j++] = 30;

        /* add the row to lpsolve */
        if(!add_constraintex(lp, j, row, colno, LE, 4000))
            ret = 3;
    }

    if(ret == 0)
    {
        /* construct third row (x + y <= 75) */
        j = 0;

        colno[j] = 1; /* first column */
        row[j++] = 1;

        colno[j] = 2; /* second column */
        row[j++] = 1;

        /* add the row to lpsolve */
        if(!add_constraintex(lp, j, row, colno, LE, 75))
            ret = 3;
    }

    if(ret == 0)
    {
        set_add_rowmode(lp, FALSE); /* rowmode should be turned off again when done building the model */

        /* set the objective function (143 x + 60 y) */
        j = 0;

        colno[j] = 1; /* first column */
        row[j++] = 143;

        colno[j] = 2; /* second column */
        row[j++] = 60;

        /* set the objective in lpsolve */
        if(!set_obj_fnex(lp, j, row, colno))
            ret = 4;
    }

    if(ret == 0)
    {
        /* set the object direction to maximize */
        set_maxim(lp);

        /* just out of curioucity, now show the model in lp format on screen */
        /* this only works if this is a console application. If not, use write_lp and a filename */
        write_LP(lp, stdout);
        //write_lp(lp, "model.lp"); 

        /* I only want to see important messages on screen while solving */
        set_verbose(lp, IMPORTANT);

        /* Now let lpsolve calculate a solution */
        ret = solve(lp);
        if(ret == OPTIMAL)
            ret = 0;
        else
            ret = 5;
    }

    if(ret == 0)
    {
        /* a solution is calculated, now lets get some results */

        /* objective value */
        printf("Objective value: %f\n", get_objective(lp));

        /* variable values */
        get_variables(lp, row);
        for(j = 0; j < Ncol; j++)
            printf("%s: %f\n", get_col_name(lp, j + 1), row[j]);

        /* we are done now */
    }

    /* free allocated memory */
    if(row != NULL)
        free(row);
    if(colno != NULL)
        free(colno);

    if(lp != NULL)
    {
        /* clean up such that all used memory by lpsolve is freed */
        delete_lp(lp);
    }

    return(ret);
}

int main(int argc, char* argv[])
{
    std::string a, b, c, x;
    a = "input/A.txt";
    b = "input/B.txt";
    c = "input/C.txt";
    x = "input/X.txt";
    if(argc >= 2)
    {
        a = argv[1];
        if(argc >= 3)
        {
            b = argv[2];
            if(argc >= 4)
            {
                c = argv[3];
                if(argc >= 5)
                    x = argv[4];
            }
        }
    }
    std::cout << "A: " << a << std::endl;
    std::cout << "B: " << b << std::endl;
    std::cout << "C: " << c << std::endl;
    std::cout << "X: " << x << std::endl;
    std::ifstream file_a, file_b, file_c, file_x;
    file_a.open(a);
    file_b.open(b);
    file_c.open(c);
    file_x.open(x);

    std::fstream result;
    std::string s("result.txt");
    result.open(s.c_str(), std::fstream::in | std::fstream::out | std::fstream::trunc);
    if(!result.is_open())
        return 1;
    result << "test";

    /*

    lprec* lp;

    // Read LP model from file "input.lp"
    lp = read_LP("input.lp", NORMAL, NULL);
    if(lp == NULL)   // File could not be opened or file has wrong structure or not enough memory available to setup an lprec structure
    {
        lp = make_lp(0, 0);
        if(lp == NULL)
        {
            std::cerr << "Unable to create new LP model\n";
            return 1;
        }
    }
    // Model created

    demo();
    int i;
    std::cin >> i;

    delete_lp(lp);
    return 0;*/
}