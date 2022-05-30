#include <iostream>

#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <mpi.h>

using namespace std;

#pragma warning(disable 4703)

vector<int> prefix_function(const string& s) {
    vector<int> pi(s.length(), 0);

    for (int i = 1; i < s.length(); i++) {
        int j = pi[i - 1];

        while (j > 0 && s[i] != s[j]) {
            j = pi[j - 1];
        }

        if (s[i] == s[j]) {
            pi[i] = j + 1;
        }
        else {
            pi[i] = j;
        }
    }

    return pi;
}

void getInput(int& sizeOfProcessesGroup, int& myRank, std::string& str, long& stringSize, const long& lineCounter, long& partSize) {
    if (myRank == 0) {
        std::cout << "Enter t:\n";
        std::cin >> str;
        stringSize = str.size();
        partSize = lineCounter / sizeOfProcessesGroup;
    }
}

int main(int argc, char* argv[]) {
    string t; // строка которую мы ищем
    long stringSize = 0; // ее длинна
    long partSize = 0; // длинна части начальной строки которую будет обрабатывать отдельный процесс
    int rank, size; // ранг процесса и размер группы процессов
    double start{}, finish{}, loc_elapsed{}, elapsed{}; // время
    ifstream f("C:\\Users\\denis\\CLionProjects\\kmp_mpi\\input1.txt");
    vector<string> lines;
    while (getline(f, t))
    {
        lines.push_back(move(t));
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    getInput(size, rank, t, stringSize, lines.size(), partSize); // вводим строку только в 0 процессе, чтобы не вводить ее везде
    MPI_Bcast(&stringSize, 1, MPI_LONG, 0, MPI_COMM_WORLD); // передаем размер строки всем процессам в группе
    if (rank != 0) {
        t.resize(stringSize); // все процессы кроме 0 должны подготовиться к получению строки, т.к, она была введена только в нем
    }

    MPI_Bcast(const_cast<char*>(t.data()), stringSize, MPI_CHAR, 0, MPI_COMM_WORLD); // передаем строку всем процессам в группе
    MPI_Bcast(&partSize, 1, MPI_LONG, 0, MPI_COMM_WORLD); // передаем размер части строки, которую нужно обработать каждому
    int startPart = rank * partSize;
    int endPart = startPart + partSize;
    if (rank == size - 1) {
        endPart = lines.size();
    }

    int globalCounter = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    std::string s;
    for (int i = startPart; i < endPart; ++i) {
        s += lines[i];
    }
    start = MPI_Wtime();
    vector<int> pi = prefix_function(t + '#' + s);
    int localCounter = 0;
    int t_len = t.length();
    for (int i = 0; i < s.size(); i++) {
        if (pi[t_len + 1 + i] == t_len) {
            localCounter++;
        }
    }

    finish = MPI_Wtime();
    loc_elapsed = finish - start;
    MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        MPI_Reduce(&localCounter, &globalCounter, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        cout << "Matches: " << globalCounter << '\n';
        printf("Elapsed time = %f seconds \n", elapsed);
    }
    MPI_Finalize();
}