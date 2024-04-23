#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <time.h>

using namespace std;




int main()
{
    srand(time(0));
    string filename = "randNumber.txt";
    ofstream fout(filename.c_str());
    for(int i = 0; i < 10000; i++)
    {
        fout << (rand()%30 + 1);
        fout << ",";
    }
    fout.close();
    return 0;
}
