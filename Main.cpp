#include "lp_lib.h"
#include<iostream>
#include<fstream>

struct model
{
    int n, m;
    REAL* B;
    REAL* C;

    lprec* lp;
};

void create_model(model& M, std::string a, std::string b, std::string c, std::string x)
{
    // get matrices B and C as they will determine the size of a model
    REAL temp;

    std::ifstream file_b, file_c, file_x;
    file_b.open(b);
    file_c.open(c);

    /* get number of variables determined by a matrix C
       and number of constraints determined by a matrix B */
    M.n = 0;
    M.m = 0;
    while(file_c >> temp)
        M.n++;
    while(file_b >> temp)
        M.m++;

    // rewind files c and b
    file_c.clear();
    file_c.seekg(0, std::ios::beg);
    file_b.clear();
    file_b.seekg(0, std::ios::beg);

    // get matrices B and C

    // close files
    file_b.close();
    file_c.close();
}

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
        if(argc >= 3)
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
    
    // create model
    model M;
    create_model(M, a, b, c, x);


    std::fstream result;
    result.open(r, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if(!result.is_open())
        return 1;
    result << "test";

    
    return 0;
}