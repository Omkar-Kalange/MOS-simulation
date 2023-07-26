#include<bits/stdc++.h>

using namespace std;

void print_memory();
void print_drum();
void init();
int allocate();
int get_drum_track();
void addressMap();
void executeUserProgram();
void simulation();
void MOS();
void IR(int);
void Terminate();

fstream input;
fstream output;
char M[300][4];
int n_available_frames=30;
int n_available_tracks=50;
char drum[500][4];
int SI;   // service interrupt
int TI;   //time interrupt
int PI;   //programming interrupt
int IOI; // Input-Output interrupt
int EM;
int VA[2];
int RA;
int Timer=0;
int page_fault=0; // 0:Invalid, 1:Valid

class CPU
{
    public:
        char R[4];
        char IR[4];
        int IC[2];
        int PTR[4];
        bool C;
        int TS=5;
        int TSC;

        static CPU * getObject()
        {
            return CPU::object;
        }

    private:
        CPU(){};
        static CPU * object;

};
CPU* CPU::object = new CPU;
CPU *cpu = CPU::getObject();

struct PCB
{
    char R[4];
    int PTR, IC=0;
    bool C;

    int read = 0; //0: Card reading incomplete, 1: Card reading complete
    int IO = 0; //0: NO I/O, 1: Read, 2: Write

    string JOB_ID;
    int TTL,TLL,TTC,LLC;

    vector <int> T_ProgramCard, T_DataCard, T_OutputLines;
    int n_ProgramCard=0, n_DataCard=0;
    int n_ProgramCard_copy=0, n_DataCard_copy=0;
    char flag;      // P:Program card or D: Data Card


    int abTerminate = 0;    //0: Normal Termination, 1: Abnormal Termination
    vector <int> TC; //Termination codes
} *pcb;

struct ProgramQueue
{
    PCB *pcb = new PCB;
    ProgramQueue* next=NULL;

    ~ProgramQueue()
    {
        delete pcb;
    }
} *PQ = NULL /*Pending Queue*/, *LQ = NULL, *RQ = NULL, *IOQ = NULL, *TQ = NULL, *SQ = NULL;


void PQ_to_LQ()
{
    cout<<"            Input spooling complete for job "<<PQ->pcb->JOB_ID<<". Moving PCB to LQ. "<<endl;

    if(LQ)
    {
        struct ProgramQueue* temp = LQ;
        while(temp->next)
            temp = temp->next;
        temp->next = PQ;
        temp = temp->next;
        PQ = PQ->next;
        temp->next = NULL;
    }
    else
    {
        LQ = PQ;
        PQ = PQ->next;
        LQ->next = NULL;
    }
}


void LQ_to_RQ()
{
    cout<<"                Loading complete for job "<<LQ->pcb->JOB_ID<<". Moving PCB to RQ. "<<endl;

    if(RQ)
    {
        struct ProgramQueue* temp = RQ;
        while(temp->next)
            temp = temp->next;
        temp->next = LQ;
        temp = temp->next;
        LQ = LQ->next;
        temp->next = NULL;
    }
    else
    {
        RQ = LQ;
        LQ = LQ->next;
        RQ->next = NULL;
    }
}


void RQ_to_RQ()
{
    RQ->pcb->R[0] = cpu->R[0];
    RQ->pcb->R[1] = cpu->R[1];
    RQ->pcb->R[2] = cpu->R[2];
    RQ->pcb->R[3] = cpu->R[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->C = cpu->C;

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"            Time slice out for job "<<RQ->pcb->JOB_ID<<". Moving PCB to end of RQ. "<<endl<<endl<<endl;

    if(RQ->next)
    {
        struct ProgramQueue* temp = RQ;
        while(temp->next)
            temp = temp->next;
        temp->next = RQ;
        temp = temp->next;
        RQ = RQ->next;
        temp->next = NULL;
    }
}


void RQ_to_IOQ()
{
    RQ->pcb->R[0] = cpu->R[0];
    RQ->pcb->R[1] = cpu->R[1];
    RQ->pcb->R[2] = cpu->R[2];
    RQ->pcb->R[3] = cpu->R[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->C = cpu->C;

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"            IO request for job "<<RQ->pcb->JOB_ID<<". Moving PCB to IOQ. "<<endl<<endl<<endl;

    if(IOQ)
    {
        struct ProgramQueue* temp = IOQ;
        while(temp->next)
            temp = temp->next;
        temp->next = RQ;
        temp = temp->next;
        RQ = RQ->next;
        temp->next = NULL;
    }
    else
    {
        IOQ = RQ;
        RQ = RQ->next;
        IOQ->next = NULL;
    }
}


void IOQ_to_RQ()
{
    cout<<"                IO request for job "<<IOQ->pcb->JOB_ID<<" completed. Moving PCB to RQ. "<<endl;

    if(RQ)
    {
        struct ProgramQueue* temp = RQ;
        while(temp->next)
            temp = temp->next;
        temp->next = IOQ;
        temp = temp->next;
        IOQ = IOQ->next;
        temp->next = NULL;
    }
    else
    {
        RQ = IOQ;
        IOQ = IOQ->next;
        RQ->next = NULL;
    }
}


void RQ_to_SQ()
{
    RQ->pcb->R[0] = cpu->R[0];
    RQ->pcb->R[1] = cpu->R[1];
    RQ->pcb->R[2] = cpu->R[2];
    RQ->pcb->R[3] = cpu->R[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->C = cpu->C;

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"            Frame not avlbl, Suspending job "<<RQ->pcb->JOB_ID<<endl;

    if(SQ)
    {
        struct ProgramQueue* temp = SQ;
        while(temp->next)
            temp = temp->next;
        temp->next = RQ;
        temp = temp->next;
        RQ = RQ->next;
        temp->next = NULL;
    }
    else
    {
        SQ = RQ;
        RQ = RQ->next;
        SQ->next = NULL;
    }
}


void SQ_to_RQ()
{
    cout<<"                SQ to RQ for job "<<SQ->pcb->JOB_ID<<endl;

    if(RQ)
    {
        struct ProgramQueue* temp = RQ;
        while(temp->next)
            temp = temp->next;
        temp->next = SQ;
        temp = temp->next;
        SQ = SQ->next;
        temp->next = NULL;
    }
    else
    {
        RQ = SQ;
        SQ = SQ->next;
        RQ->next = NULL;
    }
}


void RQ_to_TQ()
{
    RQ->pcb->R[0] = cpu->R[0];
    RQ->pcb->R[1] = cpu->R[1];
    RQ->pcb->R[2] = cpu->R[2];
    RQ->pcb->R[3] = cpu->R[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->C = cpu->C;

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"            Terminating job "<<RQ->pcb->JOB_ID<<endl<<endl;

    if(TQ)
    {
        struct ProgramQueue* temp = TQ;
        while(temp->next)
            temp = temp->next;
        temp->next = RQ;
        temp = temp->next;
        RQ = RQ->next;
        temp->next = NULL;
    }
    else
    {
        TQ = RQ;
        RQ = RQ->next;
        TQ->next = NULL;
    }
}


void IOQ_to_TQ()
{
    cout<<" Terminating job "<<IOQ->pcb->JOB_ID<<endl;

    if(TQ)
    {
        struct ProgramQueue* temp = TQ;
        while(temp->next)
            temp = temp->next;
        temp->next = IOQ;
        temp = temp->next;
        IOQ = IOQ->next;
        temp->next = NULL;
    }
    else
    {
        TQ = IOQ;
        IOQ = IOQ->next;
        TQ->next = NULL;
    }
}


class SupervisoryStorage
{
    SupervisoryStorage();
    static SupervisoryStorage *object;

    struct node
    {
        char buff[40];
        struct node* next;

        node()
        {
            for (int j=0; j<40; j++)
                buff[j] = '-';

            next = NULL;
        }
    };

    public:
        node* temp;
        node *ebq = NULL;
        node *ifbq = NULL;
        node *ofbq = NULL;
        static SupervisoryStorage * getObject()
        {
            return SupervisoryStorage::object;
        }

        void ebq_to_ifbq();
        void ebq_to_ofbq();
        void last_ifb_to_ebq();
        void first_ifb_to_ebq();
        void ofbq_to_ebq();
        void print_ebq();
        void print_ifbq();
        void print_ofbq();
};
SupervisoryStorage::SupervisoryStorage()
{
    ebq = new node;
    temp = ebq;
    for (int i=0; i<9; i++)
    {
        temp->next = new node;
        temp = temp->next;
    }
}
void SupervisoryStorage::ebq_to_ifbq()
{
    if (ifbq)
    {
        temp = ifbq;
        while(temp->next)
            temp = temp->next;
        temp->next = ebq;
        temp = temp->next;
    }
    else
    {
        ifbq = ebq;
        temp = ifbq;
    }

        ebq = ebq->next;
        temp->next = NULL;
}
void SupervisoryStorage::ebq_to_ofbq()
{
    if (ofbq)
    {
        temp = ofbq;
        while(temp->next)
            temp = temp->next;
        temp->next = ebq;
        temp = temp->next;
    }
    else
    {
        ofbq = ebq;
        temp = ofbq;
    }

        ebq = ebq->next;
        temp->next = NULL;
}
void SupervisoryStorage::last_ifb_to_ebq()
{
    temp = ifbq;

    if(temp->next)
    {
        while(temp->next->next)
            temp = temp->next;

        for(int i=0; i<40; i++)
            temp->next->buff[i] = '-';

        if (ebq)
        {
            node *temp2 = ebq;
            while(temp2->next)
                temp2 = temp2->next;

            temp2->next = temp->next;
            temp->next = NULL;
        }
        else
        {
            ebq = temp->next;
            temp->next = NULL;
        }
    }
    else
    {
        for(int i=0; i<40; i++)
            temp->buff[i] = '-';

        if (ebq)
        {
            node *temp2 = ebq;
            while(temp2->next)
                temp2 = temp2->next;

            temp2->next = temp;
            ifbq = NULL;
        }
        else
        {
            ebq = temp;
            ifbq = NULL;
        }
    }
}
void SupervisoryStorage::first_ifb_to_ebq()
{
    for(int i=0; i<40; i++)
        ifbq->buff[i] = '-';

    if (ebq)
    {
        temp = ebq;
        while(temp->next)
            temp = temp->next;

        temp->next = ifbq;
        ifbq = ifbq->next;
        temp->next->next = NULL;
    }
    else
    {
        ebq = ifbq;
        ifbq = ifbq->next;
        ebq->next = NULL;
    }

}
void SupervisoryStorage::ofbq_to_ebq()
{
    for(int i=0; i<40; i++)
        ofbq->buff[i] = '-';

    if (ebq)
    {
        temp = ebq;
        while(temp->next)
            temp = temp->next;

        temp->next = ofbq;
        ofbq = ofbq->next;
        temp->next->next = NULL;
    }
    else
    {
        ebq = ofbq;
        ofbq = ofbq->next;
        ebq->next = NULL;
    }

}
void SupervisoryStorage::print_ebq()
{
    cout<<"********EBQ start********"<<endl;
    temp = ebq;

    int i=1;
    while(temp)
    {
        cout<<i++<<") ";
        for(int i=0; i<40; i++)
            cout<<temp->buff[i];
        temp = temp->next;
        cout<<endl;
    }
    cout<<"********EBQ over********"<<endl;
}
void SupervisoryStorage::print_ifbq()
{
    cout<<"********IFBQ start********"<<endl;
    temp = ifbq;

    int i=1;
    while(temp)
    {
        cout<<i++<<") ";
        for(int i=0; i<40; i++)
            cout<<temp->buff[i];
        temp = temp->next;
        cout<<endl;
    }
    cout<<"********IFBQ over********"<<endl;
}
void SupervisoryStorage::print_ofbq()
{
    cout<<"********OFBQ start********"<<endl;
    temp = ofbq;

    int i=1;
    while(temp)
    {
        cout<<i++<<") ";
        for(int i=0; i<40; i++)
            cout<<temp->buff[i];
        temp = temp->next;
        cout<<endl;
    }
    cout<<"********OFBQ over********"<<endl;
}
SupervisoryStorage* SupervisoryStorage::object = new SupervisoryStorage();
SupervisoryStorage* S = SupervisoryStorage::getObject();

class CH1
{
    private:
        CH1(){};
        static CH1* object;

    public:
        int flag = 0;   //0:stopped, 1:working
        int chTotalTime = 5;
        int chTimer;

        static CH1* getObject()
        {
           return CH1::object;
        }
        void startChannel()
        {
            chTimer = 0;
            flag = 1;
            IOI-=1;
        }
        void stopChannel()
        {
            flag = 0;
        }
};
CH1* CH1 :: object = new CH1;
CH1* ch1 = CH1::getObject();

class CH2
{
    private:
        CH2(){};
        static CH2* object;

    public:
        int flag = 0;   //0:stopped, 1:working
        int chTotalTime = 5;
        int chTimer;

        static CH2* getObject()
        {
           return CH2::object;
        }
        void startChannel()
        {
            chTimer = 0;
            flag = 1;
            IOI-=2;
        }
        void stopChannel()
        {
            flag = 0;
        }
};
CH2* CH2 :: object = new CH2;
CH2* ch2 = CH2::getObject();

class CH3
{
    private:
        static CH3* object3;
        CH3(){};

    public:
        int flag = 0;   //0:stopped, 1:working
        int chTotalTime = 2;
        int chTimer;
        string Task="";
        static CH3* getObject3()
        {
           return CH3::object3;
        }
        void startChannel()
        {
            chTimer = 0;
            flag = 1;
            IOI-=4;
        }
        void stopChannel()
        {
            flag = 0;
        }
};
CH3* CH3 :: object3 = new CH3;
CH3* ch3 = CH3::getObject3();


void print_memory()
{
    cout<<endl<<"******* Main Memory ********"<<endl;
    cout<<"           -----------------"<<endl;
    for(int i=0;i<300;i++)
    {
        cout<<"    ";
        if (i<10) cout<<"00";
        else if (i<100) cout<<'0';

        cout<<i<<"    | ";
        for(int j=0;j<4;j++)
        {
            cout<<M[i][j]<<" | ";
        }
        cout<<endl;
        if (i%10 == 9) cout<<"           -----------------"<<endl;
    }
    cout<<endl<<endl<<endl;
}


void print_drum()
{
    cout<<endl<<"*********** DRUM ***********"<<endl;
    cout<<"           -----------------"<<endl;
    for(int i=0; i<500;i++)
    {
        cout<<"    ";
        if (i<10) cout<<"00";
        else if (i<100) cout<<'0';

        cout<<i<<"    | ";
        for(int j=0; j<4;j++)
            cout<<drum[i][j]<<" | ";
        cout<<endl;
        if (i%10 == 9) cout<<"           -----------------"<<endl;
    }
    cout<<endl<<endl<<endl;
}


void init()
{
    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    for(int i=0;i<300;i++)
    {
        for(int j=0;j<4;j++)
        {
            M[i][j] = '-';
        }
    }

    for(int i=0;i<500;i++)
    {
        for(int j=0;j<4;j++)
        {
            drum[i][j] = '-';
        }
    }

    SI=0;
    TI=0;
    PI=0;
    IOI=1;

    Timer=0;
}


int allocate()
{
    label:

    int allNum = rand()%30;

    //check if the allocated frame is empty
    for(int i=allNum*10; i<allNum*10+10 ; i++)
    {
        for(int j =0; j<4; j++)
        {
            if(M[i][j] != '-')
                goto label;
        }
    }

    //check if the frame was already allocated in page table of any process in the memory;
    for(int i=0; i<300 ; i++)
    {
        if(M[i][0] == '*')
        {
            if(((M[i][2]-'0')*10+(M[i][3]-'0')) == allNum)
                goto label;
        }
        else
            i+=9;
    }



    n_available_frames--;
    return allNum;
}


int get_drum_track()
{
    for(int i=0; i<500; i+=10)
    {
        int flag = 1;
        for(int j=i; j<i+10; j++)
        {
            if(flag)
                for(int k=0; k<4; k++)
                {
                    if(drum[j][k] != '-')
                    {
                        flag = 0;
                        break;
                    }
                }
            else
                break;
        }
        if (flag)
            return i/10;
    }
    return -1;
}


void addressMap()
{
    if(0<=VA[0] && VA[0]<=9 && 0<=VA[1] && VA[1]<=9)
    {
        int va = VA[0]*10 + VA[1];

        int PTE=(cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3]) + (va/10);

        if(M[PTE][0]=='*' && M[PTE][1]=='*' && M[PTE][2]=='*' && M[PTE][3]=='*')
        {
            PI=3;
            page_fault = 0;
        }
        else
        {
            RA = ((M[PTE][0]-'0')*1000+(M[PTE][1]-'0')*100+(M[PTE][2]-'0')*10+(M[PTE][3]-'0')) *10 +(va%10);
        }
    }
    else
    {
      PI=2;
    }
}


void executeUserProgram()
{
    if(!RQ)
    {
        cpu->TSC = 0;
        return;
    }

    if(cpu->IC[0]=='-' && cpu->IC[1]=='-')
    {
        cpu->R[0] = RQ->pcb->R[0];
        cpu->R[1] = RQ->pcb->R[1];
        cpu->R[2] = RQ->pcb->R[2];
        cpu->R[3] = RQ->pcb->R[3];

        cpu->IC[1] = RQ->pcb->IC%10;
        cpu->IC[0] = RQ->pcb->IC/10;

        cpu->PTR[3] = RQ->pcb->PTR%10;
        cpu->PTR[2] = RQ->pcb->PTR/10%10;
        cpu->PTR[1] = RQ->pcb->PTR/100%10;
        cpu->PTR[0] = 0;

        cpu->C = RQ->pcb->C;

        cpu->TSC = 0;
    }

    VA[0] = cpu->IC[0];
    VA[1] = cpu->IC[1];
    addressMap();

    cout<<"    CPU:"<<endl;
    if (PI!=0)
    {
        return;
    }

    for(int i=0;i<4;i++)
    {
        cpu->IR[i]=M[RA][i];
    }

    cpu->IC[1]+=1;
    if(cpu->IC[1]==10)
    {
        cpu->IC[1]=0;
        cpu->IC[0]+=1;
    }

    cout<<"       Job: "<<RQ->pcb->JOB_ID<<endl;
    cout<<"        IC: "<<cpu->IC[0]<<cpu->IC[1]<<endl;
    cout<<"        IR: "<<cpu->IR[0]<<cpu->IR[1]<<cpu->IR[2]<<cpu->IR[3]<<endl;
    cout<<"        TS: "<<cpu->TSC<<"-"<<cpu->TSC+1<<endl;
    cout<<"       TTC: "<<RQ->pcb->TTC<<endl;

    if(cpu->IR[0]!='H')
    {
        VA[0] = cpu->IR[2]-'0';
        VA[1] = cpu->IR[3]-'0';
        addressMap();
    }

    if(PI == 3)
    {
        if(cpu->IR[0]=='G' && cpu->IR[1]=='D' || cpu->IR[0]=='S' && cpu->IR[1]=='R')
            page_fault = 1;
        return;
    }
    if(PI == 2) return;

    if(cpu->IR[0]=='G' && cpu->IR[1]=='D')
    {
        cpu->IR[3] = '0';
        SI=1;
    }
    else if(cpu->IR[0]=='P' && cpu->IR[1]=='D')
    {
        cpu->IR[3] = '0';
        SI=2;
    }
    else if(cpu->IR[0]=='L' && cpu->IR[1]=='R')
    {
        cout<<"            Loading \"";
        for(int i=0;i<4;i++)
        {
            cpu->R[i] = M[RA][i];
            cout<<cpu->R[i];
        }
        cout<<"\" to R"<<endl<<endl;
    }
    else if(cpu->IR[0]=='S' && cpu->IR[1]=='R')
    {
        cout<<"            Storing \"";
        for(int i=0;i<4;i++)
        {
            M[RA][i] = cpu->R[i];
            cout<<M[RA][i];
        }
        cout<<"\" to M"<<endl<<endl;
    }
    else if(cpu->IR[0]=='C' && cpu->IR[1]=='R')
    {
        cout<<"            Comparing \"";
        for(int i=0;i<4;i++)
            cout<<M[RA][i];
        cout<<"\" and \"";
        for(int i=0;i<4;i++)
            cout<<cpu->R[i];
        cout<<"\", Result: ";

        for(int i=0;i<4;i++)
        {
            if(M[RA][i]==cpu->R[i])
                cpu->C=1;
            else
            {
                cpu->C=0;
                break;
            }
        }
        cout<<cpu->C<<endl<<endl;
    }
    else if(cpu->IR[0]=='B' && cpu->IR[1]=='T')
    {
        if(cpu->C==1)
        {
            cpu->IC[0]=cpu->IR[2]-'0';
            cpu->IC[1]=cpu->IR[3]-'0';
            cout<<"            C is 1, Jumping to "<<cpu->IC[0]<<cpu->IC[1]<<"."<<endl<<endl;
        }
        else
        {
            cout<<"            C is 0, No Jumping."<<endl<<endl;
        }
    }
    else if(cpu->IR[0]=='H')
    {
        cout<<"            Halt encountered."<<endl<<endl;
         SI=3;
    }
    else
    {
        PI=1;
    }
}


void simulation()
{
    Timer++;

    cpu->TSC++;
    if (cpu->TSC == cpu->TS)
        TI = 1;

    if(ch1->flag)
    {
        ch1->chTimer++;
        if(ch1->chTimer == ch1->chTotalTime)
        {
            IOI += 1;
            ch1->chTimer = 0;
        }
    }

    if(ch2->flag)
    {
        ch2->chTimer++;
        if(ch2->chTimer == ch2->chTotalTime)
        {
            IOI += 2;
            ch2->chTimer = 0;
        }
    }

    if(ch3->flag)
    {
        ch3->chTimer++;
        if(ch3->chTimer == ch3->chTotalTime)
        {
            IOI += 4;
            ch3->chTimer = 0;
        }
    }

    if(RQ)
    {
        RQ->pcb->TTC += 1;
        if(RQ->pcb->TTC == RQ->pcb->TTL)
            TI = 2;
    }
}


void MOS()
{
    if(TI==1 && SI==0 && PI==0)
    {
        RQ_to_RQ();
    }
    if(TI==0 || TI==1)
    {
        if(SI==1)
        {
            RQ->pcb->IO = 1;
            RQ_to_IOQ();
        }
        else if(SI==2)
        {
            RQ->pcb->IO = 2;
            RQ_to_IOQ();
        }
        else if(SI==3)
        {
            RQ->pcb->TC.push_back(0);
            RQ_to_TQ();
        }

        if(PI==1)
        {
            cout<<"            Opcode Error"<<endl;
            RQ->pcb->abTerminate = 1;
            RQ->pcb->TC.push_back(4);
            RQ_to_TQ();
        }
        else if(PI==2)
        {
            cout<<"            Operand Error"<<endl;
            RQ->pcb->abTerminate = 1;
            RQ->pcb->TC.push_back(5);
            RQ_to_TQ();
        }
        else if(PI==3)
        {
            if(page_fault)
            {
                cout<<"            Valid page fault."<<endl;
                if(n_available_frames)
                {
                    int frame_no = allocate();
                    cout<<"            Frame "<<frame_no<<" allocated."<<endl<<endl;

                    int PTE=(cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3]) + VA[0];

                    for (int j=3; j>=0; j--)
                    {
                        M[PTE][j] = frame_no%10+48;
                        frame_no /= 10;
                    }

                    cpu->IC[1] -= 1;
                    if(cpu->IC[1]==-1)
                    {
                        cpu->IC[1]=9;
                        cpu->IC[0]-=1;
                    }
                    if(TI)
                        RQ_to_RQ();
                }
                else
                {
                    RQ_to_SQ();
                }
                page_fault = 0;
                PI = 0;
            }
            else
            {
                cout<<"            Invalid Page fault"<<endl;
                RQ->pcb->abTerminate = 1;
                RQ->pcb->TC.push_back(6);
                RQ_to_TQ();
            }
        }
    }
    else if(TI==2)
    {
        cout<<"            Time limit exceeded."<<endl;

        RQ->pcb->abTerminate = 1;
        if(SI==1)
        {
            RQ->pcb->TC.push_back(3);
            RQ_to_TQ();
        }
        else if(SI==2)
        {
            RQ->pcb->IO = 2;
            RQ->pcb->TC.push_back(3);
            RQ_to_IOQ();
        }
        else if(SI==3)
        {
            RQ->pcb->TC.push_back(0);
            RQ_to_TQ();
        }

        else if(PI==1)
        {
            cout<<"            Opcode Error"<<endl;
            RQ->pcb->TC.push_back(3);
            RQ->pcb->TC.push_back(4);
            RQ_to_TQ();
        }
        else if(PI==2)
        {
            cout<<"            Operand Error"<<endl;
            RQ->pcb->TC.push_back(3);
            RQ->pcb->TC.push_back(5);
            RQ_to_TQ();
        }
        else if(PI==3)
        {
            cout<<"            Page fault."<<endl;
            RQ->pcb->TC.push_back(3);
            RQ->pcb->TC.push_back(6);
            RQ_to_TQ();
        }
        else
        {
            RQ->pcb->TC.push_back(3);
            RQ_to_TQ();
        }
    }

    switch(IOI)
    {
        case(0): break;
        case(1): IR(1); break;
        case(2): IR(2); break;
        case(3): IR(2); IR(1); break;
        case(4): IR(3); break;
        case(5): IR(1); IR(3); break;
        case(6): IR(3), IR(2); break;
        case(7): IR(2); IR(1); IR(3); break;
    }

    SI = 0;
    TI = 0;
    PI = 0;
}


void IR(int i)
{
    cout<<"    IOI: "<<IOI<<endl;
    if (i==1)
    {
        cout<<"        IR1:-"<<endl;
        if(S->ebq)
        {
            cout<<"            Empty buffer available for card reading."<<endl;
            string line;
            getline(input,line);
            cout<<"            '"<<line<<"' read from file \n";

            for(int i=0; i<line.size() && i<40; i++)
                S->ebq->buff[i] = line[i];

            S->ebq_to_ifbq();

            S->temp = S->ifbq;
            while(S->temp->next)
                S->temp = S->temp->next;

            if(S->temp->buff[0]=='$' && S->temp->buff[1]=='A' && S->temp->buff[2]=='M' && S->temp->buff[3]=='J')
            {
                pcb = new PCB;
                pcb->JOB_ID.push_back(S->temp->buff[4]);
                pcb->JOB_ID.push_back(S->temp->buff[5]);
                pcb->JOB_ID.push_back(S->temp->buff[6]);
                pcb->JOB_ID.push_back(S->temp->buff[7]);

                pcb->TTL = (S->temp->buff[8]-'0')*1000 + (S->temp->buff[9]-'0')*100 + (S->temp->buff[10]-'0')*10 + (S->temp->buff[11]-'0');
                pcb->TLL = (S->temp->buff[12]-'0')*1000 + (S->temp->buff[13]-'0')*100 + (S->temp->buff[14]-'0')*10 + (S->temp->buff[15]-'0');
                pcb->TTC = 0;
                pcb->LLC = 0;
                pcb->flag = 'P';

                int ptr_no = allocate()*10;

                cout<<"            Frame no. "<<ptr_no<<" allocated for page table of "<<pcb->JOB_ID<<endl;

                pcb->PTR = ptr_no;

                // Initializing Page table
                for(int i=ptr_no;i<ptr_no+10;i++)
                {
                    for(int j=0;j<4;j++)
                    {
                        M[i][j]='*';
                    }
                }

                /*
                // Initializing PTR
                for (int j=3; j>=0; j--)
                {
                    cpu->PTR[j] = ptr_no%10;
                    ptr_no /= 10;
                }
                */

                //Add PCB to PQ
                if(PQ)
                {
                    struct ProgramQueue *temp = PQ;
                    while(temp->next)
                        temp = temp->next;
                    temp->next = new ProgramQueue;
                    temp->next->pcb = pcb;
                }
                else
                {
                    PQ = new ProgramQueue;
                    PQ->pcb = pcb;
                }
                cout<<"            PCB added to PQ"<<endl;

                S->last_ifb_to_ebq();
            }
            else if(S->temp->buff[0]=='$' && S->temp->buff[1]=='D' && S->temp->buff[2]=='T' && S->temp->buff[3]=='A')
            {
                cout<<"            for job "<<PQ->pcb->JOB_ID<<endl;
                PQ->pcb->flag = 'D';
                S->last_ifb_to_ebq();
            }
            else if(S->temp->buff[0]=='$' && S->temp->buff[1]=='E' && S->temp->buff[2]=='N' && S->temp->buff[3]=='D')
            {
                PQ->pcb->read = 1;

                if(PQ->pcb->n_DataCard_copy!=0)
                    cout<<"            Input spooling not complete for job "<<PQ->pcb->JOB_ID<<". Retaining PCB in PQ. "<<endl;
                else
                    PQ_to_LQ();

                S->last_ifb_to_ebq();
            }
            else
            {
                if(pcb->flag == 'P')
                {
                    cout<<"            Program card encountered for "<<pcb->JOB_ID<<endl;
                    pcb->n_ProgramCard++;
                    pcb->n_ProgramCard_copy++;
                }
                else if (pcb->flag == 'D')
                {
                    cout<<"            Data card encountered for "<<pcb->JOB_ID<<endl;
                    pcb->n_DataCard++;
                    pcb->n_DataCard_copy++;
                }
                if(ch3->flag==0)
                {
                    IOI+=4;
                    ch3->startChannel();
                    ch3->Task = "IS";
                }
            }
        }
        else
        {
            cout<<"            Empty buffer not available for card reading."<<endl;
        }

        if(input.eof())
        {
            ch1->startChannel();    // To decrement IOI by 1
            ch1->stopChannel();
        }
        else if(!S->ebq)
            ch1->stopChannel();     // Temporarily stop CH1 till empty buffer is avlbl
        else
            ch1->startChannel();
    }
    else if(i==2)
    {
        if(S->ofbq)
        {
            cout<<"        IR2:-"<<endl;
            cout<<"            Line printer prints EMPTY LINE ";
            if(S->ofbq->buff[0] != '\n')
            {
                cout<<"\b\b\b\b\b\b\b\b\b\b\b\"" ;
                for(char x : S->ofbq->buff)
                {
                    output<<x;
                    cout<<x;
                }
                cout<<"\"";
            }

            S->ofbq_to_ebq();
            output<<endl;
            cout<<endl<<endl;
            ch2->startChannel();
        }
        else
        {
            ch2->startChannel();
            ch2->stopChannel();
        }
    }
    else if(i==3)
    {
        // Complete the assigned task
        cout<<"        IR3:-"<<endl;
        cout<<"            Current task: "<<ch3->Task<<endl;

        if(ch3->Task == "IS")
        {
            char spool;
            int drum_word = get_drum_track()*10;

            if(PQ->pcb->n_ProgramCard_copy !=0)
            {
                //Program card is to be spooled
                pcb->T_ProgramCard.push_back(drum_word/10);

                    spool = 'P';
                cout<<"                Program card \"";
            }
            else if(PQ->pcb->n_DataCard_copy !=0)
            {
                //Data card is to be spooled
                PQ->pcb->T_DataCard.push_back(drum_word/10);

                spool = 'D';

                cout<<"                Data card \"";
            }

            for(int i=drum_word, k=0; k<40 && i<drum_word+10; i++)
            {
                for(int j=0;j<4;j++)
                {
                    drum[i][j] = S->ifbq->buff[k++];
                    cout<<drum[i][j];

                    if(spool == 'P' && drum[i][j]=='H')
                        break;
                }
            }

            if(spool == 'P')
                PQ->pcb->n_ProgramCard_copy--;
            else
                PQ->pcb->n_DataCard_copy--;

            cout<<"\" spooled for job "<<PQ->pcb->JOB_ID<<endl;

            S->first_ifb_to_ebq();
            n_available_tracks--;

            if(PQ->pcb->n_DataCard_copy == 0 && PQ->pcb->read)
            {
                cout<<"    ";
                PQ_to_LQ();
            }
        }
        else if (ch3->Task == "OS")
        {
            if(TQ->pcb->T_OutputLines.empty())
            {
                if(S->ebq && S->ebq->next && S->ebq->next->next && S->ebq->next->next->next)
                {
                    cout<<"                Output spooling complete for job "<<TQ->pcb->JOB_ID<<", deleting PCB and memory frames. "<<endl;
                    Terminate();
                    S->ebq->buff[0] = '\n';
                    S->ebq_to_ofbq();
                    S->ebq->buff[0] = '\n';
                    S->ebq_to_ofbq();

                    //Clear memory blocks:
                    for(int i=TQ->pcb->PTR; i<TQ->pcb->PTR+10; i++)
                    {
                        if(M[i][3]!='*')
                        {
                            int j = ((M[i][2]-'0')*10 + (M[i][3]-'0'))*10;
                            for(int k=j; k<j+10; k++)
                            {
                                for(int l=0; l<4;l++)
                                    M[k][l] = '-';
                            }

                            n_available_frames++;
                        }
                        M[i][0] = '-';
                        M[i][1] = '-';
                        M[i][2] = '-';
                        M[i][3] = '-';

                        n_available_frames++;
                    }

                    //Release PCB;
                    ProgramQueue *temp = TQ;
                    TQ = TQ->next;
                    delete temp;

                    if(!ch2->flag)
                    {
                        IOI += 2;
                        ch2->startChannel();
                    }
                }
            }
            else
            {
                int drum_word = TQ->pcb->T_OutputLines.front() * 10;

                cout<<"                Output spooling \"";
                for(int i=0,j=0; i<40; i++,j++)
                {
                    if(j==4)
                    {
                        j=0;
                        drum_word++;
                    }

                    S->ebq->buff[i] = drum[drum_word][j];
                    cout<<drum[drum_word][j];

                    drum[drum_word][j] = '-';
                }

                cout<<"\" for job "<<TQ->pcb->JOB_ID<<endl;

                S->ebq_to_ofbq();
                TQ->pcb->T_OutputLines.erase(TQ->pcb->T_OutputLines.begin());
                n_available_tracks++;
            }
        }
        else if (ch3->Task == "LD")
        {
            int pt_last_row;
            for(pt_last_row=LQ->pcb->PTR; M[pt_last_row][3]!='*'; pt_last_row++);
            pt_last_row--;

            int frame_no = (M[pt_last_row][2]-'0')*10 + (M[pt_last_row][3]-'0');
            int drum_word = LQ->pcb->T_ProgramCard.front() * 10;

            cout<<"                Program card \"";
            for(int i=frame_no*10 ; i<frame_no*10+10; i++, drum_word++)
                for(int k=0; k<4; k++)
                {
                    M[i][k] = drum[drum_word][k];
                    drum[drum_word][k] = '-';
                    cout<<M[i][k];
                }
            cout<<"\" loaded for job "<<LQ->pcb->JOB_ID<<endl;

            LQ->pcb->n_ProgramCard--;
            n_available_tracks++;
            LQ->pcb->T_ProgramCard.erase(LQ->pcb->T_ProgramCard.begin());

            if (LQ->pcb->n_ProgramCard == 0)
            {
                LQ_to_RQ();
                print_memory();
            }
        }
        else if (ch3->Task == "RD")
        {
            int drum_word = IOQ->pcb->T_DataCard.front() * 10;
            IOQ->pcb->T_DataCard.erase(IOQ->pcb->T_DataCard.begin());

            //Get the block no for GD
            int VA = IOQ->pcb->IC-1;    //Not using addressMap() since PCB is in IOQ, so cpu->PTR != IOQ->pcb->PTR
            int PTE = IOQ->pcb->PTR + VA/10;
            RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

            int block = (M[RA][2]-'0')*10 + (M[RA][3]-'0');

            VA = block;
            PTE = IOQ->pcb->PTR + VA/10;
            RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

            cout<<"                Data card \"";
            for(int i=RA ; i<RA+10; i++, drum_word++)
                for(int k=0; k<4; k++)
                {
                    M[i][k] = drum[drum_word][k];
                    cout<<M[i][k];
                    drum[drum_word][k] = '-';
                }
            cout<<"\" read for job "<<IOQ->pcb->JOB_ID<<endl;

            n_available_tracks++;
            IOQ_to_RQ();
        }
        else if (ch3->Task == "WT")
        {
            IOQ->pcb->LLC++;
            if(IOQ->pcb->LLC > IOQ->pcb->TLL)
            {
                IOQ->pcb->TC.push_back(2);
                cout<<"                Line limit exceeded.";

                if(IOQ->pcb->abTerminate == 1)
                    cout<<" Time limit exceeded.";

                IOQ->pcb->abTerminate == 1;
                IOQ_to_TQ();
            }
            else
            {
                int drum_word = get_drum_track()*10;
                IOQ->pcb->T_OutputLines.push_back(drum_word/10);

                //Get the block no for PD
                int VA = IOQ->pcb->IC-1;    //Not using addressMap() since PCB is in IOQ, so cpu->PTR != IOQ->pcb->PTR
                int PTE = IOQ->pcb->PTR + VA/10;
                RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

                int block = (M[RA][2]-'0')*10 + (M[RA][3]-'0');

                VA = block;
                PTE = IOQ->pcb->PTR + VA/10;
                RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

                cout<<"                Data card \"";
                for(int i=RA ; i<RA+10; i++, drum_word++)
                    for(int k=0; k<4; k++)
                    {
                        drum[drum_word][k] = M[i][k];
                        cout<<drum[drum_word][k];
                    }
                cout<<"\" printed to drum for job "<<IOQ->pcb->JOB_ID<<endl;

                n_available_tracks--;

                if(IOQ->pcb->abTerminate == 1)
                {
                    cout<<"                IO request fulfilled, but time limit exceeded.";
                    IOQ_to_TQ();
                }
                else
                    IOQ_to_RQ();
            }
        }
        else if (ch3->Task == "SQ")
        {
            if(n_available_frames)
            {
                int frame_no = allocate();
                cout<<"                Frame "<<frame_no<<" allocated for job "<<SQ->pcb->JOB_ID<<endl<<endl;

                SQ->pcb->IC -= 1;

                int x = SQ->pcb->PTR + SQ->pcb->IC/10;
                int program_frame = (M[x][2]-'0')*10 + (M[x][3]-'0');   //frame where the current program card is stored
                int row  = program_frame*10 + SQ->pcb->IC%10;           //Memory row where the current instruction is present
                int page = (M[row][2]-'0')*10;        //Virtual address, e.g. 80 for GD88

                int PTE = SQ->pcb->PTR + page/10;

                for (int j=3; j>=0; j--)
                {
                    M[PTE][j] = frame_no%10+48;
                    frame_no /= 10;
                }

                SQ_to_RQ();
            }
            else
                cout<<"                Memory frame not available for job "<<SQ->pcb->JOB_ID<<endl;

        }

        // Assign new task in priority order
        cout<<endl<<"            Next task:-";
        if(TQ && S->ebq)
        {
            ch3->Task="OS";
            cout<<" OS"<<endl;
        }
        else if (S->ifbq && n_available_tracks)
        {
            ch3->Task="IS";
            cout<<" IS"<<endl;
        }
        else if (LQ && n_available_frames)
        {
            int frame_no = allocate();

            int k=LQ->pcb->PTR;

            for(; M[k][3]!='*'; k++) ;

            for (int j=3; j>=0; j--)
            {
                M[k][j] = frame_no%10+48;
                frame_no /= 10;
            }

            ch3->Task = "LD";
            cout<<" LD"<<endl;
        }
        else if(IOQ && ((IOQ->pcb->IO == 2)?n_available_tracks:1))
        {
            if(IOQ->pcb->IO == 1)
            {
                if(IOQ->pcb->T_DataCard.empty())
                {
                    IOQ->pcb->TC.push_back(1);
                    cout<<" RD, Out of Data.";
                    IOQ_to_TQ();
                    ch3->Task = "";
                }
                else
                {
                    ch3->Task = "RD";
                    cout<<" RD"<<endl;
                }
            }
            else if(IOQ->pcb->IO == 2)
            {
                ch3->Task = "WT";
                cout<<" WT"<<endl;
            }
        }
        else if(SQ)
        {
            ch3->Task="SQ";
            cout<<" SQ"<<endl;
        }
        else
        {
            ch3->Task="";
            cout<<endl;
        }
        ch3->startChannel();    // To decrement IOI by 4
    }
}


void Terminate()
{
    string EM;
    for(int i : TQ->pcb->TC)
    {
        if (i==0)
            EM = "Normal Termination";
        else
        {
            EM = "Abnormal Termination: ";
            switch(i)
            {
                case(1): EM += "Out Of Data"; break;
                case(2): EM += "Line Lim. Exceeded"; break;
                case(3): EM += "Time Lim. Exceeded"; break;
                case(4): EM += "OpCode Error"; break;
                case(5): EM += "Operand Error"; break;
                case(6): EM += "Invalid Page Fault"; break;
            }
        }
        for(int i=0; i<EM.size(); i++)
            S->ebq->buff[i] = EM[i];

        S->ebq_to_ofbq();
    }

    string status1;
    status1 += "IC:";
    status1 += to_string(TQ->pcb->IC);
    status1 += ", TTL:";
    status1 += to_string(TQ->pcb->TTL);
    status1 += ", TTC:";
    status1 += to_string(TQ->pcb->TTC);
    status1 += ", TLL:";
    status1 += to_string(TQ->pcb->TLL);
    status1 += ", LLC:";
    status1 += to_string(TQ->pcb->LLC);

    for(int i=0; i<status1.size(); i++)
            S->ebq->buff[i] = status1[i];

    S->ebq_to_ofbq();
}


main()
{
    input.open("input.txt",ios::in);
    output.open("output.txt",ios::out);
    srand(time(0));

    init();
    ch1->startChannel();

    do
    {
        cout<<"------------------------------------------"<<Timer<<"------------------------------------------"<<endl;

        executeUserProgram();
        simulation();
        MOS();
        cout<<"    IOI: "<<IOI<<endl<<endl;

    }while(ch1->flag!=0 || S->ifbq || S->ofbq || LQ || RQ || IOQ || TQ || SQ);

    ch2->stopChannel();
    ch3->stopChannel();

    input.close();
    output.close();
}
