#include "stdafx.h"
#include <mpi.h>
#include <iostream>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>					
#include <time.h>					
#include <math.h>		
#include <omp.h>

using namespace std;

int main(int argc, char* argv[])
{
	double starttime;
	int my_id, numprocs,length;
	int out=0;
	int in=0,MAXEVENTS,thread_grant; 
	omp_set_num_threads(omp_get_num_procs());//setta il numero dei thread al numero dei processi
	//THREAD_MULTIPLE -> Multiple threads may call MPI, with no restrictions.
	MPI_Init_thread(&argc, &argv,MPI_THREAD_MULTIPLE,&thread_grant) ;   // Chiamata obbligatoria di inizializzazione con richiesta di supporto ai thread	
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id) ;     // Ottiene l'identificativo del processo e lo mette nella variabile my_id
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs) ;  // Ottiene quanti processi sono attivi e mette il numero in numprocs
	MAXEVENTS=10000000;   
	MPI_Bcast(&MAXEVENTS,1,MPI_INT,0,MPI_COMM_WORLD); //mando in broadcast a tutti i processi un buffer(in questo caso è MAXEVENTS),quanti dati contiene il buffer(1)
													//di che tipo sono gli elementi del buffer(MPI_INT)
													//l'identificativo del processo root(0)
													//su quale canale devo fare l'invio(MPI_COMM_WORLD)

	if (my_id==0)  // Master Code
	{
		starttime=MPI_Wtime();//faccio partire il timer
		cout << "Started..." << endl;
		float ran[2];
		srand(time(NULL));
		int req;
		MPI_Status status;
		for (int i=1;i<MAXEVENTS;i++) {//continuo a inviare dati fino a MAXEVENTS

			ran[1]=(float)rand()/(float)RAND_MAX;
			ran[0]=(float)rand()/(float)RAND_MAX;

			MPI_Recv(&req,1,MPI_INT,MPI_ANY_SOURCE,10,MPI_COMM_WORLD,&status);//ricevo da ogni processo che non sia 0 il suo stesso numero per farsi mandare altri numeri da processare
			MPI_Send(&ran[0],2,MPI_FLOAT,status.MPI_SOURCE,req,MPI_COMM_WORLD);//invio a tutti i processi dei nuovi valori su cui fare dei calcoli

		}
		ran[1]=99;//li setto così per far fermare gli altri processi
		ran[0]=99;
		for (int i=1;i<numprocs;i++)
		{
			MPI_Recv(&req,1,MPI_INT,MPI_ANY_SOURCE,10,MPI_COMM_WORLD,&status);
			MPI_Send(&ran[0],2,MPI_FLOAT,status.MPI_SOURCE,req,MPI_COMM_WORLD);
		}
	} 
	else // SLAVE Code
	{ 
		int work_end=1;

		#pragma omp parallel reduction(+:in,out)// le variabili private dentro ogni thread (in,out) verranno alla fine sommate(+) tra tutti i thread che hanno partecipato
		{
			float ran[2];
			MPI_Status status;
			int TID=omp_get_thread_num(),NTH=omp_get_num_threads();//numero del thread e quanti thread ci sono
		
			do{
				#pragma omp critical//fatto da un solo thread
				{
					if (work_end)//se work end == 1
					{
						MPI_Send(&TID,1,MPI_INT,0,10,MPI_COMM_WORLD);//invio al processo 0 il mio tid e aspetto i dati
						MPI_Recv(&ran[0],2,MPI_FLOAT,0,TID,MPI_COMM_WORLD,&status);//ricevo i dati da processare dal processo 0
	
					} // if (work_end)

					if (ran[0]>10||ran[1]>10)  //i processi si fermano quando ricevono dal thread 0 un numero > 10
						work_end=0; //setto la condizione di uscita a false
	
				} // end critical

				if((ran[0]*ran[0]+ran[1]*ran[1])<1) in++;
				out++;
			} while(work_end);


		#pragma omp critical
		cout << "Ended ! Thread:"<< TID << " Process:"<< my_id<< " Processed Data: " <<out << endl;//per ogni thread di ogni processo stampo quanti dati ho processato

		}// end multithread
	}// if my_id==0
	MPI_Barrier(MPI_COMM_WORLD);//finchè tutti i processi non sono arrivati alla barriera l'esecuzione dei processi non va avanti
	cout << " Process:"<< my_id<< " Total Processed Data: " <<out << endl;
	int pi;
	//ogni processo deve ridurre la sua variabile 'in' in un'unica variabile 'pi' utilizzando la funzione MPI_SUM => sommo tutti i valori di 'in' dei vari processi in 'pi'
	MPI_Reduce(&in,&pi,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	if (my_id==0)//se sono il processo root
	cout << "And here is PI: "<< (float)pi*4/(float)MAXEVENTS << "  In " << MPI_Wtime()-starttime<< " seconds." <<endl;
	MPI_Finalize();

	return 0;
}

