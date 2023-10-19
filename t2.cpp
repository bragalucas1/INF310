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

const int chairsNumber = 5; //numero de cadeiras na sala de espera
const int clientsNumber = 10; //numero arbitrario de clientes
int clientsWhoEntriedBarberShop = 0; //contador inerente a função de parada

class BarberMonitor {
private:
    /*Declaração das variáveis a respeito de: 
    1) mutex para garantir a entrada das threads.
    2) condição variável para notificar
    3) numero de clientes que estão esperando na fila para cortar
    4) booleano a fim de verificar se o barbeiro esta ocioso ou não
    5) fila para colocar os que estão esperando o corte.*/
    mutex mux;
    condition_variable isSleep;
    int clientsWaiting;
    bool barberSleeping;
    queue<int> waitingToCut;

public:
    BarberMonitor() : clientsWaiting(0), barberSleeping(true) {}

    const int getQueueSize() { return waitingToCut.size(); }

    void cliente_chega(int id) {
        unique_lock<mutex> lck(mux);
        clientsWhoEntriedBarberShop++;

        if (clientsWaiting < chairsNumber) { /* Se o barbeiro está atendendo outro cliente, o cliente recém-chegado espera sentado em uma das cadeiras da sala de espera; */
            waitingToCut.push(id);
            cout << "Cliente: " << id << " entrou no salão para ficar na régua.\n";
            clientsWaiting++;
            
        } else if (clientsWaiting >= chairsNumber) {
            cout << "Cliente " << id << " cansou de esperar e foi embora." << endl;
        }

        if (!waitingToCut.empty()) { /*Temos clientes na fila, acorda o dorminhoco.*/
            barberSleeping = false;
            isSleep.notify_one();
        }
    }

    void barbeiro_faz_corte() {
        unique_lock<mutex> lck(mux);

        while (clientsWaiting == 0) { 
            barberSleeping = true;
            cout << "Barbeiro: não há clientes, irei tirar um cochilo. " << endl;
            isSleep.wait(lck); 
        }
        
        int id = waitingToCut.front();
        waitingToCut.pop();
        clientsWaiting--;
        cout << "Barbeiro atendeu o Cliente: " << id << endl;
        cout << "Temos na fila ainda: " << waitingToCut.size() << " clientes." << endl;
    }
};

BarberMonitor barbearia;

void barbeiro() {
    while (1) {
        barbearia.barbeiro_faz_corte();
        this_thread::sleep_for(chrono::milliseconds(60));
        this_thread::yield(); /*Permitir que outras threads entrem durante os cortes.*/
        if(clientsWhoEntriedBarberShop == clientsNumber && !(barbearia.getQueueSize())) break; 
        /*Acima, a condição de parada crucial: se o numero de clientes que entraram na barbearia for o número setado de clientes total E 
        >>não houver mais ninguem na fila para cortar<< encerra o atendimento.*/
    }
    cout << "Barbeiro encerrou o expediente. Happy Hour!!!" << endl;
}

void cliente(int id) {
    barbearia.cliente_chega(id);
}

int main() {
    /* Declaração das threads - clientes teremos inúmeras e apenas 1 para o barbeiro.*/
    vector<thread> clientThreads;
    thread barber(barbeiro);

    for (int i = 0; i < clientsNumber; i++) {
        thread clientT(cliente, i);
        clientThreads.push_back(move(clientT));
    }

    for (int i = 0; i < clientsNumber; i++) {
        clientThreads[i].join();
    }

    barber.join();
    return 0;
}