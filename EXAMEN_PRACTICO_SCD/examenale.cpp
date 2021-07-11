#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> 
#include <chrono> 
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


Semaphore puede_poner = 1;
Semaphore puede_fumar[3] = { 0 , 0 , 0 };
Semaphore puede_sacar = 0;
Semaphore espacio_libre = 3;
int ingrediente = 0;
const int num_fumadores = 3;
const int tam_buzon     = 3;
int buzon[tam_buzon];
int cigarros_aportados[3]={ 0 , 0 , 0 };   //En la pos 1 se guarda el nº de cigarros aportados por el jugador 1...
int contador=0;
int espacios_libres=3;


mutex mtx;
//************************
// plantilla de función para generar un entero aleatorio uniformemente
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// hebra del estanquero

void funcion_hebra_estanquero(  )
{
	while(true){
	ingrediente = aleatorio<0,2>();
	sem_wait(puede_poner);
	mtx.lock();
	cout << "Se ha puesto el ingrediente: " << ingrediente << endl;
	mtx.unlock();
	sem_signal(puede_fumar[ingrediente]);
}
}

// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );
mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
mtx.unlock();
   this_thread::sleep_for( duracion_fumar );

mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
mtx.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador(int num_fumador)
{
   while( true )
   {
	sem_wait(puede_fumar[num_fumador]);
	mtx.lock();
	cout << "El fumador " << num_fumador << " ha cogido el ingrediente " << num_fumador << " del mostrador." << endl;
mtx.unlock();	
	sem_signal(puede_poner);
	sem_wait(espacio_libre);
	mtx.lock();
	buzon[tam_buzon - espacios_libres] = num_fumador;
	espacios_libres--;
	mtx.unlock();
	sem_signal(puede_sacar);	

   }
}

void funcion_hebra_contrabandista(){
	while(true){
	chrono::milliseconds duracion_suenio( aleatorio<20,150>() );
	this_thread::sleep_for( duracion_suenio );
	sem_wait(puede_sacar);
	
	int aux = buzon[0];

	for(int i=0 ; i<tam_buzon-1 ; i++)
		buzon[i] = buzon[i+1];
	
	cigarros_aportados[aux]++;
	mtx.lock();
	cout << "***************+++El jugador " << aux << " aporta un cigarrillo***************" << endl;
	mtx.unlock();
	espacios_libres++;
	
	sem_signal(espacio_libre);
	contador++;

	if(contador%4==0){
		for(int i=0 ; i<num_fumadores ; i++){
			mtx.lock();
			cout << "El jugador " << i+1 << " ha aportado " << cigarros_aportados[i] << " cigarros." << endl;
			mtx.unlock();
		}
	}
	}
	
}

	


int main()
{
   	thread hebra_estanquero(funcion_hebra_estanquero), hebra_contrabandista(funcion_hebra_contrabandista);
	thread hebra_fumador[num_fumadores];
	for(int i=0 ; i<num_fumadores ; i++)
		hebra_fumador[i] = thread (funcion_hebra_fumador, i);


	hebra_estanquero.join();
	hebra_contrabandista.join();
	for(int i=0 ; i<num_fumadores ; i++)
		hebra_fumador[i].join();

	
}
