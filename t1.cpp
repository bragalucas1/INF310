/*Lucas Braga Moura - INF310 - Trabalho prático 1.
Matrícula: 98909
UFV - 2023. */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
using namespace std;

const int numProcessForEachType = 5;
const int numPrinterForEachType = 2;
bool processAOk = false;
bool processBOk = false;


class Printer{
private:
    string printerType;
    int numPrintersAvaliable;
    bool isAvaliable = false;
    condition_variable condition;
    mutex m1x;

    public:
    Printer(const string &printerType, int numPrintersAvaliable) : printerType(printerType), numPrintersAvaliable(numPrintersAvaliable), isAvaliable(true) {}

    bool requirePrinter(){
        unique_lock<mutex> lock(m1x);
        condition.wait(lock, [this] { return numPrintersAvaliable > 0; });

        if(numPrintersAvaliable > 0){
            --numPrintersAvaliable; //Usei pré-decremento com o receio de usar o pós, visto que quando uma thread tentasse acessar antes da diminuição do valor, pudesse gerar algum conflito.
            return true;
        }

        return false;
    };

    void freePrinter(){
        unique_lock<mutex> lock(m1x);
        ++numPrintersAvaliable;
        isAvaliable = true;
        condition.notify_all(); // Notifique uma thread em espera que a impressora está disponível

    };
};


void processA(Printer &laserPrinter){
// Processos do tipo A podem usar apenas impressoras Laser. 
    while (true) {
        if (laserPrinter.requirePrinter()) {
            printf("Processo A está utilizando a impressora a Laser.\n");
            this_thread::sleep_for(chrono::milliseconds(1));
            laserPrinter.freePrinter();
            printf("Impressora a laser liberada pelo processo A.\n");
            break;
        }
    }
}

void processB(Printer &jetPrinter){
// Processos do tipo B podem usar apenas impressoras Jato de Tinta. 
    while (true) {
        if (jetPrinter.requirePrinter()) {
            printf("Processo B está utilizando a impressora a Laser.\n");
            this_thread::sleep_for(chrono::milliseconds(1));
            jetPrinter.freePrinter();
            printf("Impressora a laser liberada pelo processo B.\n");
            break;
        }
    }
}

void processC(Printer &laserPrinter, Printer &jetPrinters){
// Processos do tipo C podem usar qualquer tipo de impressora, com preferência para impressoras Laser, quando existem os dois tipos de impressora disponíveis.
    while (true) {
        if (jetPrinters.requirePrinter()) {
            printf("Processo C está utilizando a impressora a Jato.\n");
            this_thread::sleep_for(chrono::milliseconds(1));
            jetPrinters.freePrinter();
            printf("Impressora a Jato liberada pelo processo C.\n");
            break;
        }
        else if(laserPrinter.requirePrinter()){
            printf("Processo C está utilizando a impressora a Jato.\n");
            this_thread::sleep_for(chrono::milliseconds(1));
            laserPrinter.freePrinter();
            printf("Impressora a laser liberada pelo processo C..\n");
            break;
        }
    }
}

int main(){
    /*No sistema existem 2 impressoras de cada tipo e 5 processos de cada tipo.*/
    Printer laserPrinters("Laser", 2);
    Printer jetPrinters("Jet", 2);
    vector<thread> processThreadsA(numProcessForEachType);
    vector<thread> processThreadsB(numProcessForEachType);
    vector<thread> processThreadsC(numProcessForEachType);

    //Criação de 5 processos para cada tipo - 5A, 5B E 5C.
    for (int i = 0; i < numProcessForEachType; ++i) {
        processThreadsA[i] = thread([&laserPrinters](){
            processA(laserPrinters);
        });
        processThreadsB[i] = thread([&jetPrinters](){
            processB(jetPrinters);
        });
        processThreadsC[i] = thread([&jetPrinters, &laserPrinters](){
            processC(laserPrinters, jetPrinters);
        });        
    }

    for (int i = 0; i < numProcessForEachType; ++i) {
        processThreadsA[i].join();
        processThreadsB[i].join();
        processThreadsC[i].join();
    }

    return 0;
}