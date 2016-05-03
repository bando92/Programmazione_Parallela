// ParallelMergeSortOMP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <omp.h>
#include <vector>
#include <iostream>
#include <time.h>
#define ELEMENTS 10000000
#define THREADNUM 2
using namespace std;

//funzione di merge
vector<long> merge(const vector<long>& left, const vector<long>& right)
{
    vector<long> result;
    unsigned left_it = 0, right_it = 0;

    while(left_it < left.size() && right_it < right.size())
    {
        if(left[left_it] < right[right_it])
        {
            result.push_back(left[left_it]);
            left_it++;
        }
        else					
        {
            result.push_back(right[right_it]);
            right_it++;
        }
    }

    while(left_it < left.size())
    {
        result.push_back(left[left_it]);
        left_it++;
    }

    while(right_it < right.size())
    {
        result.push_back(right[right_it]);
        right_it++;
    }

    return result;
}

//mergesort
vector<long> mergesort(vector<long>& vec, int threads)
{

	//è ricorsivo...quando arrivo ad avere un solo elemento mi fermo
    if(vec.size() == 1)
    {
        return vec;
    }

    // determino l'elemento di mezzo del vettore
    std::vector<long>::iterator middle = vec.begin() + (vec.size() / 2);

	//creo 2 vettori sinistro e destro
    vector<long> left(vec.begin(), middle);
    vector<long> right(middle, vec.end());

	//se ho più di un thread faccio un merge sort parallelo,altrimenti ne faccio uno lineare
    if (threads > 1)
    {
      #pragma omp parallel sections//attribuisco ad ogni thread una metà del vettore per ordinarlo
      {
        #pragma omp section
        {
          left = mergesort(left, threads/2);
        }
        #pragma omp section
        {
          right = mergesort(right, threads - threads/2);
        }
      }
    }
    else
    {
      left = mergesort(left, 1);
      right = mergesort(right, 1);
    }

    return merge(left, right);
}

int main()
{
	omp_set_num_threads(THREADNUM); //setto il numero dei thread a THREADNUM
	double start_time, total_time1, total_time2; //inizializzo le variabili per calcolare il tempo di esecuzione

	vector<long> v(ELEMENTS),v2(ELEMENTS);//creo 2 vettori di elementi
	srand((unsigned)time(NULL));
	for (long i=0; i<ELEMENTS; ++i)//riempimento vettori
		//v[i] = (i * i) % ELEMENTS;
		v[i] = v2[i] = rand() % 999;

	start_time = omp_get_wtime();//mergesort con 2 thread
	v = mergesort(v, THREADNUM);
	total_time1 = omp_get_wtime() - start_time;

	cout << "Tempo impiegato dal parallel merge sort con 2 thread: " << total_time1 << endl;

	start_time = omp_get_wtime();//mergesort con 1 thread
	v2 = mergesort(v2, 1);
	total_time2 = omp_get_wtime() - start_time;

	cout << "Tempo impiegato dal merge sort: " << total_time2 << endl;

	system("pause");
	return 0;
}

