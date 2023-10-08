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
mutex mux;
mutex printerMux;

void wakeup(thread::id t);

void block();

class Printer{
private:
    string printerType;
    int numPrintersAvaliable;
    bool isAvaliable = false;

    public:
    Printer(const string &printerType, int numPrintersAvaliable) : printerType(printerType), numPrintersAvaliable(numPrintersAvaliable), isAvaliable(true) {}
    
    bool requirePrinter(bool processCondition, char tag, Printer printerType){
        cout << "Tag: " << tag << " entrou.\n";
        while(!processCondition){
            if(numPrintersAvaliable > 0){
                cout << "Processo " << tag << " está utilizando a impressora.\n";
                numPrintersAvaliable--;
                this_thread::sleep_for(std::chrono::milliseconds(1));
                printerType.freePrinter();
                cout << "Processo " << tag << " liberou a impressora.\n";
                processCondition = true;
                return true;
            }

            else{
                cout << "Não há impressoras disponíveis." << endl;
                isAvaliable = false;
                bloqueadas.push(this_thread::get_id());
                block();
            }
        }
        return false;
    };

    void freePrinter(){
        printerMux.lock();
        if(bloqueadas.size() > 0){
            thread::id blockedThread = bloqueadas.front();
            wakeup(blockedThread);
            bloqueadas.pop();
        }
        ++numPrintersAvaliable;
        isAvaliable = true;
        printerMux.unlock();
    }   
};

void wakeup(thread::id t) {
    mux.lock();
    A[t]++;
    mux.unlock();
}

void block() {
    bool sair = false;
    thread::id eu = this_thread::get_id();
    do {
        mux.lock();
        if(A[eu] > 0) {
            A[eu]--;
            sair = true;
        }
        mux.unlock();
    } while (!sair); 
}

void processA(Printer &laserPrinter){
    bool processAOk = false;
    char tag = 'A';
    laserPrinter.requirePrinter(processAOk, tag, laserPrinter);
}

void processB(Printer &jetPrinter){
    bool processBOk = false;
    char tag = 'B';
    jetPrinter.requirePrinter(processBOk, tag, jetPrinter);
}

void processC(Printer &laserPrinter, Printer &jetPrinter){
    bool processCOk = false;
    char tag = 'C';
    jetPrinter.requirePrinter(processCOk, tag, jetPrinter);
    laserPrinter.requirePrinter(processCOk, tag, laserPrinter);
}

int main(){
    /*No sistema existem 2 impressoras de cada tipo e 5 processos de cada tipo.*/
    Printer laserPrinters("Laser", 2);
    Printer jetPrinters("Jet", 2);
    vector<thread> processThreadsA;
    vector<thread> processThreadsB;
    vector<thread> processThreadsC;

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