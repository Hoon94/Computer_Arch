#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int M[0x800000];
int Reg[32];
void Fetch();
void Decode();
void Exe();
void Memory();
void WriteBack();
int SignExt(int immediate);
int SignExtImm;
int ZeroExt(int immediate);
int ZeroExtImm;
int BranchAddr;
int JumpAddr;
int opcode;
int rs;
int rt;
int rd;
int shamt;
int funct;
int immediate;
int address;
int inst;
int pc = 0x0;
int res;
int Index;

//Control
int RegDst; //mux
//int Jump; //mux
int PCSrc1;
//int Branch;
int PCSrc2;
int MemRead;
int MemtoReg; //mux
int ALUOp;
int MemWrite;
int ALUSrc; //mux
int RegWrite;

void control();
int cycle = 0;
int Rtype;
int Itype;
int Jtype;
int MA;
int TB;

void
main(int argc, char *argv[])
{
        // FILE ptr
        FILE *fp = NULL;
        // path string
        char *path = "fib2.bin";
        // read-in value
        int val = 0;
	int i = 0;
	Reg[29] = 0x800000; //SP
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

	while((pc/4) <= i && pc != 0xFFFFFFFF)
	{	
		cycle++;
		printf("-------------------------------  %d  -------------------------------\n", cycle);
		printf(" pc : 0x%X\n\n", pc);
		Fetch();
		Decode();
		Exe();
		Memory();
		WriteBack();
		printf("next pc = 0x%X\n\n", pc);
	}
	printf("--------------------------------------------------------------------\n");
        printf("Fianl : R[2] = %d, Cycle = %d\n", Reg[2], cycle);
	printf("R-type : %d, I-type : %d, J-type : %d\n", Rtype, Itype, Jtype);
	printf("Memory Access : %d\n", MA);
	printf("Taken Branches : %d\n", TB);
	// close the file
        fclose(fp);
}

void
Exe()
{
	control();
	switch(opcode){
	//R type exe
	case 0x0:
		switch(funct){
		case 0x20: //add
			res = Reg[rs] + Reg[rt];
			printf("add : R[%d] = R[%d] + R[%d]\n", rd, rs, rt);
			break;
		case 0x21: //addu
			res = Reg[rs] + Reg[rt];
			printf("addu : R[%d] = R[%d] + R[%d]\n", rd, rs, rt);
			break;
		case 0x24: //and
			res = Reg[rs] & Reg[rt];
			printf("and : R[%d] = R[%d] & R[%d]\n", rd, rs, rt);
			break;
		case 0x08: //jr
			res = Reg[rs];
			printf("jr : PC = R[%d]\n", rs);
			break;
		case 0x27: //nor
			res =~ (Reg[rs]|Reg[rt]);
			printf("nor : R[%d] =~(R[%d] | R[%d])\n", rd, rs, rt);
			break;
		case 0x25: //or
			printf("or : R[%d] = R[%d] | R[%d]\n", rd, rs, rt);
			break;
		case 0x2A: //slt
			res = (Reg[rs]<Reg[rt])?1:0;
			printf("slt : R[%d] = (R[%d] < R[%d]) ? 1 : 0\n", rd, rs, rt);
			break;
		case 0x2B: //sltu
			res = (Reg[rs]<Reg[rt])?1:0;
			printf("sltu : R[%d] = (R[%d] < R[%d]) ? 1 : 0\n", rd, rs, rt);
			break;
		case 0x00: //sll
			res = Reg[rt] << shamt;
			printf("sll : R[%d] = R[%d] << %d\n", rd, rs, shamt);
			break;
		case 0x02: //srl
			res = Reg[rt] >> shamt;
			printf("srl : R[%d] + R[%d] >> %d\n", rd, rs, shamt);
			break;
		case 0x22: //sub
			res = Reg[rs] - Reg[rt];
			printf("sub : R[%d] = R[%d] - R[%d]\n", rd, rs, rt);
			break;
		case 0x23: //subu
			res = Reg[rs] - Reg[rt];
			printf("subu : R[%d] = R[%d] - R[%d]\n", rd, rs, rt);
			break;
		case 0x9: //jalr
			Reg[31] = pc + 4;
			res = Reg[rs];
			printf("Reg[31] = 0x%X, pc = 0x%X\n", Reg[31], res);
			break;
		}
		break;

	// I type
	case 0x8: //addi
		res = Reg[rs] + SignExt(immediate);
		printf("addi : R[%d] = R[%d] + %d\n", rt, rs, SignExtImm);
		break;
	case 0x9: //addiu
		res = Reg[rs] + SignExt(immediate);
		printf("addiu : R[%d] = R[%d] + %d\n", rt, rs, SignExtImm);
		break;
	case 0xC: //andi
		res = Reg[rs] & ZeroExt(immediate);
		printf("andi : R[%d] = R[%d] & %d\n", rt, rs, ZeroExtImm);
		break;
	case 0x4: //beq
		printf("beq : R[%d] == R[%d]\n", rs, rt);
		break;
        case 0x5: //bne
		printf("bne : R[%d] != R[%d]\n", rs, rt);
		break;
	case 0x24: //lbu
		res = (M[Reg[rs]+SignExt(immediate)] & 0xFF);
		printf("lbu : R[%d] = M[R[%d] + %d]\n", rt, rs, SignExtImm);
		break;
	case 0x25: //lhu
		res = (M[Reg[rs]+SignExt(immediate)] & 0xFFFF);
		printf("lhu : R[%d] = M[R[%d] + %d]\n", rt, rs, SignExtImm);
		break;
	case 0x30: //ll
		res = M[Reg[rs] + SignExt(immediate)];
		printf("ll : R[%d] = M[R[%d] + %d]\n", rt, rs, SignExtImm);
		break;
	case 0xF: //lui
		res = immediate << 16;
		printf("lui : R[%d] = %X << 16\n", rt, immediate);
		break;
	case 0x23: //lw
		res = Reg[rs] + SignExt(immediate);
		printf("lw : R[%d] = M[R[%d] + %d]\n", rt, rs, SignExtImm);
		break;
	case 0xd: //ori
		res = Reg[rs]|ZeroExt(immediate);
		printf("ori : R[%d] = R[%d] | %d\n", rt, rs, ZeroExtImm);
		break;
	case 0xa: //slti
		res = (Reg[rs] < SignExt(immediate))? 1:0;
		printf("slti : R[%d] = (R[%d] < %d)? 1 : 0\n", rt, rs, SignExtImm);
		break;
	case 0xb: //sltiu
		res = (Reg[rs] < SignExt(immediate))? 1:0;
		printf("sltiu : R[%d] = (R[%d] < %d)? 1 : 0\n", rt, rs, SignExtImm);
		break;
	case 0x28: //sb
		Index = Reg[rs]+SignExt(immediate);
		res = Reg[rt] & 0xFF;
		break;
	/*case 0x38: //sc
		Index = Reg[rs]+SignExt(immediate);
		res = Reg[rt];
		Reg[rt] = (atomic)? 1:0;
		break;*/ 
	case 0x29: //sh
		Index = Reg[rs]+SignExt(immediate);
		res = Reg[rt] & 0xFFFF;
		break;
	case 0x2B: //sw
		res = Reg[rt];
		Index = Reg[rs] + SignExt(immediate);
		printf("Index = 0x%X, Reg[%d] = %X, SignExtImm = %X\n\n", Index, rs, Reg[rs], SignExtImm);
	 	break;

	// J type
	case 0x2: //j
		printf("j : PC = 0x%X\n", JumpAddr);
		break;
	case 0x3: //jal
		Reg[31] = pc + 8;
		printf("Reg[31] = 0x%X, pc = 0x%X\n", Reg[31], pc);
		break;

	// Except	
	default:
		break;
	}
}

void
control()
{
	if(opcode == 0x0) //R type
	{
		RegDst = 1;
	}
	else
	{
		RegDst = 0;
	}
	if((opcode != 0x0) && (opcode != 0x4) && (opcode != 0x5))
	{
		ALUSrc = 1;
	}
	else
	{
		ALUSrc = 0;
	}
	if(opcode == 0x23) //lw
	{
		MemtoReg = 1;
	}
	else
	{
		MemtoReg = 0;
	}
	if((opcode != 0x2B) && (opcode != 0x4) && (opcode != 0x5) && (opcode != 0x2) && (opcode != 0x3))
	{
		RegWrite = 1;
	}
	else
	{
		RegWrite = 0;
	}
	if(opcode == 0x23) //lw
	{
		MemRead = 1;
	}
	else
	{
		MemRead = 0;
	}
	if(opcode == 0x2B) //sw
	{
		MemWrite = 1;
	}
	else
	{
		MemWrite = 0;
	}
	if((opcode == 0x2) || (opcode == 0x3))
	{
		PCSrc1 = 1;
	}
	else
	{
		PCSrc1 = 0;
	}
	if(((opcode == 0x4) && (Reg[rs] == Reg[rt])) || ((opcode == 0x5) && (Reg[rs] != Reg[rt])))
	{
		PCSrc2 = 1;
	}
	else
	{
		PCSrc2 = 0;
	}
}

void
Memory()
{
	if(MemRead == 1)
	{
		res = M[res];
		MA++;
	}
	if(MemWrite == 1)
	{
		M[Index] = res;
		printf("M[Index] = %d\n", M[Index]);
		MA++;
	}
	
}
	

void
WriteBack()
{
	if(RegDst == 1)
	{
		if(rd != 0)
		{
			if((opcode == 0x0) && (funct == 0x9))
			{
			}
			else
			{
				Reg[rd] = res;
			}
		}
	}
	else
	{
		if(rt != 0)
		{
			Reg[rt] = res;
		}
	}
	
	if((opcode == 0x2) || (opcode == 0x3))
	{
		pc = JumpAddr;
	}
	else if(PCSrc2 == 1)
	{
		pc = pc + 4 + BranchAddr;
		TB++;
	}
	else if(((opcode == 0x0) && (funct == 0x8)) || ((opcode == 0x0) && (funct == 0x9)))
	{
		pc = res;
	}
	else
	{
		pc = pc + 4;
	}
}

void
Fetch()
{
	inst = M[(pc/4)];
}

void
Decode()
{
	opcode = (inst & 0xFC000000) >> 26;
	// R type
	if(opcode == 0x0)
	{
		rs = (inst & 0x3E00000) >> 21;
		rt = (inst & 0x1F0000) >> 16;
		rd = (inst & 0xF800) >> 11;
		shamt = (inst & 0x7C0) >> 6;
		funct = (inst & 0X3F);
		printf("0x%08X (opcode : 0x%X, rs : %d, rt : %d, rd : %d, shamt : 0x%X, funct : 0x%X)\n", inst, opcode, rs, rt, rd, shamt, funct);
		Rtype++;
	}
	// J type
	else if(opcode == 0x2 || opcode == 0x3)
	{
		address = (inst & 0x3FFFFFF);
		JumpAddr = pc & 0xF0000000 | address << 2;
		printf("0x%08X (opcode : 0x%X, address : 0x%X, JumpAddr : 0x%X)\n", inst, opcode, address, JumpAddr);
		Jtype++;
	}
	// I type
	else
	{
		rs = (inst & 0x3E00000) >> 21;
		rt = (inst & 0x1F0000) >> 16;
		immediate = inst & 0xFFFF;
		BranchAddr = SignExt(immediate) << 2;
		printf("0x%08X (opcode : 0x%X, rs : %d, rt : %d, immediate : %d, BranchAddr : 0x%X)\n", inst, opcode, rs, rt, immediate, BranchAddr);
		Itype++;
	}
}

int
SignExt(int inst)
{
	if((inst & 0xFFFF) >= 0x8000)
	{
		SignExtImm = 0xFFFF0000 | (inst & 0xFFFF);
	}
	else
	{
		SignExtImm = inst & 0xFFFF;
	}
	
	return SignExtImm;
}

int
ZeroExt(int inst)
{
	ZeroExtImm = (inst & 0x0000FFFF);
	return ZeroExtImm;
}
