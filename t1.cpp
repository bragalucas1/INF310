/*Lucas Braga Moura - INF310 - Trabalho prático 1.
Matrícula: 98909
UFV - 2023. */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <queue>

using namespace std;

const int numProcessForEachType = 5;
const int numPrinterForEachType = 2;
map<thread::id,int> A;
queue<thread::id> bloqueadas;
mutex globalMux;

void wakeup(thread::id t);

void block();

class Printer{
private:
    string printerType;
    int numPrintersAvaliable;
    bool isAvaliable = false;

    public:
    Printer(const string &printerType, int numPrintersAvaliable) : printerType(printerType), numPrintersAvaliable(numPrintersAvaliable), isAvaliable(true) {}
    
    bool requirePrinter(){
        globalMux.lock();
        if(numPrintersAvaliable > 0){
            if(bloqueadas.size() > 0){
                thread::id blockedThread = bloqueadas.front();
                wakeup(blockedThread);
                bloqueadas.pop();
            }
            numPrintersAvaliable--;
            return true;
        }

        else{
            isAvaliable = false;
            bloqueadas.push(this_thread::get_id());
            block();
        }

        return false;
        globalMux.unlock();
    };

    void freePrinter(){
        globalMux.lock();
        ++numPrintersAvaliable;
        isAvaliable = true;
        globalMux.lock();
    };
};

void wakeup(thread::id t) {
    globalMux.lock();
    A[t]++;
    globalMux.unlock();
}

void block() {
    bool sair = false;
    thread::id eu = this_thread::get_id();
    do {
        globalMux.lock();
        if(A[eu] > 0) {
        A[eu]--;
        sair = true;
        }
        globalMux.unlock();
    } while (!sair); 
}

void processA(Printer &laserPrinter){
    bool processAOk = false;
    while(!processAOk){
        if(laserPrinter.requirePrinter()){
            printf("Processo A está utilizando a impressora a Laser.\n");
            this_thread::sleep_for(std::chrono::milliseconds(1));
            laserPrinter.freePrinter();
            printf("Processo A liberou a impressora a Laser.\n");
            processAOk = true;
        }
    }
}

void processB(Printer &jetPrinter){
    bool processBOk = false;
    while(!processBOk){
        if(jetPrinter.requirePrinter()){
            printf("Processo B está utilizando a impressora a Jato.\n");
            this_thread::sleep_for(std::chrono::milliseconds(600));
            jetPrinter.freePrinter();
            printf("Processo B liberou a impressora a Jato.\n");
            processBOk = true;
        }
    }
}

void processC(Printer &laserPrinter, Printer &jetPrinters){
    printf("Not implemented.\n");
}

int main(){
    /*No sistema existem 2 impressoras de cada tipo e 5 processos de cada tipo.*/
    Printer laserPrinters("Laser", 2);
    Printer jetPrinters("Jet", 2);
    vector<thread> processThreadsA(numProcessForEachType);
    vector<thread> processThreadsB(numProcessForEachType);
    vector<thread> processThreadsC(numProcessForEachType);

    //Criação de 5 processos para cada tipo - 5A, 5B E 5C.
    for (int i = 0; i < numProcessForEachType; i++) {
        thread tA(processA, ref(laserPrinters));
        A[tA.get_id()] = 0;
        processThreadsA.push_back(move(tA));
   
        thread tB(processB, ref(jetPrinters));
        A[tB.get_id()] = 0;
        processThreadsB.push_back(move(tB));
    
        thread tC(processC, ref(laserPrinters), ref(jetPrinters));
        A[tC.get_id()] = 0;
        processThreadsC.push_back(move(tC));
    }

    for (int i = 0; i < numProcessForEachType; i++) {
        processThreadsA[i].join();
        processThreadsB[i].join();
        processThreadsC[i].join();
    }

    return 0;
}