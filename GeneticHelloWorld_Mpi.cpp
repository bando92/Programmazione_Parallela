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

#pragma warning(disable:4786)		// disable debug warning


#define GA_POPSIZE		10000		// ga population size
#define GA_MAXITER		500		// maximum iterations
#define GA_ELITRATE		0.10f		// elitism rate
#define GA_MUTATIONRATE	0.25f		// mutation rate
#define GA_MUTATION		RAND_MAX * GA_MUTATIONRATE
#define GA_TARGET		std::string("Bandini Lorenzo 0000626451")

using namespace std;

struct ga_struct //elemento della popolazione...è un singolo individuo
{
	string str;						// stringa
	unsigned int fitness;			// il suo fitness
};

typedef vector<ga_struct> ga_vector; // vettore di individui


void init_population(ga_vector*, ga_vector* );
void calc_fitness(ga_vector*);
int calc_fitness_single(string);
bool fitness_sort(ga_struct, ga_struct);
inline void sort_by_fitness(ga_vector*);
void elitism(ga_vector*, ga_vector*, int);
void mutate(ga_struct*);
void mate(ga_vector*, ga_vector*);
inline void print_best(ga_vector*);
string getString(ga_vector*);
inline void swapp(ga_vector**, ga_vector**);


//inizializzo la popolazione con una stringa casuale
void init_population(ga_vector &population, ga_vector &buffer ) 
{
	int tsize = GA_TARGET.size();

	for (int i=0; i<GA_POPSIZE; i++) {//crea GA_POPSIZE individui per popolare la "città"
		ga_struct citizen;
		
		citizen.fitness = 0;
		citizen.str.erase();

		for (int j=0; j<tsize; j++)
			citizen.str += (rand() % 90) + 32;

		population.push_back(citizen);
	}

	buffer.resize(GA_POPSIZE);
}

// funzione che calcola il fitness (quanto si avvicina alla soluzione) di ogni elemento del vettore population
void calc_fitness(ga_vector &population)
{
	string target = GA_TARGET; //frase obiettivo
	int tsize = target.size(); //dimensione frase obiettivo
	unsigned int fitness;

	for (int i=0; i<GA_POPSIZE; i++) { //per ogni elemento della popolazione
		fitness = 0;
		fitness = calc_fitness_single(population[i].str);
		//for (int j=0; j<tsize; j++) {
		//	fitness += abs(int(population[i].str[j] - target[j])); //calcola il fitness della stringa come somma delle distanze delle lettere della stringa dalle lettere corrispondenti della frase obiettivo
		//}
		population[i].fitness = fitness;
	}
}

// funzione che calcola il fitness (quanto si avvicina alla soluzione) di una singola stringa
int calc_fitness_single(string str)
{
	string target = GA_TARGET; //frase obiettivo
	int tsize = target.size(); //dimensione frase obiettivo
	unsigned int fitness;

	fitness = 0;

	for (int j=0; j<tsize; j++) {
		fitness += abs(int(str[j] - target[j])); //calcola il fitness della stringa come somma delle distanze delle lettere della stringa dalle lettere corrispondenti della frase obiettivo
	}
	return fitness;
}

//funzione per l'ordinamento
bool fitness_sort(ga_struct x, ga_struct y)
{ return (x.fitness < y.fitness); }

//ordina la popolazione dall'inizio alla fine secondo il fitness crescente
inline void sort_by_fitness(ga_vector &population)
{ sort(population.begin(), population.end(), fitness_sort); }

//stampa il primo elemento del vettore
inline void print_best(ga_vector &gav)
{ cout << "Frase: " << gav[0].str << " Fitness: " << gav[0].fitness << endl; }

//restituisce la frase del primo elemento del vettore
string getString(ga_vector &gav)
{ return gav[0].str; }

//scambia population con buffer
inline void swapp(ga_vector *&population, ga_vector *&buffer)
{ ga_vector *temp = population; population = buffer; buffer = temp; }

//copio nel buffer solo i primi esize individui(che sono i migliori)
void elitism(ga_vector &population, ga_vector &buffer, int esize )
{
	for (int i=0; i<esize; i++) {
		buffer[i].str = population[i].str;
		buffer[i].fitness = population[i].fitness;
	}
}

//muta un individuo della popolazione
void mutate(ga_struct &member)
{
	int tsize = GA_TARGET.size();
	int ipos = rand() % tsize;
	int delta = (rand() % 90) + 32; 

	member.str[ipos] = ((member.str[ipos] + delta) % 122);
}

void mate(ga_vector &population, ga_vector &buffer)
{
	int esize = GA_POPSIZE * GA_ELITRATE; // prendo una percentuale della popolazione
	int tsize = GA_TARGET.size(), spos, i1, i2;

	elitism(population, buffer, esize); // ora nel buffer ho i migliori esize

	// Mate the rest
#pragma omp parallel
	{
		#pragma omp for
		for (int i=esize; i<GA_POPSIZE; i++)
		{ //fonde 2 sottostringhe di lunghezza casuale prese da 2 elementi del vettore scelti a caso
			i1 = rand() % (GA_POPSIZE / 2);
			i2 = rand() % (GA_POPSIZE / 2);
			spos = rand() % tsize;
			buffer[i].str = population[i1].str.substr(0, spos) + 
							population[i2].str.substr(spos, esize - spos);

			if (rand() < GA_MUTATION) mutate(buffer[i]);
		}
	};
}

int main(int argc, char* argv[])
{
	
	ga_vector pop_alpha, pop_beta;
	ga_vector *population, *buffer;

	int provided,my_id,numprocs;
    MPI_Comm com; //mpi_comm determina quali processi sono coinvolti nella comunicazione
	MPI_Status status;
	com=MPI_COMM_WORLD; //così gli dico che ci sono tutti
	//MPI_Init_thread(&argc, &argv,MPI_THREAD_MULTIPLE,&provided) ;// Chiamata obbligatoria di inizializzazione -> programmazione ibrida	multiple -> i diversi processi possono avere più thread ma 1 solo thread alla volta può comunicare con gli altri
	MPI_Init_thread(&argc, &argv,MPI_THREAD_SINGLE,&provided) ;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id) ;     // Ottiene l'identificativo del processo
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs) ;  // Ottiene quanti processi sono attivi
	
	//ogni processo avrà la sua popolazione
	
	omp_set_num_threads(2);//setta il numero dei thread al numero dei processori

	if (my_id==0)//se sono il processo 0
	{
		cout <<"Frase da raggiungere: "<< GA_TARGET << endl;
	}

	srand(unsigned(time(NULL))*my_id);

	init_population(pop_alpha, pop_beta);//inizializza la popolazione
	population = &pop_alpha;
	buffer = &pop_beta;


	calc_fitness(*population);		// calcola il fitness per ogni elemento della popolazione
	sort_by_fitness(*population);	// ordina secondo il fitness crescente...il primo è quello che si avvicina di più
	cout <<"\nFrase e fitness migliore della prima popolazione random generata da "<< my_id << ": "<< endl;
	print_best(*population);		// stampa il prima del vettore ordinato e il suo fitness
	
	int nn=0;

	double start_time, total_time;
	start_time = omp_get_wtime();

	do //ciclo di evoluzione della popolazione, continua a evolvere il sistema finchè non hai ottenuto un certo score
	{
		if((*population)[0].fitness == 0) break;
		mate(*population, *buffer);		// mate the population together
		swapp(population, buffer);		// swap buffers
		calc_fitness(*population);
		sort_by_fitness(*population);
		nn++;
	} while(nn < GA_MAXITER);
	total_time = omp_get_wtime() - start_time;

	cout <<"Sono il processo " << my_id << ", ho terminato in " << total_time << " secondi e aspetto gli altri alla barriera!!" << endl;
	MPI_Barrier(com);

	if (my_id==0)//se sono il processo 0
	{
		int l;
		vector<string> topString;
		topString.push_back( getString(*population));
		MPI_Status status;
		int i;
		for(i=1; i<numprocs; i++)
		{//raccolgo i migliori da gli altri processi

			MPI_Probe(i, 0, com, &status); //riempie la struttura status con i dettagli del messaggio in arrivo,per poter pooi inizializzare le variabili adeguatamente 
			MPI_Get_count(&status, MPI_CHAR, &l);//conta quanti elementi del tipo MPI_CHAR sono contenuti nel messaggio in arrivo
			char *buf = new char[l]; //inizializzo un buffer di char per la stringa in arrivo
			MPI_Recv(buf, l, MPI_CHAR, i, 0, com, &status); //ricevo il messaggio mandato dal processo i di l elementi del tipo MPI_CHAR e lo metto in buf
			string str(buf, l);
			delete [] buf;
			topString.push_back(str);

		}
		cout <<"\nSono il processo 0 e ho ricevuto dagli altri processi: " << endl;
		int k=0,j,min;
		string frasemin;
		// per ogni stringa ricevuta la stampo e calcolo quella con il fitness migliore
		for( vector<string>::const_iterator i = topString.begin(); i != topString.end(); ++i)
		{
			cout << "\nStringa ricevuta da "<< k <<": " << *i << endl;
			if(k==0)
			{
				min = calc_fitness_single(*i);
				frasemin = *i;
			}
			else if (calc_fitness_single(*i) < min)
			{
				min = calc_fitness_single(*i);
				frasemin = *i;
			}
			k++;
		}
		cout << "\nStringa con il miglior fitness: " << frasemin << "(" << min << ")"<< endl;
		char c;
		cin >> c;
	}
	else // se non sono lo 0 faccio una send
	{ 
		string frase = getString(*population);
		MPI_Send(&frase[0], frase.length(), MPI_CHAR, 0, 0, com); //mando la frase al processo 0
	}

	MPI_Finalize();
	return 0;
}