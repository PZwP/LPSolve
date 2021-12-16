# LPSolver

## Instructions
1. Download `lp_solve_5.5.2.11_dev_win32.zip` from https://sourceforge.net/projects/lpsolve/files/lpsolve/5.5.2.11/
2. Unpack contents of that file into folder with files from this repository
3. Change your compiler to 32-bit
4. Add `lpsolve55.dll` to linker's additional dependencies/options
5. If necessary, add `#define WIN32` as a first line in `main.cpp`

## Usage
Program solves Mixed Integer Linear Programming problems of the form  
  ```
  max C^T X  
  Ax<=B  
  X>=0
  ```  
where matrices A,B,C are stored in files `A.txt`, `B.txt`, `C.txt`.  
File `X.txt` stores names and types of variables.  
File `result.txt` is used to store a solution to the problem  
File `config.txt` allows adding a timeout  
Program exits after an optimal solution is found, a timeout occured, or the user pressed any button. If no optimal solution was found until that happened, file `result.txt` will contain best solution found before the program terminated.
