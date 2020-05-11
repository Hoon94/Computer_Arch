//Always branch taken

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int M[0x100000];
int Reg[32];
int pc;
int Fin;
void Fetch();
void Decode();
void Exe();
void Memory();
void WriteBack();
void control();

//print After the execution completion
int cycle;
int Rtype = -4;
int Itype;
int Jtype;
int Total;
int MA;
int TB;
int NB;
int Jump;
int Register;

//Branch prediction
int Targetpc1 = -1;
int Targetadd1;
int Targetpc2 = -1;
int Targetadd2;
int valid1;
int valid2;

int SignExtImm;
int SignExt(int immediate);
int ZeroExtImm;
int ZeroExt(int immediate);

typedef struct{
	int pc;
	int inst;
}ifid;

typedef struct{
	int pc;
	int inst;
	int opcode;
	int rs;
	int rd;
	int rt;
	int shamt;
	int funct;
	int address;
	int JumpAddr;
	int BranchAddr;
	int immediate;
	int v1;
	int v2;
	int simm;
	int zimm;

	//control
	int RegDst;
	int ALUSrc;
	int MemtoReg;
	int RegWrite;
	int MemRead;
	int MemWrite;
	int PCSrc1;
	int PCSrc2;
}idex;

typedef struct{
	int pc;
	int res;
	int Index;	
	int funct;
	int opcode;
	int v1;
	int v2;
	int v3;
	int rd;
	int rs;
	int rt;
	int End;

	//control
	int MemRead;
	int MemWrite;
	int MemtoReg;
	int RegWrite;	
}exmm;

typedef struct{
	int opcode;
	int res;
	int Mres;
	int MemtoReg;
	int RegWrite;
	int funct;
	int pc;
	int v3;
	int End;
}mmwb;

ifid ifid_latch[2] = {0};
idex idex_latch[2] = {0};
exmm exmm_latch[2] = {0};
mmwb mmwb_latch[2] = {0};	

void
main(int argc, char *argv[])
{
        // FILE ptr
        FILE *fp = NULL;
        // path string
        char *path;
	path = (char*)malloc(8*sizeof(char));
	printf("Write the file name : ");
	scanf("%s", path);

        // read-in value
        int val = 0;
	int i = 0;
	int res = 0;
	int inst;
	Reg[29] = 0x100000; //SP
	Reg[31] = 0xFFFFFFFF; //RA

        if(argc == 2)
        {
                path = argv[1];
        }

        // OPEN the file path
        fp = fopen(path, "rb");
        if(fp == NULL)
        {
                printf("invalid input file : %s\n", path);
                return;
        }
        // in a loop,
        // read file in from path
        while(1)
        {
                res = fread(&val, sizeof(val), 1, fp);
		inst = htonl(val);
		M[i] = inst;
		if(res == 0)
		{
                	break;
		}
		i++;
        }

	while(Fin != 0xFFFFFFFF)
	{
		cycle++;
		printf("---------------------cycle : %d----------------------\n", cycle);
		printf("PC = 0x%X\n", pc);
		Fetch();
		printf("Fetch : pc = 0x%X, inst = 0x%X\n", ifid_latch[0].pc, ifid_latch[0].inst);
		WriteBack();
		printf("Decode : pc = 0x%X, inst = 0x%X\t", idex_latch[0].pc, idex_latch[0].inst);
		Decode();
		Exe();
		Memory();

		ifid_latch[1] = ifid_latch[0];
	        idex_latch[1] = idex_latch[0];
		exmm_latch[1] = exmm_latch[0];
		mmwb_latch[1] = mmwb_latch[0];
	
		Total = Rtype + Itype + Jtype;
	}
	printf("--------------------------------------------------------------------\n");
        printf("Fianl : R[2] = %d\n", Reg[2]);
	printf("Number of cycle : %d\n", cycle);
	printf("R-type : %d, I-type : %d, J-type : %d\n", Rtype, Itype, Jtype);
	printf("Total number of instructions : %d\n", Total);
	printf("Memory Access : %d\n", MA);
	printf("Register Access : %d\n", Register);
	printf("Branches : %d\n", TB);
	printf("Not-taken branches : %d\n", NB);
	printf("Number of Jumps : %d\n", Jump);
	// close the file
        fclose(fp);
}

void
Fetch()
{
	ifid_latch[0].inst = M[(pc/4)];
        ifid_latch[0].pc = pc;
	if(pc == 0xFFFFFFFF)
	{
	}
	else if(pc == Targetpc1)
	{
		pc = Targetadd1;
		TB++;
	}
	else if(pc == Targetpc2)
	{
		pc = Targetadd2;
		TB++;
	}
	else
	{
        	pc = pc + 4;
	}
}

void
Decode()
{
        int inst = ifid_latch[1].inst;
        idex_latch[0].opcode = (inst & 0xFC000000) >> 26;
        // R type
        if(idex_latch[0].opcode == 0x0)
        {
                idex_latch[0].rs = (inst & 0x3E00000) >> 21;
                idex_latch[0].rt = (inst & 0x1F0000) >> 16;
                idex_latch[0].rd = (inst & 0xF800) >> 11;
                idex_latch[0].shamt = (inst & 0x7C0) >> 6;
                idex_latch[0].funct = (inst & 0X3F);
                idex_latch[0].v1 = Reg[idex_latch[0].rs];
                idex_latch[0].v2 = Reg[idex_latch[0].rt];
                //printf("0x%08X (opcode : 0x%X, rs : %d, rt : %d, rd : %d, shamt : 0x%X,     funct : 0x%X)\n", inst, opcode, rs, rt, rd, shamt, funct);
                printf("R type\n");
        }
        // J type
        else if(idex_latch[0].opcode == 0x2 || idex_latch[0].opcode == 0x3)
        {
                idex_latch[0].address = (inst & 0x3FFFFFF);
                idex_latch[0].JumpAddr = ifid_latch[1].pc & 0xF0000000 | idex_latch[0].address << 2;
                //printf("0x%08X (opcode : 0x%X, address : 0x%X, JumpAddr : 0x%X)\n", ins    t, opcode, address, JumpAddr);
                printf("J type\n");
        }
        // I type
        else
        {
                idex_latch[0].rs = (inst & 0x3E00000) >> 21;
                idex_latch[0].rt = (inst & 0x1F0000) >> 16;
                idex_latch[0].immediate = inst & 0xFFFF;
                idex_latch[0].simm = SignExt(idex_latch[0].immediate);
                idex_latch[0].zimm = ZeroExt(idex_latch[0].immediate);
                idex_latch[0].BranchAddr = idex_latch[0].simm << 2;
                idex_latch[0].v1 = Reg[idex_latch[0].rs];
                idex_latch[0].v2 = Reg[idex_latch[0].rt];
                //printf("0x%08X (opcode : 0x%X, rs : %d, rt : %d, immediate : %d, Branch    Addr : 0x%X)\n", inst, opcode, rs, rt, immediate, BranchAddr);
                printf("I type\n");
        }
	
	idex_latch[0].pc = ifid_latch[1].pc;
        idex_latch[0].inst = inst;

        control();

	//Jump
        if(idex_latch[0].opcode == 0x2)
        {
                pc = idex_latch[0].JumpAddr;
       		Jump++;
	}
        else if(idex_latch[0].opcode == 0x3)
        {
                Reg[31] = idex_latch[0].pc + 8;
                pc = idex_latch[0].JumpAddr;
        	Jump++;
	}
	
	if((idex_latch[0].opcode == 0x4) && (idex_latch[0].pc != Targetpc1))
	{
		Targetpc1 = ifid_latch[1].pc;
		Targetadd1 = ifid_latch[1].pc + 4 + idex_latch[0].BranchAddr;
		valid1 = 1;
	}
	if((idex_latch[0].opcode == 0x5) && (idex_latch[0].pc != Targetpc2))
	{
		Targetpc2 = ifid_latch[1].pc;
		Targetadd2 = ifid_latch[1].pc + 4 + idex_latch[0].BranchAddr;
		valid2 = 1;
	}
}

void
Exe()
{
	int ALUinput;
	//Data Forwarding Paths (v2)
	if((idex_latch[1].rs != 0) && (idex_latch[1].rs == mmwb_latch[0].v3) && (mmwb_latch[0].RegWrite))
	{
		if(mmwb_latch[0].MemtoReg == 1)
		{
			idex_latch[1].v1 = mmwb_latch[0].Mres;
		}
		else
		{
			idex_latch[1].v1 = mmwb_latch[0].res;
		}
	}
	if((idex_latch[1].rs != 0) && (idex_latch[1].rs == exmm_latch[0].v3) && (exmm_latch[0].RegWrite))
	{
		idex_latch[1].v1 = exmm_latch[0].res;
	}

	if((idex_latch[1].rt != 0) && (idex_latch[1].rt == mmwb_latch[0].v3) && (mmwb_latch[0].RegWrite))
	{
		if(mmwb_latch[0].MemtoReg == 1)
		{
			idex_latch[1].v2 = mmwb_latch[0].Mres;
		}
		else
		{
			idex_latch[1].v2 = mmwb_latch[0].res;
		}
	}
	if((idex_latch[1].rt != 0) && (idex_latch[1].rt == exmm_latch[0].v3) && (exmm_latch[0].RegWrite))
	{
		idex_latch[1].v2 = exmm_latch[0].res;
	}

	if(idex_latch[1].ALUSrc == 1)
	{
		ALUinput = idex_latch[1].simm;
	}
	else
	{
		ALUinput= idex_latch[1].v2;
	}
	
	printf("EXE : ");
	switch(idex_latch[1].opcode){
	//R type exe
	case 0x0:
		switch(idex_latch[1].funct){
		case 0x20: //add
			exmm_latch[0].res = idex_latch[1].v1 + ALUinput;
			printf("add : R[%d] = R[%d] + R[%d]\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x21: //addu
			exmm_latch[0].res = idex_latch[1].v1 + ALUinput;
			printf("addu : R[%d] = R[%d] + R[%d](%d)\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt, exmm_latch[0].res);
			break;
		case 0x24: //and
			exmm_latch[0].res = idex_latch[1].v1 & ALUinput;
			printf("and : R[%d] = R[%d] & R[%d]\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x08: //jr
			pc = idex_latch[1].v1;
			exmm_latch[0].End = pc;
			printf("jr : PC = R[%d]\n", idex_latch[1].rs);
			break;
		case 0x27: //nor
			exmm_latch[0].res =~ (idex_latch[1].v1|ALUinput);
			printf("nor : R[%d] =~(R[%d] | R[%d])\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x25: //or
			exmm_latch[0].res = idex_latch[1].v1|ALUinput;
			printf("or : R[%d] = R[%d] | R[%d]\n", idex_latch[0].rd, idex_latch[0].rs, idex_latch[0].rt);
			break;
		case 0x2A: //slt
			exmm_latch[0].res = (idex_latch[1].v1 < ALUinput)?1:0;
			printf("slt : R[%d] = (R[%d] < R[%d]) ? 1 : 0\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x2B: //sltu
			exmm_latch[0].res = (idex_latch[1].v1 < ALUinput)?1:0;
			printf("sltu : R[%d] = (R[%d] < R[%d]) ? 1 : 0\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x00: //sll
			exmm_latch[0].res = ALUinput << idex_latch[1].shamt;
			printf("sll : R[%d] = R[%d] << %d\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].shamt);
			break;
		case 0x02: //srl
			exmm_latch[0].res = ALUinput >> idex_latch[1].shamt;
			printf("srl : R[%d] + R[%d] >> %d\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].shamt);
			break;
		case 0x22: //sub
			exmm_latch[0].res = idex_latch[1].v1 - ALUinput;
			printf("sub : R[%d] = R[%d] - R[%d]\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x23: //subu
			exmm_latch[0].res = idex_latch[1].v1 - ALUinput;
			printf("subu : R[%d] = R[%d] - R[%d]\n", idex_latch[1].rd, idex_latch[1].rs, idex_latch[1].rt);
			break;
		case 0x9: //jalr
			Reg[31] = idex_latch[1].pc + 4;
			pc = idex_latch[1].v1;
			memset(&ifid_latch[0], 0, sizeof(ifid));
			printf("Reg[31] = 0x%X, pc = 0x%X\n", Reg[31], exmm_latch[0].res);
			break;
		}
		break;

	// I type
	case 0x8: //addi
		exmm_latch[0].res = idex_latch[1].v1 + ALUinput;
		printf("addi : R[%d] = R[%d] + %d\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0x9: //addiu
		exmm_latch[0].res = idex_latch[1].v1 + ALUinput;
		printf("addiu : R[%d] = R[%d] + %d\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0xC: //andi
		exmm_latch[0].res = idex_latch[1].v1 & idex_latch[1].zimm;
		printf("andi : R[%d] = R[%d] & %d\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].zimm);
		break;
	case 0x4: //beq
		if(valid1 == 1)
		{
			if(idex_latch[1].v1 == idex_latch[1].v2)
			{
				memset(&ifid_latch[0], 0, sizeof(ifid));
				memset(&idex_latch[0], 0, sizeof(idex));
				pc = idex_latch[1].pc + 4 + idex_latch[1].BranchAddr;
				Rtype = Rtype - 2;
				TB++;
				valid1 = 0;
				Register = Register - 2;
			}
		}
		if(idex_latch[1].v1 != idex_latch[1].v2)
		{
			memset(&ifid_latch[0], 0, sizeof(ifid));
			memset(&idex_latch[0], 0, sizeof(idex));
			pc = idex_latch[1].pc + 4;
			Rtype = Rtype - 2;
			NB++;
			Register = Register - 2;
		}
		printf("beq : R[%d] == R[%d]\n", idex_latch[1].rs, idex_latch[1].rt);
		break;
        case 0x5: //bne
		if(valid2 == 1)
		{
			if(idex_latch[1].v1 != idex_latch[1].v2)
			{
				memset(&ifid_latch[0], 0, sizeof(ifid));
				memset(&idex_latch[0], 0, sizeof(idex));
				pc = idex_latch[1].pc + 4 + idex_latch[1].BranchAddr;
				Rtype = Rtype - 2;
				TB++;
				valid2 = 0;
				Register = Register - 2;
			}
		}
		if(idex_latch[1].v1 == idex_latch[1].v2)
		{
			memset(&ifid_latch[0], 0, sizeof(ifid));
			memset(&idex_latch[0], 0, sizeof(idex));
			pc = idex_latch[1].pc + 4;
			Rtype = Rtype - 2;
			NB++;
			Register = Register - 2;
		}
		printf("bne : R[%d] != R[%d]\n", idex_latch[1].rs, idex_latch[1].rt);
		break;
	case 0x24: //lbu
		exmm_latch[0].res = (M[idex_latch[1].v1 + ALUinput] & 0xFF);
		printf("lbu : R[%d] = M[R[%d] + %d]\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0x25: //lhu
		exmm_latch[0].res = (M[idex_latch[1].v1 + ALUinput] & 0xFFFF);
		printf("lhu : R[%d] = M[R[%d] + %d]\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0x30: //ll
		exmm_latch[0].res = M[idex_latch[1].v1 + ALUinput];
		printf("ll : R[%d] = M[R[%d] + %d]\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0xF: //lui
		exmm_latch[0].res = idex_latch[1].immediate << 16;
		printf("lui : R[%d] = %X << 16\n", idex_latch[1].rt, idex_latch[1].immediate);
		break;
	case 0x23: //lw
		exmm_latch[0].res = idex_latch[1].v1 + ALUinput;
		printf("lw : R[%d] = M[0x%X], Mvalue = %d\n", idex_latch[1].rt, exmm_latch[0].res, M[exmm_latch[0].res]);
		break;
	case 0xd: //ori
		exmm_latch[0].res = idex_latch[1].v1|idex_latch[1].zimm;
		printf("ori : R[%d] = R[%d] | %d\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].zimm);
		break;
	case 0xa: //slti
		exmm_latch[0].res = (idex_latch[1].v1 < ALUinput)? 1:0;
		printf("slti : R[%d] = (R[%d] < %d)? 1 : 0\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0xb: //sltiu
		exmm_latch[0].res = (idex_latch[1].v1 < ALUinput)? 1:0;
		printf("sltiu : R[%d] = (R[%d] < %d)? 1 : 0\n", idex_latch[1].rt, idex_latch[1].rs, idex_latch[1].simm);
		break;
	case 0x28: //sb
		exmm_latch[0].Index = idex_latch[1].v1 + ALUinput;
		exmm_latch[0].res = idex_latch[1].v2 & 0xFF;
		break;
	/*case 0x38: //sc
		Index = Reg[rs]+SignExt(immediate);
		res = Reg[rt];
		Reg[rt] = (atomic)? 1:0;
		break;*/ 
	case 0x29: //sh
		exmm_latch[0].Index = idex_latch[1].v1 + ALUinput;
		exmm_latch[0].res = idex_latch[1].v2 & 0xFFFF;
		break;
	case 0x2B: //sw
		exmm_latch[0].res = idex_latch[1].v2;
		exmm_latch[0].Index = idex_latch[1].v1 + ALUinput;
		printf("Index = 0x%X, Reg[%d] = %X, SignExtImm = %X, rt = %d\n\n", exmm_latch[0].Index, idex_latch[1].rs, idex_latch[1].v1, idex_latch[1].simm, idex_latch[1].rt);
	 	break;

	// Except	
	default:
		break;
	}

	exmm_latch[0].v2 = idex_latch[1].v2;
	exmm_latch[0].pc = idex_latch[1].pc;


	if(idex_latch[1].RegDst == 1)
	{
			if((idex_latch[1].opcode == 0x0) && (idex_latch[1].funct == 0x9))
			{
			}
			else
			{
				exmm_latch[0].v3 = idex_latch[1].rd;
			}
	}
	else
	{
		exmm_latch[0].v3 = idex_latch[1].rt;
	}
	exmm_latch[0].MemRead = idex_latch[1].MemRead;
	exmm_latch[0].MemWrite = idex_latch[1].MemWrite;
	exmm_latch[0].MemtoReg = idex_latch[1].MemtoReg;
	exmm_latch[0].RegWrite = idex_latch[1].RegWrite;
	exmm_latch[0].funct = idex_latch[1].funct;
	exmm_latch[0].opcode = idex_latch[1].opcode;
}

void
control()
{
	if(idex_latch[0].opcode == 0x0) //R type
	{
		idex_latch[0].RegDst = 1;
	}
	else
	{
		idex_latch[0].RegDst = 0;
	}
	if((idex_latch[0].opcode != 0x0) && (idex_latch[0].opcode != 0x4) && (idex_latch[0].opcode != 0x5))
	{
		idex_latch[0].ALUSrc = 1;
	}
	else
	{
		idex_latch[0].ALUSrc = 0;
	}
	if(idex_latch[0].opcode == 0x23) //lw
	{
		idex_latch[0].MemtoReg = 1;
	}
	else
	{
		idex_latch[0].MemtoReg = 0;
	}
	if((idex_latch[0].opcode != 0x2B) && (idex_latch[0].opcode != 0x4) && (idex_latch[0].opcode != 0x5) && (idex_latch[0].opcode != 0x2) && (idex_latch[0].opcode != 0x3) && (idex_latch[0].funct != 0x8))
	{
		idex_latch[0].RegWrite = 1;
	}
	else
	{
		idex_latch[0].RegWrite = 0;
	}
	if(idex_latch[0].opcode == 0x23) //lw
	{
		idex_latch[0].MemRead = 1;
	}
	else
	{
		idex_latch[0].MemRead = 0;
	}
	if(idex_latch[0].opcode == 0x2B) //sw
	{
		idex_latch[0].MemWrite = 1;
	}
	else
	{
		idex_latch[0].MemWrite = 0;
	}
	if((idex_latch[0].opcode == 0x2) || (idex_latch[0].opcode == 0x3))
	{
		idex_latch[0].PCSrc1 = 1;
	}
	else
	{
		idex_latch[0].PCSrc1 = 0;
	}
	if((idex_latch[0].opcode == 0x4) || (idex_latch[0].opcode == 0x5))
	{
		idex_latch[0].PCSrc2 = 1;
	}
	else
	{
		idex_latch[0].PCSrc2 = 0;
	}
}

void
Memory()
{
	printf("Memory : pc = 0x%X\n", exmm_latch[1].pc);
	if(exmm_latch[1].MemRead == 1)
	{
		mmwb_latch[0].Mres = M[exmm_latch[1].res];
		printf("Memory(LW) : %d <= M[0x%X]\n", mmwb_latch[0].Mres, M[exmm_latch[1].res]);
		MA++;
	}
	if(exmm_latch[1].MemWrite == 1)
	{
		M[exmm_latch[1].Index] = exmm_latch[1].res;
		printf("Memory(SW) : M[0x%X] => %d\n", exmm_latch[1].Index, exmm_latch[1].res);
		MA++;
	}
	
	mmwb_latch[0].res = exmm_latch[1].res;
	mmwb_latch[0].v3 = exmm_latch[1].v3;
	mmwb_latch[0].pc = exmm_latch[1].pc;
	mmwb_latch[0].End = exmm_latch[1].End;
	mmwb_latch[0].RegWrite = exmm_latch[1].RegWrite;
	mmwb_latch[0].MemtoReg = exmm_latch[1].MemtoReg;
	mmwb_latch[0].funct = exmm_latch[1].funct;
	mmwb_latch[0].opcode = exmm_latch[1].opcode;
}
	

void
WriteBack()
{
	printf("WriteBack : pc = 0x%X\n", mmwb_latch[1].pc);
	if((mmwb_latch[1].RegWrite == 1) && (mmwb_latch[1].funct != 0x8))
	{
		if(mmwb_latch[1].MemtoReg == 1)
		{
			Reg[mmwb_latch[1].v3] = mmwb_latch[1].Mres;
		}
		else
		{
			Reg[mmwb_latch[1].v3] = mmwb_latch[1].res;
		}
		Register++;
	}
	else
	{
	}
	
	if(mmwb_latch[1].opcode == 0x0)
	{
		Rtype++;
	}
	else if((mmwb_latch[1].opcode == 0x2) || (mmwb_latch[1].opcode == 0x3))
	{
		Jtype++;
	}
	else
	{
		Itype++;
	}
		

	Fin = mmwb_latch[1].End;
}


int
SignExt(int immediate)
{
	if((immediate & 0xFFFF) >= 0x8000)
	{
		SignExtImm = 0xFFFF0000 | (immediate & 0xFFFF);
	}
	else
	{
		SignExtImm = immediate & 0xFFFF;
	}
	
	return SignExtImm;
}

int
ZeroExt(int immediate)
{
	ZeroExtImm = (immediate & 0x0000FFFF);
	return ZeroExtImm;
}
