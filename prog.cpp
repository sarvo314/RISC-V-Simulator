#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

using namespace std;

#pragma region FILE_RELATED_DATA
    // stores the file name of the dump file
    string filename;
    ifstream readFile("input.mc");
#pragma endregion FILE_RELATED_DATA 


#pragma region INSTRUCTION_RELATED_DATA 
    bitset<32> currentPCAdd(0);
    bitset<32> nextPCAdd(0);
    bitset<32> currentInstruction;
#pragma endregion INSTRUCTION_RELATED_DATA 


#pragma region DECODE_RELATED_DATA

typedef struct if_de_handshake{
    int Op1,Op2;//oprands
    int Rd;//RF write destinstion
    int imm,immU,immS,immJ,immB;
    int branch_target_select;//0 for immB; 1 for immJ
    int Result_select;//0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int ALU_Operation;//0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
    int mem_OP;//0:No operation 1:write 2:read
    int RFWrite;
} If_DE;

If_DE handShake;

int RF[32];//Register file
#pragma endregion DECODE_RELATED_DATA

class Fetch
{
    bitset<32> fetch_instruction(int flag)
    {
        //the part b4 space PC address
        string hex_str;
        
        if(flag == 0)
        {
            string pc_str = read();
            currentPCAdd = HexStringToBitset(pc_str);
            hex_str = read();
            addToBitset();
        }
        else if(flag == 1)
        {
            currentPCAdd = nextPCAdd;
            ifstream tempRead("input.mc");
            string tempHexa;
            tempRead>>tempHexa;
            bitset<32> tempBitset(HexStringToBitset(tempHexa));
            while(tempBitset != nextPCAdd)
            {
                tempRead>>tempHexa;
                tempRead>>tempHexa;
                tempBitset = HexStringToBitset(tempHexa); 
            }

            tempRead>>hex_str;
        }
        //stoul converts string of type 0x012312 to its decimal value
        return HexStringToBitset(hex_str);

    }
    string read()
    {
        string line;
        readFile>>line;
        return line;
    }
    void addToBitset()
    {
        currentPCAdd = currentPCAdd.to_ulong() + 4; // add 4 to the bitset
        
    }

    bitset<32> HexStringToBitset(string hex_instr)
    {
        unsigned long hex_to_dec_val = stoul(hex_instr, nullptr, 16);
        bitset<32> binary_num(hex_to_dec_val);
        return binary_num;
    }

    public:
    Fetch(int flag = 0)
    {
       
        currentInstruction = fetch_instruction(flag);
    }
    

};
//makes .mc file
void make_file()
{
        cout << "Enter input filename: ";
        // cin >> filename;
        filename = "input.txt";


        ifstream infile;
        //the input dump file from venus
        //format of it is
        //0x12312312
        infile.open(filename);
        ofstream outfile;
        outfile.open("input.mc");
        if (!infile)
        {
            cerr << "Error: Could not open input file.\n";
            exit(0);
        }

        string line;
        int offset = 0;
        
        while (getline(infile, line))
        {
            cout << "0x" << hex<<offset << " " << line << endl; 
            //skip 0x part
            string hex_str = line;
            //stoul converts string of type 0x012312 to its decimal value
            unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
            bitset<32> binary_num(hex_to_dec_val);
            cout<<binary_num<<endl;
            outfile << "0x" <<hex<<offset << " " << line << endl;
            offset += 4; // increase offset by 4 characters
        }

        infile.close();
        outfile.close(); 
}

class Decode
{
    void Decode_Instruction()
    {
        bitset<5> opcode;
        char type_of_instruction;

        for (int i = 0; i < 5; i++)//ignoring last two bit as these are always 11
        {
            opcode[i]=currentInstruction[i+2];
        }
        // cout<<opcode;

        if((!opcode[4])&&opcode[3]&&opcode[2]&&(!opcode[1])&&(!opcode[0]))//01100
        type_of_instruction = 'R';

        else if((!opcode[4])&&(!opcode[3])&&(!opcode[1])&&(!opcode[0]))//00X00
        type_of_instruction = 'I';

        else if((!opcode[4])&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&(!opcode[0]))//01000
        type_of_instruction = 'S';

        else if(opcode[4]&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&(!opcode[0]))//11000
        type_of_instruction = 'B';

        else if(opcode[4]&&opcode[3]&&(!opcode[2])&&opcode[1]&&opcode[0])//11011
        type_of_instruction = 'J';

        else if(opcode[4]&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&opcode[0])//11001
        type_of_instruction = 'I';

        else if((!opcode[4])&&opcode[2]&&(!opcode[1])&&opcode[0])//0X101
        type_of_instruction = 'U';

        else if(opcode[4]&&opcode[3]&&opcode[2]&&(!opcode[1])&&(!opcode[0]))//11100
        type_of_instruction = 'I';

        else type_of_instruction = 'Z';

        // cout<<type_of_instruction;

        switch (type_of_instruction)
        {
            case 'R':
               {
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())//alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
                    {
                    case 0:
                    {
                        if (currentInstruction[30])
                        {
                            handShake.ALU_Operation=1;
                        }
                        else{
                            handShake.ALU_Operation=0;
                            
                        }
                    }
                        break;
                    case 4:
                        handShake.ALU_Operation=2;
                    
                        break;
                    case 6:
                        handShake.ALU_Operation=3;
                        break;
                    case 7:
                        handShake.ALU_Operation=4;
                        break;
                    case 1:
                        handShake.ALU_Operation=5;
                        break;
                    case 5:
                        {
                            if (currentInstruction[30])
                            {
                                handShake.ALU_Operation=7;
                            }
                            else{
                                handShake.ALU_Operation=6;
                            }
                        }
                        break;
                    case 2:
                        handShake.ALU_Operation=8;
                        break;
                                          
                    default:
                        break;
                    }

                    // cout<<"operation is"<<handShake.ALU_Operation;

                    handShake.Result_select=3;
                    handShake.mem_OP=0;
                    handShake.RFWrite=1;

                    bitset <5> rs1;
                    bitset <5> rs2;
                    bitset <5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                        rd[i]=currentInstruction[i+7]; 
                    }

                    handShake.Op1 = RF[rs1.to_ulong()];//value of rs1
                    handShake.Op2 = RF[rs2.to_ulong()];//value of rs2
                    handShake.Rd = rd.to_ulong();//address of RD
                
               }
                break;

            case 'I':
                {
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())//alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
                    {
                    case 0:
                        handShake.ALU_Operation=0;
                        break;
                    case 4:
                        handShake.ALU_Operation=2;
                    
                        break;
                    case 6:
                        handShake.ALU_Operation=3;
                        break;
                    case 7:
                        handShake.ALU_Operation=4;
                        break;
                    case 1:
                        handShake.ALU_Operation=5;
                        break;
                    case 5:
                        {
                            if (currentInstruction[30])
                            {
                                handShake.ALU_Operation=7;
                            }
                            else{
                                handShake.ALU_Operation=6;
                            }
                        }
                        break;
                    case 2:
                        handShake.ALU_Operation=8;
                        break;
                                          
                    default:
                        break;
                    }

                    handShake.Result_select=3;
                    handShake.mem_OP=0;
                    handShake.RFWrite=1;

                    bitset <5> rs1;
                    bitset <5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rd[i]=currentInstruction[i+7];     
                    }

                    bitset <12> imm;
                    for (int i = 0; i < 12; i++)
                    {
                        imm[i] = currentInstruction[i+20];
                    }

                    handShake.Op1 = RF[rs1.to_ulong()];//value of rs1
                    handShake.Op2 = imm.to_ulong();//value of rs2
                    handShake.Rd = rd.to_ulong();//address of RD
                }
                break;

            case 'S':
                {
                    handShake.ALU_Operation=0;//add

                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())
                    {
                    case '0':
                        {
                            
                        }
                        break;
                    case '1':
                        {
                            
                        }
                        break;
                    case '2':
                        {
                            
                        }
                        break;
                    
                    default:
                        break;
                    }

                    bitset <5> rs1;
                    bitset <5> rs2;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                    }

                }
                break;
            case 'B':
               {
                    handShake.ALU_Operation=1;//subtract
                    handShake.branch_target_select=0;//immB
                    handShake.RFWrite= 0;
                    handShake.mem_OP=0;                    

                    bitset <5> rs1;
                    bitset <5> rs2;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                    }

                    handShake.Op1 = RF[rs1.to_ulong()];
                    handShake.Op2 = RF[rs2.to_ulong()];

                    bitset<14> immB;

                    immB[0]=0;//lsb is 0
                    immB[11]=currentInstruction[7];
                    immB[12]=currentInstruction[31];

                    for (int i = 1; i < 5; i++)
                    {
                        immB[i] = currentInstruction[7+i];
                    }

                    for (int i = 5; i < 11; i++)
                    {
                        immB[i] = currentInstruction[20+i];
                    }

                    handShake.immB = immB.to_ulong();

                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())
                    {
                    case 0:
                        {

                        }
                        break;
                    case 1:
                        {
                            
                        }
                        break;
                    case 4:
                        {
                            
                        }
                        break;
                    case 5:
                        {
                            
                        }
                        break;
                    case 6:
                        {
                            
                        }
                        break;
                    case 7:
                        {
                            
                        }
                        break;
                    
                    default:
                    {
                        cerr << "Error: Could not identify instruction.\n";
                        exit(0);
                    }
                        break;
                    }
                    

                
               }
                break;
            
            case 'U':
               {
                
               }
                break;
            
            case 'J':
                {
                    
                }
                break;

            
            default: 
                {
                    cerr << "Error: Could not identify instruction.\n";
                    exit(0);
                }
                break;
        }
        

    }


    public:

    Decode()
    {
        Decode_Instruction();
    }

};

void reset_pointer()
{

}
int main()
{
    make_file();
    //whenever you create an object pc increments by 4 and new instruction is loaded in currentInstruction global var
    // Fetch a;
    // Fetch b;
    // cout<<endl<<"Current instr"<<currentInstruction;

    Fetch a;
    cout<<endl;
    Decode A;

    return 0;
}