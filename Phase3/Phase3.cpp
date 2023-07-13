#include<bits/stdc++.h>

using namespace std;

void init();
void simulation();
int allocate();
void MOS();
void IR(int);
void executeUserProgram();
void addressMap();
void Terminate(int EC1, int EC2=0);

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
    int PTR, IC=0;
    int IO = 0; //0: NO I/O, 1: Read, 2: Write

    string JOB_ID;
    int TTL,TLL,TTC,LLC;

    vector <int> T_ProgramCard, T_DataCard, T_OutputLines;
    int n_ProgramCard=0, n_DataCard=0, n_OutputLines=0;
    char flag;      // P:Program card or D: Data Card

    int n_ProgramCard_copy=0, n_DataCard_copy=0, n_OutputLines_copy=0;

    vector <int> TC; //Termination code

} *pcb;

struct ProgramQueue
{
    PCB *pcb = new PCB;
    ProgramQueue* next=NULL;

    ~ProgramQueue()
    {
        delete pcb;
    }
} *LQ = NULL, *RQ = NULL, *IOQ = NULL, *TQ = NULL, *SQ = NULL;


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
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"        Time slice out for job "<<RQ->pcb->JOB_ID<<endl;

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
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"        IO request for job "<<RQ->pcb->JOB_ID<<"."<<endl;

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
    cout<<"                IO request for job "<<IOQ->pcb->JOB_ID<<" completed."<<endl;

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
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"        Suspending job "<<RQ->pcb->JOB_ID<<"."<<endl;

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
    cout<<"        Suspend to Ready job "<<SQ->pcb->JOB_ID<<endl;

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
    RQ->pcb->PTR = cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
    RQ->pcb->IC = cpu->IC[0]*10+cpu->IC[1];

    for(auto i=0;i<4;i++)
    {
        cpu->R[i]='-';
        cpu->IR[i]='-';
        cpu->PTR[i]='-';
        if(i<2)
            cpu->IC[i]='-';
    }
    cpu->C=0;

    cout<<"        Terminating job "<<RQ->pcb->JOB_ID<<"."<<endl;

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
    cout<<"        IO request can't be fulfilled. Terminating job "<<IOQ->pcb->JOB_ID<<"."<<endl;

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


int allocate()
{
    label:

    int allNum = rand()%30;

    if(M[allNum*10][0] != '-')
        goto label;

    n_available_frames--;
    return allNum;
}


void MOS()
{
    if(TI==1 && SI==0 && PI==0)
    {
        RQ_to_RQ();
    }

    if((TI==0 || TI==1) && SI==1)
    {
        RQ->pcb->IO = 1;
        RQ_to_IOQ();
    }
    else if((TI==0 || TI==1) && SI==2)
    {
        RQ->pcb->IO = 2;
        RQ_to_IOQ();
    }
    else if((TI==0 || TI==1) && SI==3)
    {
        RQ->pcb->TC.push_back(0);
        RQ_to_TQ();
    }
    else if(TI==2 && SI==1)
    {
        RQ->pcb->TC.push_back(3);
    }
    else if(TI==2 && SI==2)
    {
        RQ->pcb->IO = 2;
        RQ_to_IOQ();
    }
    else if(TI==2 && SI==3)
    {
        RQ->pcb->TC.push_back(0);
        RQ_to_TQ();
    }

    if((TI==0 || TI==1) && PI==1);
    else if((TI==0 || TI==1) && PI==2);
    else if((TI==0 || TI==1) && PI==3)
    {
        if(page_fault)
        {
            cout<<"        Valid page fault."<<endl;
            if(n_available_frames)
            {
                int frame_no = allocate();
                cout<<"        Frame "<<frame_no<<" allocated."<<endl;

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
                S->last_ifb_to_ebq();
            }
            else if(S->temp->buff[0]=='$' && S->temp->buff[1]=='D' && S->temp->buff[2]=='T' && S->temp->buff[3]=='A')
            {
                cout<<"            for job "<<pcb->JOB_ID<<endl;
                pcb->flag = 'D';
                S->last_ifb_to_ebq();
            }
            else if(S->temp->buff[0]=='$' && S->temp->buff[1]=='E' && S->temp->buff[2]=='N' && S->temp->buff[3]=='D')
            {
                if(LQ)
                {
                    struct ProgramQueue *temp = LQ;
                    while(temp->next)
                        temp = temp->next;
                    temp->next = new ProgramQueue;
                    temp->next->pcb = pcb;
                }
                else
                {
                    LQ = new ProgramQueue;
                    LQ->pcb = pcb;
                }

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
        IOI -= 2;
    }
    else if(i==3)
    {
        // Complete the assigned task
        cout<<"        IR3:-"<<endl;
        cout<<"            Current task: "<<ch3->Task<<endl;

        if(ch3->Task == "IS")
        {
            char spool;
            int track = -1;
            for(int i=0; i<50; i++)
            {
                if(drum[i*10][0] == '-')
                {
                    track = i*10;
                    break;
                }
            }

            if(track == -1)
            {
                cout<<"               Drum is full."<<endl;
            }
            else
            {
                if(pcb->n_ProgramCard_copy !=0)
                {
                    //Program card is to be spooled
                    pcb->T_ProgramCard.push_back(track);

                    spool = 'P';
                    cout<<"                Program card ";
                }
                else if(pcb->n_DataCard_copy !=0)
                {
                    //Data card is to be spooled
                    pcb->T_DataCard.push_back(track);

                    spool = 'D';

                    cout<<"                Data card  ";
                }

                for(int i=track, k=0; k<40 && i<track+10; i++)
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
                    pcb->n_ProgramCard_copy--;
                else
                    pcb->n_DataCard_copy--;

                cout<<" spooled for job "<<pcb->JOB_ID<<endl;

                S->first_ifb_to_ebq();
                n_available_tracks--;

                if(pcb->n_DataCard_copy == 0)
                    cout<<"                Input spooling is complete for "<<pcb->JOB_ID<<"."<<endl;
                /*for(int i=0; i<500;i++)
                {
                    for(int j=0; j<4;j++)
                        cout<<drum[i][j];
                    cout<<endl;
                }
                cout<<endl;
                */
            }
        }
        else if (ch3->Task == "OS"){}
        else if (ch3->Task == "LD")
        {
            int pt_last_row;
            for(pt_last_row=LQ->pcb->PTR; M[pt_last_row][3]!='*'; pt_last_row++);
            pt_last_row--;

            int frame_no = (M[pt_last_row][2]-'0')*10 + (M[pt_last_row][3]-'0');
            int track = LQ->pcb->T_ProgramCard.front();

            cout<<"                Program card ";
            for(int i=frame_no*10 ; i<frame_no*10+10; i++, track++)
                for(int k=0; k<4; k++)
                {
                    M[i][k] = drum[track][k];
                    drum[track][k] = '-';
                    cout<<M[i][k];
                }
            cout<<" loaded for job "<<LQ->pcb->JOB_ID<<endl;

            LQ->pcb->n_ProgramCard--;
            n_available_tracks++;
            LQ->pcb->T_ProgramCard.erase(LQ->pcb->T_ProgramCard.begin());

            if (LQ->pcb->n_ProgramCard == 0)
            {
                LQ_to_RQ();

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
        }
        else if (ch3->Task == "GD")
        {
            int track = IOQ->pcb->T_DataCard.front();
            IOQ->pcb->T_DataCard.erase(IOQ->pcb->T_DataCard.begin());

            //Get the block no for GD
            int VA = IOQ->pcb->IC-1;
            int PTE = IOQ->pcb->PTR + VA/10;
            RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

            int block = (M[RA][2]-'0')*10 + (M[RA][3]-'0');

            VA = block;
            PTE = IOQ->pcb->PTR + VA/10;
            RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

            cout<<"                Data card ";
            for(int i=RA ; i<RA+10; i++, track++)
                for(int k=0; k<4; k++)
                {
                    M[i][k] = drum[track][k];
                    cout<<M[i][k];
                    drum[track][k] = '-';
                }
            cout<<" read for job "<<IOQ->pcb->JOB_ID<<endl;

            n_available_tracks++;
            IOQ_to_RQ();
        }
        else if (ch3->Task == "PD")
        {
            int track = -1;
            for(int i=0; i<50; i++)
            {
                if(drum[i*10][0] == '-')
                {
                    track = i*10;
                    break;
                }
            }
            IOQ->pcb->T_OutputLines.push_back(track);

            //Get the block no for PD
            int VA = IOQ->pcb->IC-1;
            int PTE = IOQ->pcb->PTR + VA/10;
            RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

            int block = (M[RA][2]-'0')*10 + (M[RA][3]-'0');

            VA = block;
            PTE = IOQ->pcb->PTR + VA/10;
            RA = ((M[PTE][0]-'0')*1000 + (M[PTE][1]-'0')*100 + (M[PTE][2]-'0')*10 + (M[PTE][3]-'0')) *10 +( VA%10);

            cout<<"                Data card ";
            for(int i=RA ; i<RA+10; i++, track++)
                for(int k=0; k<4; k++)
                {
                    drum[track][k] = M[i][k];
                    cout<<M[i][k];
                }
            cout<<" print to drum for job "<<IOQ->pcb->JOB_ID<<endl;

            n_available_tracks--;
            IOQ->pcb->LLC++;

            if(TI == 2)
                IOQ_to_TQ();
            else
                IOQ_to_RQ();
        }


        // Assign new task in priority order
        cout<<endl<<"            Next task:-";
        if(TQ && S->ebq){ch3->startChannel();}
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
                    IOQ->pcb->TC.push_back(3);
                    IOQ_to_TQ();
                }
                else
                {
                    ch3->Task = "GD";
                    cout<<" GD"<<endl;
                }
            }
            else if(IOQ->pcb->IO == 2)
            {
                if(IOQ->pcb->LLC > IOQ->pcb->TLL)
                {
                    IOQ->pcb->TC.push_back(2);
                    IOQ_to_TQ();
                }
                else
                {
                    ch3->Task = "PD";
                    cout<<" PD"<<endl;
                }
            }
        }
        else
        {
            ch3->Task="";
            cout<<endl;
        }
        ch3->startChannel();    // To decrement IOI by 4
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
        cpu->IC[1] = RQ->pcb->IC%10;
        cpu->IC[0] = RQ->pcb->IC/10;

        cpu->PTR[3] = RQ->pcb->PTR%10;
        cpu->PTR[2] = RQ->pcb->PTR/10%10;
        cpu->PTR[1] = RQ->pcb->PTR/100%10;
        cpu->PTR[0] = 0;

        cpu->TSC = 1;
    }

    VA[0] = cpu->IC[0];
    VA[1] = cpu->IC[1];
    addressMap();

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

    cout<<"    CPU:"<<endl;
    cout<<"        Job: "<<RQ->pcb->JOB_ID<<endl;
    cout<<"        IC: "<<cpu->IC[0]<<cpu->IC[1]<<endl;
    cout<<"        IR: "<<cpu->IR[0]<<cpu->IR[1]<<cpu->IR[2]<<cpu->IR[3]<<endl;
    cout<<"        TSC: "<<cpu->TSC<<endl;

    VA[0] = cpu->IR[2]-'0';
    VA[1] = cpu->IR[3]-'0';
    addressMap();

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
        SI=2;
    }
    cout<<endl;
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


void Terminate(int EC1, int EC2)
{

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

    }while(/*ch1->flag!=0 || S->ifbq || S->ofbq || LQ || RQ || IOQ || TQ || SQ */ Timer<50);

    input.close();
    output.close();


}
