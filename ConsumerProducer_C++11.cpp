// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

struct BoundedBuffer{
	int *buffer; //buffer di elementi
	int capacity; //capacità del buffer

	int front;
	int rear;
	int count;

	std::mutex lock; //creo una mutex per l'accesso al buffer

	std::condition_variable not_full;
	std::condition_variable not_empty;

	BoundedBuffer(int capacity) : capacity(capacity), front(0), rear(0), count(0) { //costruttore
		buffer = new int[capacity];
	}

	~BoundedBuffer(){ //distruttore
		delete[] buffer;
	}

	void deposit(int data){//funzione chiamata dal producer
		std::unique_lock<std::mutex> l(lock); 

		not_full.wait(l, [this](){return count != capacity; });//aspetto finche qualcuno non mi dice che procedere richiamando not_full.notify
															//intanto rilascio il lock che avevo acquisito e mi metto in coda
															//se count != capacity è false continuo ad aspettare
															//se count != capacity posso procedere e riprendere il lock per proseguire
		buffer[rear] = data;//aggiungo al buffer il dato
		rear = (rear + 1) % capacity; //incremento la posizione
		++count;

		not_empty.notify_one();//segnalo a uno dei thread consumer che è in attesa della condition_variable not_empty che può eseguire
	}

	int fetch(){//funzione chiamata dal consumer
		std::unique_lock<std::mutex> l(lock);

		not_empty.wait(l, [this](){return count != 0; });//se count = 0 aspetto

		int result = buffer[front];//prendo un elemento dalla testa
		front = (front + 1) % capacity;//sposto la testa del buffer all'elemento successivo
		--count;//decremento il numero di elementi nel buffer

		not_full.notify_one();

		return result;
	}
};

void consumer(int id, BoundedBuffer& buffer){
	for (int i = 0; i < 50; ++i){ //per 50 volte vado a prendere elementi dal buffer
		int value = buffer.fetch();
		std::cout << "Consumer " << id << " fetched " << value << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
}

void producer(int id, BoundedBuffer& buffer){
	for (int i = 0; i < 75; ++i){//per 75 volte vado a depositare i elementi
		buffer.deposit(i);
		std::cout << "Producer " << id << " produced " << i << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main(){

	BoundedBuffer buffer(200);//buffer inizializzato a 200

	std::thread c1(consumer, 0, std::ref(buffer));//thread c1 chiama la funzione consumer con parametri 0 e un puntatore a buffer
	std::thread c2(consumer, 1, std::ref(buffer));
	std::thread c3(consumer, 2, std::ref(buffer));
	std::thread p1(producer, 0, std::ref(buffer));
	std::thread p2(producer, 1, std::ref(buffer));

	c1.join();//mette al lavoro i processi...il thread principale si blocca in attesa che i thread in join finiscano
	c2.join();
	c3.join();
	p1.join();
	p2.join();
	system("pause");

	return 0;

	return 0;
}
