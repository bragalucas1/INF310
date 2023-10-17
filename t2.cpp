#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include<unistd.h>

/* Lucas Braga Moura - 98909
Trabalho Prático 2 - INF 310 */

using namespace std;

const int chairNumber = 5; //numero de cadeiras na sala de espera
const int barberChair = 1; //1 cadeira onde o barbeiro corta
const int clientNumber = 10; //numero arbitrario de clientes
int clientsWhoEntriedBarberShop = 0;

class BarberMonitor {
private:
    mutex mux;
    condition_variable isSleep;
    int clientsWaiting;
    bool barberSleeping;
    queue<int> waitingToCut;

public:
    BarberMonitor() : clientsWaiting(0), barberSleeping(true) {
    }

    const int getQueueSize() { return waitingToCut.size(); }

    void cliente_chega(int id) {
        unique_lock<mutex> lck(mux);
        clientsWhoEntriedBarberShop++;
        
        if (clientsWaiting < chairNumber) { /*Se o barbeiro está atendendo um outro cliente, o cliente recém-chegado espera sentado em uma das cadeiras da sala de espera;*/
            waitingToCut.push(id);
            clientsWaiting++;
    
            /* Se o barbeiro está dormindo, ele o acorda para poder ser atendido;*/
            if (waitingToCut.size() == 1) {
                barberSleeping = false;
                isSleep.notify_one();
            }
            
        } else {
            /*Se todas as cadeiras da sala de espera estão ocupadas, ele desiste (vai embora e não volta mais).*/
            cout << "Cliente " << id << " desistiu de esperar e procurou outra barbearia." << endl;
        }
    }

    void barbeiro_faz_corte() {
        unique_lock<mutex> lck(mux);

        cout << "Clients Waiting: " << clientsWaiting << endl;
        if (clientsWaiting == 0) { /*Ele não deve dormir se houver algum cliente esperando para ser atendido;*/
            cout << "Barbeiro: não há clientes, irei tirar um cochilo. " << endl;
            isSleep.wait(lck); //e a fila não estiver vazia, isso significa que um cliente chegou e o barbeiro deve acordar para atendê-lo.
        }
        
        cout << "Opa, chegou cliente!" << endl;
        int id = waitingToCut.front();
        waitingToCut.pop();
        clientsWaiting--;
        // Barbeiro atende o cliente
        cout << "Barbeiro atendeu o Cliente: " << id << endl;
        cout << "Temos na fila ainda: " << waitingToCut.size() << " clientes." << endl;
        this_thread::sleep_for(chrono::seconds(4));
        //lck.unlock();
    }
};

BarberMonitor barbearia;

void barbeiro() {
    while (1) {
        barbearia.barbeiro_faz_corte();
        if(clientsWhoEntriedBarberShop == clientNumber && !(barbearia.getQueueSize())) break; 
    }

    cout << "Barbeiro encerrou o expediente." << endl;
    
}

void cliente(int id) {
    this_thread::sleep_for(chrono::seconds((1/2)));
    barbearia.cliente_chega(id);
}

int main() {
    vector<thread> clientThreads;
    thread barber(barbeiro);

    for (int i = 0; i < clientNumber; i++) {
        thread clientT(cliente, i);
        clientThreads.push_back(move(clientT));
    }

    for (int i = 0; i < clientNumber; i++) {
        clientThreads[i].join();
    }
    
    barber.join();

    return 0;
}