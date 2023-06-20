/****************************************************
                GROUP TY29
Class: CS-B
.-----------------------------------------------.
|     PRN      | Roll no |         Name         |
|-----------------------------------------------|
|   12010594   |    26   |   Omkar Kalange      |
|   12011412   |    27   |   Atharv Kale        |
|   12010560   |    38   |   Tejaswini Katale   |
|   12010253   |    41   |   Ketan Aggarwal     |
`-----------------------------------------------'

*****************************************************/

#include<bits/stdc++.h>

using namespace std;

void init();
void simulation();
int allocate();
void MOS();
void IR(int);
void startExecution();
void executeUserProgram();
void addressMap();

fstream input;
fstream output;
char M[300][4];
int n_available_frames=30;
char drum[500][4];
int Track_no = 0;
int SI=3;   // service interrupt
int TI=0;   //time interrupt
int PI=0;   //programming interrupt
int IOI=0; // Input-Output interrupt
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
    int PTR;

    string JOB_ID;
    int TTL,TLL,TTC,LLC;

    int T_ProgramCard=-10, T_DataCard=-10, T_OutputLines=-10;
    int n_ProgramCard=0, n_DataCard=0, n_OutputLines=0;
    char flag;      // P:Program card or D: Data Card

    int n_ProgramCard_copy=0, n_DataCard_copy=0, n_OutputLines_copy=0;

} *pcb;

struct ProgramQueue
{
    PCB *pcb;
    ProgramQueue* next=NULL;
} *LQ=NULL, *RQ=NULL, *IOQ=NULL, *TQ=NULL;

void LQ_to_RQ()
{
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


class SupervisoryStorage
{
    SupervisoryStorage();
    static SupervisoryStorage *object;
    struct node
    {
        char buff[40];
        struct node* next;
    };

    public:
        node* temp;
        struct node *ebq;
        struct node *ifbq = NULL;
        struct node *ofbq = NULL;

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
        for (int j=0; j<40; j++)
            temp->buff[j] = '-';

        temp->buff;
        temp->next = new node;

        temp = temp->next;
    }
        for (int j=0; j<40; j++)
            temp->buff[j] = '-';

        temp->buff;
        temp->next = NULL;
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
            temp->next->next = ebq;
            ebq = temp->next;

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
            temp->next = ebq;
            ebq = temp;

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
        temp = ifbq;
        ifbq = ifbq->next;
        temp->next = ebq;
        ebq = temp;
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

class Channel
{
    protected:
        Channel(){}
    private:
        static Channel* object1;
        static Channel* object2;


    public:
        int flag = 0;
        int chTotalTime = 5;
        int chTimer;

        static Channel* getObject1()
        {
           return Channel::object1;
        }
        static Channel* getObject2()
        {
           return Channel::object2;
        }
        void startChannel()
        {
            if(!flag)
            {
                chTimer = 0;
            }
            flag = 1;
        }
        void stopChannel()
        {
            flag = 0;
        }
};
Channel* Channel :: object1 = new Channel;
Channel* Channel :: object2 = new Channel;
Channel* ch1 = Channel::getObject1();
Channel* ch2 = Channel::getObject2();

class CH3: public Channel
{
    public:
        int chTotalTime = 2;
        string Task="";
        static CH3* getObject3()
        {
           return CH3::object3;
        }
    private:
        static CH3* object3;
        CH3(){};
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

    SI=3;
    TI=0;
    PI=0;

    Timer=0;
}

void simulation()
{
    Timer++;

    cpu->TSC++;
    if (cpu->TSC ==cpu->TS)
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
    switch(IOI)
    {
        case(1): IR(1); break;
        case(2): IR(2); break;
        case(3): IR(2); IR(1); break;
        case(4): IR(3); break;
        case(5): IR(1); IR(3); break;
        case(6): IR(3), IR(2); break;
        case(7): IR(2); IR(1); IR(3); break;
    }

}


void IR(int i)
{
    cout<<"     IOI: "<<IOI<<endl;
    if (i==1)
    {
        string line;
        getline(input,line);
        cout<<"     '"<<line<<"' read from file \n";

        for(int i=0; i<line.size() && i<40; i++)
            S->ebq->buff[i] = line[i];

        S->ebq_to_ifbq();

        S->temp = S->ifbq;
        while(S->temp->next)
            S->temp = S->temp->next;

        if(S->temp->buff[0]=='$' && S->temp->buff[1]=='A' && S->temp->buff[2]=='M' && S->temp->buff[3]=='J')
        {
            cout<<"     $AMJ encountered ";

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

            cout<<"for "<<pcb->JOB_ID<<endl<<"      Frame no. "<<ptr_no<<" allocated for page table of "<<pcb->JOB_ID<<endl;

            pcb->PTR = ptr_no;
            // Initializing Page table
            for(int i=ptr_no;i<ptr_no+10;i++)
            {
                for(int j=0;j<4;j++)
                {
                    M[i][j]='*';
                }
            }

            // Initializing PTR
            for (int j=3; j>=0; j--)
            {
                cpu->PTR[j] = ptr_no%10;
                ptr_no /= 10;
            }

            S->last_ifb_to_ebq();
        }
        else if(S->temp->buff[0]=='$' && S->temp->buff[1]=='D' && S->temp->buff[2]=='T' && S->temp->buff[3]=='A')
        {
            cout<<"     $DTA encountered for "<<pcb->JOB_ID<<endl;
            pcb->flag = 'D';
            S->last_ifb_to_ebq();
        }
        else if(S->temp->buff[0]=='$' && S->temp->buff[1]=='E' && S->temp->buff[2]=='N' && S->temp->buff[3]=='D')
        {
            cout<<"     $END encountered for "<<pcb->JOB_ID<<endl;
            if(ch3->Task == "")
                ch3->Task = "IS";

            S->last_ifb_to_ebq();

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
        }
        else
        {
            if(pcb->flag == 'P')
            {
                cout<<"     Program card encountered for "<<pcb->JOB_ID<<endl;
                pcb->n_ProgramCard++;
                pcb->n_ProgramCard_copy++;
            }
            else if (pcb->flag == 'D')
            {
                cout<<"     Data card encountered for "<<pcb->JOB_ID<<endl;
                pcb->n_DataCard++;
                pcb->n_DataCard_copy++;
            }
        }

        IOI -= 1;
    }
    else if(i==2)
    {
        IOI -= 2;
    }
    else if(i==3)
    {
        if(ch3->Task == "IS")
        {
            if(LQ->pcb->n_ProgramCard_copy !=0)
            { //Program card is to be spooled
                if (LQ->pcb->T_ProgramCard == -10)
                    LQ->pcb->T_ProgramCard = Track_no;

                for(int i=Track_no, k=0; k<40 && i<Track_no+10; i++)
                {
                    for(int j=0;j<4;j++)
                    {
                        drum[i][j] = S->ifbq->buff[k++];

                    /*    if(drum[i][j]=='H')
                        break;
                   */ }
                }
                pcb->n_ProgramCard_copy--;

                cout<<"     Program card spooled for job "<<LQ->pcb->JOB_ID<<endl;

                S->first_ifb_to_ebq();

                Track_no  += 10;

                if (Track_no == 50)
                    Track_no = 0;
            }
            else if(LQ->pcb->n_DataCard_copy !=0)
            { //Data card is to be spooled

                if (LQ->pcb->T_DataCard == -10)
                    LQ->pcb->T_DataCard = Track_no;

                for(int i=Track_no, k=0; k<40 && i<Track_no+10; i++)
                {
                    for(int j=0;j<4;j++)
                    {
                        drum[i][j] = S->ifbq->buff[k++];
                    }
                }
                pcb->n_DataCard_copy--;

                cout<<"     Data card spooled for job "<<LQ->pcb->JOB_ID<<endl;

                S->first_ifb_to_ebq();

                Track_no  += 10;

                if (Track_no == 50)
                    Track_no = 0;

                if(drum[Track_no][0]!='-') cout<<"      Drum if full now."<<endl;
            }
        }
        else if (ch3->Task == "OS"){}
        else if (ch3->Task == "LD")
        {
            int pt_last_row;
            for(pt_last_row=LQ->pcb->PTR; M[pt_last_row][3]!='*'; pt_last_row++);
            pt_last_row--;

            int frame_no = (M[pt_last_row][2]-'0')*10 + (M[pt_last_row][3]-'0');
            int track_no = LQ->pcb->T_ProgramCard;
            for(int i=frame_no*10 ; i<frame_no*10+10; i++, track_no++)
                for(int k=0; k<4; k++)
                    M[i][k] = drum[track_no][k];

            LQ->pcb->n_ProgramCard--;

            for(int i=pcb->T_ProgramCard; i<10; i++)
                for (int j=0; j<4; j++)
                    drum[i][j] = '-';

            LQ->pcb->T_ProgramCard+=10;

            cout<<"     Program card loaded for job "<<LQ->pcb->JOB_ID<<endl;

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
        else if (ch3->Task == "RD"){}
        else if (ch3->Task == "WT"){}


        if(TQ && S->ebq)
        {

        }
        else if (S->ifbq && drum[Track_no][0]=='-')
        {
            ch3->Task="IS";
        }
        else if (LQ && n_available_frames)
        {
            int frame_no = allocate();

            int k=LQ->pcb->PTR;

            for(; M[k][3]!='*'; k++) ;//cout<<k<<" "<<M[k][3]<<endl;

            for (int j=3; j>=0; j--)
            {
                M[k][j] = frame_no%10+48;
                frame_no /= 10;
            }


            ch3->Task = "LD";
        }
        IOI -= 4;
    }
}

void startExecution()
{
    cpu->IC[0]=0;
    cpu->IC[1]=0;
    executeUserProgram();
}

void executeUserProgram()
{
    VA[0] = cpu->IC[0];
    VA[1] = cpu->IC[1];
    addressMap();

    if (PI!=0)
        MOS();

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

    cout<<cpu->IR[0]<<cpu->IR[1]<<cpu->IR[2]<<cpu->IR[3]<<endl;

    if(cpu->IR[0]=='G' && cpu->IR[1]=='D')
    {
        if('0'<=cpu->IR[3] && cpu->IR[3]<='9')
            cpu->IR[3] = '0';
        VA[0] = cpu->IR[2]-'0';
        VA[1] = cpu->IR[3]-'0';
        addressMap();

        if(PI == 3)
            page_fault = 1;

        SI=1;

        MOS();
    }

    else if(cpu->IR[0]=='P' && cpu->IR[1]=='D')
    {
        if('0'<=cpu->IR[3] && cpu->IR[3]<='9')
            cpu->IR[3] = '0';
        VA[0] = cpu->IR[2]-'0';
        VA[1] = cpu->IR[3]-'0';
        addressMap();

        SI=2;
        MOS();
    }

    else if(cpu->IR[0]=='H')
    {
        SI=3;

        MOS();
    }
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

main()
{
    input.open("input.txt",ios::in);
    output.open("output.txt",ios::out);
    srand(time(0));

    init();
    int i=1;

    do
    { cout<<"--------------"<<i<<"--------------"<<endl;

        if(!input.eof())
            ch1->startChannel();
        else
            ch1->stopChannel();

        if(LQ)
            ch3->startChannel();
        else
            {ch3->stopChannel(); ch3->Task="";}

     /*   if (RQ)
        {
            if(cpu->IC[0]=='-' && cpu->IC[1]=='-')
                simulation();
            else
                executeUserProgram();
        }
    */

        simulation();
        MOS();
        cout<<"     IOI: "<<IOI<<endl<<endl;
        i++;

    }while(ch1->flag!=0 || ch3->flag!=0);



    input.close();
    output.close();


}
