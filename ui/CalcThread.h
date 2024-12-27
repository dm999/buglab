#ifndef CALC_THREAD_H
#define CALC_THREAD_H

//https://docwiki.embarcadero.com/CodeExamples/Sydney/en/SleepSort_(C%2B%2B)
//https://docwiki.embarcadero.com/CodeExamples/Athens/en/TThreadList_(C%2B%2B)
class TCalcThread : public TThread {
private:

    TCriticalSection *FLock;
    TBUIForm * Form;

    void __fastcall Execute();

    void __fastcall repaint();

public:
    TCalcThread(bool CreateSuspended, TCriticalSection *ALock, TBUIForm * f);

    bool isTerminated() const
    {
        return this->Terminated;
    }
};

#endif

