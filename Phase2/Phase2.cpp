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
void clearbuff();
int allocate();
void loadtoMemory(int word_no);
void LOAD();
void startExecution();
void executeUserProgram();
void addressMap();
void Terminate();
void simulation();
void MOS();


class CPU
{
    public:
        char R[4];
        char IR[4];
        int IC[2];
        int PTR[4];
        bool C;

        static CPU * getObject()
        {
            return object;
        }

    private:
        CPU(){};
        static CPU * object;

};

CPU* CPU::object = new CPU;
CPU *cpu = CPU::getObject();

struct PCB
{
    string JOB_ID;
    int TTL,TLL,TTC,LLC;

    PCB()
    {
        this->JOB_ID="";
        this->TTL=0;
        this->TLL=0;
        this->TTC=0;
        this->LLC=0;
    }
} *pcb;

char M[300][4];
char BUFF[40];
int SI=3;
int TI=0;   //time interrupt
int PI=0;   //programming interrupt
int EM;
int VA[2];
int RA;
int page_fault; // 0:Invalid, 1:Valid
int abTerminate; // 0:Normal Termination, 1:Abnormal Termination

fstream input;
fstream output;


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

    SI=3;
    TI=0;
    PI=0;

    abTerminate=0;
}


void clearbuff()
{
    for(auto i=0;i<40;i++)
    {
        BUFF[i]='-';
    }
}


int allocate()
{

    label:

    int allNum = rand()%29;

    if(cpu->PTR[3]=='-')
    {
        return allNum;
    }
    else if((cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3])==allNum*10)
    {
        goto label;
    }
    else
    {
        int j=(cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3]);
        for(int i=j; i<j+10 ; i++)
        {
            int m = (M[i][0]-'0')*1000+(M[i][1]-'0')*100+(M[i][2]-'0')*10+(M[i][3]-'0');
            if( m == allNum)
               goto label;
        }
    }

    return allNum;
}


void loadtoMemory(int word_no)
{
    int k=0,j=0;
    while(k!=40)
    {

        M[word_no][j]=BUFF[k];

        if(BUFF[k]=='H'&& SI!=1) //to differentiate between the H instruction and 'H' string
        {
            j+=3;
        }
        k++,j++;
        if(j==4)
        {
            j=0;
            word_no++;
        }
    }

    clearbuff();
}


void LOAD()
{
    string line;
    int jobNo = 1;

    while(!input.eof())
    {

        getline(input,line);

        for(auto i=0;i<line.size() && i<40;i++)
        {
            BUFF[i]=line[i];
        }

        if(BUFF[0]=='$' && BUFF[1]=='A' && BUFF[2]=='M' && BUFF[3]=='J')
        {
            cout<<"------------------------ Job No."<<jobNo++<<" ------------------------"<<endl;
            init();
            pcb = new PCB();
            pcb->JOB_ID.push_back(BUFF[4]);
            pcb->JOB_ID.push_back(BUFF[5]);
            pcb->JOB_ID.push_back(BUFF[6]);
            pcb->JOB_ID.push_back(BUFF[7]);

            pcb->TTL=(BUFF[8]-'0')*1000 + (BUFF[9]-'0')*100 + (BUFF[10]-'0')*10 + (BUFF[11]-'0');
            pcb->TLL=(BUFF[12]-'0')*1000 + (BUFF[13]-'0')*100 + (BUFF[14]-'0')*10 + (BUFF[15]-'0');
            clearbuff();

            int ptr_no = allocate()*10;

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

            // Read Program cards
            int i=cpu->PTR[0]*1000+cpu->PTR[1]*100+cpu->PTR[2]*10+cpu->PTR[3];
            label:

            getline(input,line);

            for(auto i=0;i<line.size() && i<40;i++)
            {
                BUFF[i]=line[i];
            }

            if(!(BUFF[0]=='$' && BUFF[1]=='D' && BUFF[2]=='T' && BUFF[3]=='A'))
            {
                int frame_no = allocate();
                loadtoMemory(frame_no*10);

                for (int j=3; j>=0; j--)
                {
                    M[i][j] = frame_no%10+48;
                    frame_no /= 10;
                }

                i++;
                goto label;
            }
        }

        if(BUFF[0]=='$' && BUFF[1]=='D' && BUFF[2]=='T' && BUFF[3]=='A')
        {
            clearbuff();
            startExecution();
        }

        if(BUFF[0]=='$' && BUFF[1]=='E' && BUFF[2]=='N' && BUFF[3]=='D')
        {
            delete pcb;

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
}


void startExecution()
{
    cpu->IC[0]=0;
    cpu->IC[1]=0;
    executeUserProgram();
}


void executeUserProgram()
{
    while(!abTerminate)
    {
        VA[0] = cpu->IC[0];
        VA[1] = cpu->IC[1];
        addressMap();

        if (PI!=0)
        {
            MOS();
            break;
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

        else if(cpu->IR[0]=='L' && cpu->IR[1]=='R')
        {
            VA[0] = cpu->IR[2]-'0';
            VA[1] = cpu->IR[3]-'0';
            addressMap();

            if(PI!=0 || TI!=0)
            {
                MOS();
            }
            else
            {
                for(int i=0;i<4;i++)
                {
                    cpu->R[i]=M[RA][i];
                }

                simulation();
            }
        }

        else if(cpu->IR[0]=='S' && cpu->IR[1]=='R')
        {
            VA[0] = cpu->IR[2]-'0';
            VA[1] = cpu->IR[3]-'0';
            addressMap();

            if(PI == 3)
            {
                page_fault = 1;
                MOS();
            }
            else if(TI!=0 || PI==2)
            {
                MOS();
            }
            else
            {
                for(int i=0;i<4;i++)
                {
                    M[RA][i]=cpu->R[i];
                }

                simulation();
            }
        }

        else if(cpu->IR[0]=='C' && cpu->IR[1]=='R')
        {
            VA[0] = cpu->IR[2]-'0';
            VA[1] = cpu->IR[3]-'0';
            addressMap();

            if(PI!=0 || TI!=0)
            {
                MOS();
            }

            else
            {
                for(int i=0;i<4;i++)
                {
                    if(M[RA][i]==cpu->R[i])
                    {
                        cpu->C=1;
                    }
                    else
                    {
                        cpu->C=0;
                        break;
                    }
                }
                simulation();
            }
        }

        else if(cpu->IR[0]=='B' && cpu->IR[1]=='T')
        {
            VA[0] = cpu->IR[2]-'0';
            VA[1] = cpu->IR[3]-'0';
            addressMap();

            if(PI!=0 || TI!=0)
            {
                MOS();
            }

            else if(cpu->C==1)
            {
                cpu->IC[0]=cpu->IR[2]-'0';
                cpu->IC[1]=cpu->IR[3]-'0';

                simulation();
            }
        }

        else if(cpu->IR[0]=='H')
        {
            SI=3;

            MOS();
            break;
        }
        else
        {
            PI=1;
            MOS();
            break;
        }
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


void Terminate()
{
    if (EM==0) output<<"Normal Termination";
    else
    {
        output<<"Abnormal Termination: ";
        switch(EM)
        {
            case(1): output<<"Out Of Data"; break;
            case(2): output<<"Line Limit Exceeded"; break;
            case(3): output<<"Time Limit Exceeded"; break;
            case(4): output<<"Operation Code Error"; break;
            case(5): output<<"Operand Error"; break;
            case(6): output<<"Invalid Page Fault"; break;
        }

        abTerminate=1;
    }
    output<<endl<<"IC: "<<cpu->IC[0]<<cpu->IC[1]<<", IR: "<<cpu->IR[0]<<cpu->IR[1]<<cpu->IR[2]<<cpu->IR[3];
    output<<", TTL: "<<pcb->TTL<<", TTC: "<<pcb->TTC<<", TLL: "<<pcb->TLL<<", LLC: "<<pcb->LLC;
    output<<", SI: "<<SI<<", TI: "<<TI<<", PI: "<<PI;
    output<<endl<<endl<<endl;
}


void simulation()
{
    pcb->TTC += 1;

    if(pcb->TTC == pcb->TTL)
        TI = 2;
}


void MOS()
{
    if(TI==0)
    {
        if (PI == 1)
        {
            EM=4;
            Terminate();
        }

        else if (PI == 2)
        {
            EM=5;
            Terminate();
        }

        else if (PI == 3)
        {
            if (page_fault == 1)
            {
                int frame_no = allocate();

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

                page_fault = 0;
                PI = 0;

                simulation();
            }
            else
            {
                EM=6;
                Terminate();
            }
        }

        else if(SI==1)
        {
            string line;
            getline(input,line);

            for(auto i=0;i<line.size() && i<40;i++)
            {
                BUFF[i]=line[i];
            }

            if(BUFF[0]=='$' && BUFF[1]=='E' && BUFF[2]=='N' && BUFF[3]=='D')
            {
                EM = 1;
                Terminate();
                input.seekg(-8,ios::cur);
            }

            else
            {
                loadtoMemory(RA);

                SI = 0;
                simulation();
            }
        }

        else if(SI==2)
        {
            pcb->LLC +=1 ;
            if(pcb->LLC > pcb->TLL)
            {
                EM = 2;
                Terminate();
            }
            else
            {
                for(int i=RA;i<RA+10;i++)
                {
                    for(int j=0;j<4;j++)
                    {
                      output<<M[i][j];
                    }
                }
                output<<endl;

                SI = 0;
                simulation();
            }
        }

        else if(SI==3)
        {
            simulation();
            SI = 0;

            EM = 0;
            Terminate();
        }
    }

    else if(TI==2)
    {
        if (PI == 1)
        {
            EM=3;
            Terminate();
            output.seekg(-4, ios::cur);
            EM=4;
            Terminate();
        }

        else if (PI == 2)
        {
            EM=3;
            Terminate();
            output.seekg(-4, ios::cur);
            EM=5;
            Terminate();
        }

        else if (PI == 3)
        {
            EM=3;
            Terminate();
        }

        else if(SI==1)
        {
            EM=3;
            Terminate();
        }

        else if(SI==2)
        {
            pcb->LLC +=1 ;
            if(pcb->LLC > pcb->TLL)
            {
                EM = 2;
                Terminate();
                output.seekg(-4, ios::cur);
            }
            else
            {
                for(int i=RA;i<RA+10;i++)
                {
                    for(int j=0;j<4;j++)
                    {
                      output<<M[i][j];
                    }
                }
                output<<endl;

                simulation();
            }
            EM=3;
            Terminate();
        }

        else
        {
            EM = 3;
            Terminate();
        }
    }
}


int main()
{
    input.open("input.txt",ios::in);
    output.open("output.txt",ios::out);

    srand(time(0));

    LOAD();
    input.close();
    output.close();

    return 0;
}
