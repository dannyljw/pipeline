#include<stdio.h>
#include<string.h>
#define _CRT_SECURE_NO_WARNING
#define j 0x2
#define jal 0x3
#define jr 0x08
#define add 0x20
#define addi 0x8
#define addiu 0x9
#define addu 0x21
#define and 0x24
#define andi 0xc
#define beq 0x4
#define bne 0x5
#define lui 0xf
#define lw 0x23
#define nor 0x27
#define or 0x25
#define ori 0xd
#define slt 0x2a
#define slti 0xa
#define sltiu 0xb
#define sltu 0x2b
#define sll 0x00
#define srl 0x02
#define sw 0x2b
#define sub 0x22
#define subu 0x23

int memory[0x100000];
int r[32] = {0, };
int pc=0;
int ninst,nexe,nmem,nreg,nbranch,nnotbranch,njump=0;
struct ifid{
	int inst;
	int pc;
}ifid;

struct idex{
	int inst;
	int pc;
	int opcode, rs,rt,rd,immediate,address,shamt,funct, signextimm;
	int jumpaddr,branchaddr,zeroextimm;
	int regdest,alusrc,memtoreg,regwrite,memread,memwrite,pcsrc1,pcsrc2;
	int v1,v2,temp,word;
}idex;

struct exmem{
	int inst;
	int pc;
	int opcode, rs,rt,rd,immediate,address,shamt,funct, signextimm;
	int jumpaddr,branchaddr,zeroextimm;
	int regdest,alusrc,memtoreg,regwrite,memread,memwrite,pcsrc1,pcsrc2;
	int v1,v2,temp,word;
	int writereg,memresult;
}exmem;

struct memwb{
	int inst;
	int pc;
	int opcode, rs,rt,rd,immediate,address,shamt,funct, signextimm;
	int jumpaddr,branchaddr,zeroextimm;
	int regdest,alusrc,memtoreg,regwrite,memread,memwrite,pcsrc1,pcsrc2;
	int v1,v2,temp,word;
	int memresult,writereg;
}memwb;

struct ifid ifid_latch[2];
struct idex idex_latch[2];
struct memwb memwb_latch[2];
struct exmem exmem_latch[2];
/*
int signextimm(int b){
	int a;
	if(b>>15 ==1)
		a=0xffff0000 | b;
	else
		a=0x0000ffff & b;
	return a;
}

*/

void fetch(){
	ifid_latch[0].inst=memory[pc/4];
	ifid_latch[0].pc=pc;
   pc=pc+4;
	//	pc=pc+4;
//0		printf("pc? : %x\n", ifid_latch[0].pc);
//0	printf("FETCH \n pc[0x%08x]=0x%08x \n\n\n",ifid_latch[0].pc, ifid_latch[0].inst);
}
void decodelatchupdate(){
	idex_latch[0].pc=ifid_latch[1].pc;
	idex_latch[0].inst=ifid_latch[1].inst;

}

void control_s(){
	idex_latch[0].opcode=((ifid_latch[1].inst & 0xfc000000) >> 26) & 0x3f;

	if(idex_latch[0].opcode==0)idex_latch[0].regdest=1;
	else idex_latch[0].regdest=0;
	if((idex_latch[0].opcode!=0)&&(idex_latch[0].opcode!=beq) &&(idex_latch[0].opcode != bne)) idex_latch[0].alusrc=1;
	else idex_latch[0].alusrc=0;
	if(idex_latch[0].opcode==lw) idex_latch[0].memtoreg=1;
	else idex_latch[0].memtoreg=0;    
	if((idex_latch[0].opcode!=sw) && (idex_latch[0].opcode != beq)&& (idex_latch[0].opcode !=bne)&&(idex_latch[0].opcode !=jr)&&(idex_latch[0].opcode!=j)&&(idex_latch[0].opcode !=jal)) idex_latch[0].regwrite=1;
	else idex_latch[0].regwrite=0;
	if(idex_latch[0].opcode==lw) idex_latch[0].memread=1;
	else idex_latch[0].memread=0;
	if(idex_latch[0].opcode==sw) idex_latch[0].memwrite=1;
	else idex_latch[0].memwrite=0;
	if((idex_latch[0].opcode==j) || (idex_latch[0].opcode ==jal)) idex_latch[0].pcsrc1=1;
	else idex_latch[0].pcsrc1=0;
	if((idex_latch[0].opcode == bne) || (idex_latch[0].opcode == beq)) idex_latch[0].pcsrc2=1;
	else idex_latch[0].pcsrc2=0;



}
void decode(){
//0	printf("DECODE INSTRUCTION *************%x\n\n", ifid_latch[1].inst);
	idex_latch[0].opcode=((ifid_latch[1].inst & 0xfc000000)>> 26)& 0x3f;
//0	printf("opcode %x\n",idex_latch[0].opcode);
	switch(idex_latch[0].opcode){
		case 0x0:
			idex_latch[0].rs=(ifid_latch[1].inst & 0x03e00000)>>21;
			idex_latch[0].rt=(ifid_latch[1].inst & 0x001f0000)>>16;
			idex_latch[0].rd=(ifid_latch[1].inst & 0x0000f800)>>11;
			idex_latch[0].shamt=(ifid_latch[1].inst & 0x00000fc0)>>6;
			idex_latch[0].funct=(ifid_latch[1].inst & 0x0000003f);
		//	idex_latch[0].v1=r[idex_latch[0].rs];
		//	idex_latch[0].v2=r[idex_latch[0].rt];
//0			printf("DECODE R-TYPE\n rs : %d rt: %d rd : %d shamt : %d funct : %d v1: %d v2: %d\n",idex_latch[0].rs, idex_latch[0].rt, idex_latch[0].rd, idex_latch[0].shamt, idex_latch[0].funct, idex_latch[0].v1, idex_latch[0].v2);
		break;


		case j: 
			idex_latch[0].address=(ifid_latch[1].inst & 0x03ffffff);
//0			printf("DECODE J-TYPE: %d\n",idex_latch[0].address);
		break;
		case jal:
			idex_latch[0].address=(ifid_latch[1].inst & 0x03ffffff);
//0			printf("DECODE JAL-TYPE: %d\n", idex_latch[0].address);
		break;
		default : 
			idex_latch[0].rs=(ifid_latch[1].inst & 0x03e00000)>>21;
			idex_latch[0].rt=(ifid_latch[1].inst & 0x001f0000)>>16;
			idex_latch[0].immediate=(ifid_latch[1].inst & 0x0000ffff);
		//	idex_latch[0].v1=r[idex_latch[0].rs];
		//	idex_latch[0].v2=r[idex_latch[0].rt];
//0			printf("DECODE I-TYPE\n rs : %d rt: %d immediate : %x v1: %d v2 : %d", idex_latch[0].rs, idex_latch[0].rt, idex_latch[0].immediate, idex_latch[0].v1, idex_latch[0].v2);
		break;
	}//switch(opcdoe) close



//ADDING SIGNEXTIMMEDIATE
	if((ifid_latch[1].inst & 0x0000ffff) >= 0x8000)
		idex_latch[0].signextimm= 0xffff0000 | (ifid_latch[1].inst & 0x0000ffff);
	else
		idex_latch[0].signextimm=(ifid_latch[1].inst & 0x0000ffff);
//FINISHING SIGNEXTIMMEDIATE

//ADDING JUMPADDRESS
	idex_latch[0].jumpaddr=(ifid_latch[1].pc & 0xf0000000) | (idex_latch[0].address <<2);	
//CLISING JUMPADDRESS

	decodelatchupdate();

//jumpaddrasdfas;ldkfja;slkdfj;lkajsd;lkfas;lkdfj;laksjdf;lkajsdf
	if(idex_latch[0].opcode ==j){
		pc=idex_latch[0].jumpaddr;
		printf("JUMP pc : %d",idex_latch[0].jumpaddr);	
		nexe++;
		njump++;
		}
	else if (idex_latch[0].opcode==jal)
		{	
		r[31]=idex_latch[0].pc+8;
		pc=idex_latch[0].jumpaddr;
		printf("JAL R[31]=%d,pc=%d\n",r[31], idex_latch[0].jumpaddr);
		nexe++;
		njump++;
		}

	control_s();

/*	printf("regdest %d\n",idex_latch[0].regdest);printf("alusrc %d \n",idex_latch[0].alusrc);printf("memtoreg %d\n",idex_latch[0].memtoreg);printf("memread %d\n",idex_latch[0].memread);printf("regwrite %d\n",idex_latch[0].regwrite);printf("memwrite : %d\n",idex_latch[0].memwrite);printf("pcsrc1 %d\n",idex_latch[0].pcsrc1);printf("pcsrc2 %d\n",idex_latch[0].pcsrc2);*/
	

	

//0printf("\n\n\n");
}//decode() close




void execution(){
//0printf("EXECUTION ^^^^^^^^^^MEMTOREG %d\n", exmem_latch[0].memtoreg);
	int v1;
	int v2;

	idex_latch[0].v1=r[idex_latch[0].rs];
    idex_latch[0].v2=r[idex_latch[0].rt];
    
   

//	idex_latch[1].branchaddr=idex_latch[1].signextimm<<2;
//DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
	idex_latch[1].zeroextimm = (idex_latch[1].inst & 0x0000ffff);

	if((idex_latch[1].rs!=0)&&(idex_latch[1].rs==exmem_latch[0].writereg)&&(exmem_latch[0].regwrite)) 
	{
		idex_latch[1].v1  = exmem_latch[0].temp;
		printf("DATA DEPENDENCY WORKING distance (1) r[rs]\n");
	}

	else if((idex_latch[1].rs!=0) && (idex_latch[1].rs==memwb_latch[0].writereg) && (memwb_latch[0].regwrite))
	{	
		if(memwb_latch[0].memtoreg==1)
			{
			idex_latch[1].v1 = memwb_latch[0].memresult;
			printf("DATA DEPENDENCY WORKING distance (2) LORD WORD r[rs]\n");
			}  
        else
			{
			idex_latch[1].v1 = memwb_latch[0].temp;
			printf("DATA DEPENDENCY WORKING distance (2)  r[rs]\n");
			}
	}

/*else 
{
	idex_latch[0].v1=r[idex_latch[0].rs];
	idex_latch[0].v2=r[idex_latch[0].rt];


}*/
 
	if((idex_latch[1].rt!=0)&&(idex_latch[1].rt==exmem_latch[0].writereg)&&(exmem_latch[0].regwrite))
	{
		idex_latch[1].v2 = exmem_latch[0].temp;
		printf("DATA DEPENDENCY WORKING distance (1) rt\n");
	}

	else if((idex_latch[1].rt!=0) && (idex_latch[1].rt==memwb_latch[0].writereg) && (memwb_latch[0].regwrite))
	{
		if(memwb_latch[1].memtoreg==1)
		{
			idex_latch[1].v2 = memwb_latch[0].memresult;
			printf("DATA DEPENDENCY WORKING distance (2) LORD WORD r[rt]\n");
		}
	else
		{
			idex_latch[1].v2 = memwb_latch[0].temp;
			printf("DATA DEPENDENCY WORKING distance (2) r[rt]\n");
		}
	}
//DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDd

	v1=idex_latch[1].v1;
	v2=idex_latch[1].v2;
//0	printf("VALUE CHECKING \n v1 %d v2 %d\n", idex_latch[1].v1, idex_latch[1].v2);
//0    switch(idex_latch[1].opcode)/////////////////

	if(idex_latch[1].regdest==1){

		switch(idex_latch[1].funct){
			case add:
				exmem_latch[0].temp=v1+v2;
				printf("add execution\n");
				nexe++;
				break;
			case addu:
				exmem_latch[0].temp= v1+v2;
//0				printf("v1 %d vv2 %d  exmem_latch[0].temp %d \n", v1, v2, exmem_latch[0].temp);
				printf("addu execution \n");
				nexe++;
				break;
			case and: 
				exmem_latch[0].temp= v1&v2;
				printf("and execution \n");
				nexe++;
				break;
			case jr:
				pc=v1;
				printf("v1 %d\n",v1);
				memset(&ifid_latch[0], 0 ,sizeof(ifid));
				memset(&idex_latch[0], 0 ,sizeof(idex));
				printf("jr 작동한다 pc: %d\n",pc);
				break;
			case nor :
				exmem_latch[0].temp!=(v1|v2);
				printf("nor execution \n");
				nexe++;
				break;
			case or:
				exmem_latch[0].temp=(v1|v2);
				printf("or execution \n");
				nexe++;
				break;
			case slt:
				exmem_latch[0].temp=((v1<v2) ? 1:0);
				printf("slt execution \n");
				nexe++;
				break;
			case sltu:
				exmem_latch[0].temp=((v1<v2) ? 1:0);
				printf("sltu execution \n");
				nexe++;
				break;
			case sll:
				exmem_latch[0].temp=v2 << idex_latch[1].shamt;	
				nexe++;
			if(idex_latch[1].inst == 0x00000000)
			{	
				printf("nop instruction execution \n");
			}	
				break;
			case srl:
				exmem_latch[0].temp= v2 >> idex_latch[1].shamt;
				printf("srl execution \n");
				nexe++;
				break;
			case sub:
				exmem_latch[0].temp= v1 - v2;
				printf("sub execution \n");
				nexe++;
				break;
			case subu:
				exmem_latch[0].temp = v1 - v2;
				printf("subu execution \n");
				nexe++;
				break;
			default:
				break;
		}//switch(idex_latch[1].funct) close
//		exmem_latch[0].writereg=idex_latch[1].rd;
	}//if(idex_latch[1].regdest==1) close

	else if (idex_latch[1].alusrc ==1){
		switch(idex_latch[1].opcode){
			case addi:
				exmem_latch[0].temp=(v1 + idex_latch[1].signextimm);
				printf("addi execution\n");
				nexe++;
				break;
			case sw:
				exmem_latch[0].temp=v2;
				exmem_latch[0].word= v1+ idex_latch[1].signextimm;
				printf("SW execution,v1 %d  signextimm : %d\n",v1, idex_latch[1].signextimm);
				nexe++;
	/*	idex_latch[1].word=(v1+signextimm(idex_latch[1].immediate));
				printf("sw execution \n");*/
				break;
			case lw:
				exmem_latch[0].word=v1+ idex_latch[1].signextimm;
//				printf("==========\n\nlw test : %d\n\n\n", exmem_latch[0].temp);
				printf("LW execution,v1 %d  signextimm : %d\n",v1, idex_latch[1].signextimm);
				nexe++;
				break;
			case addiu:
				exmem_latch[0].temp=v1+idex_latch[1].signextimm;
				printf("addiu execution \n");
				nexe++;
				break;
			case andi:
				exmem_latch[0].temp=v1 & idex_latch[1].zeroextimm;
				printf("lui execution \n");
				nexe++;
				break;
			case lui:
				exmem_latch[0].temp= idex_latch[1].immediate << 16;
				printf("lui execution \n");
				nexe++;
				break;
			case ori:
				exmem_latch[0].temp= v1 | idex_latch[1].zeroextimm;
				printf("ori execution \n");
				nexe++;
				break;
			case slti:
				exmem_latch[0].temp=(v1 < (idex_latch[1].signextimm) ? 1:0);
				printf("slti execution \n");
				nexe++;
				break;
			case sltiu:
				exmem_latch[0].temp=((v1 < idex_latch[1].signextimm)? 1:0);
				printf("sltiu execution \n");
				nexe++;
				break;
			default:
				break;
		}//switch(idex_latch[1].opcode) close
//		exmem_latch[0].writereg=idex_latch[1].rt;
	}//else if (idex_latch[1].alusrc==1) close
	else if (idex_latch[1].pcsrc2==1){
	 	switch(idex_latch[1].opcode){
			case beq:
				if(v1 == v2)
				{
                    idex_latch[1].branchaddr=idex_latch[1].signextimm<<2;//
					pc=idex_latch[1].pc + idex_latch[1].branchaddr +4;
					memset(&ifid_latch[0],0,sizeof(ifid));
					memset(&idex_latch[0],0,sizeof(idex));
					printf("BEQ pc : %d npc %d branchaddr : %x\n", pc,idex_latch[1].pc, idex_latch[1].branchaddr);				
					nexe++;
					nbranch++;
				}
				else 
				{
					nnotbranch++;
				//	pc=idex_latch[1].pc+4;
					printf("BEQ else pc : %d npc %d  branchaddr : %x \n", pc,idex_latch[1].pc, idex_latch[1].branchaddr);
				}
			break;
			case bne:
				if(v1 != v2)
				{
                    idex_latch[1].branchaddr=idex_latch[1].signextimm<<2;
					pc=idex_latch[1].pc+idex_latch[1].branchaddr +4;
					memset(&ifid_latch[0],0,sizeof(ifid));
					memset(&idex_latch[0],0,sizeof(idex));
					printf("BNE right pc : %d npc %d  branchaddr : %x \n", pc,idex_latch[1].pc, idex_latch[1].branchaddr);
					nexe++;
					nbranch++;

				}
				else
				{
				//	pc= idex_latch[1].pc+4;
					printf("BNE else pc : %d npc %d  branchaddr : %x \n", pc,idex_latch[1].pc, idex_latch[1].branchaddr);
					nexe++;
					nnotbranch++;
				}
		
				break;
			default:
				break;
		}//switch(idex_latch[1].opcode) close
//		exmem_latch[0].writereg=idex_latch[1].rt;
} //else if (idex_latch[1].pcsrc2==1) close
	if(idex_latch[1].regdest==1){
		exmem_latch[0].writereg = idex_latch[1].rd;
//0		printf("writereg : %d 값 들어가냐 ㅠ\n",exmem_latch[0].writereg);
	}
	else {
		exmem_latch[0].writereg = idex_latch[1].rt;
//0		printf("writereg: %d 값 들어가냐 ㅠ\n", exmem_latch[0].writereg);
	}
//updating next using latch//exemlatch[0]=idex_latch[1]
	exmem_latch[0].pc=idex_latch[1].pc;
	exmem_latch[0].inst=idex_latch[1].inst;
	exmem_latch[0].opcode=idex_latch[1].opcode;
	exmem_latch[0].funct=idex_latch[1].funct;
	exmem_latch[0].memtoreg=idex_latch[1].memtoreg;
	exmem_latch[0].regwrite=idex_latch[1].regwrite;
	exmem_latch[0].memread=idex_latch[1].memread;
	exmem_latch[0].memwrite=idex_latch[1].memwrite;
	exmem_latch[0].pcsrc1=idex_latch[1].pcsrc1;
	exmem_latch[0].pcsrc2=idex_latch[1].pcsrc2;
	exmem_latch[0].v2= v2;
}//void execution close



void memoryacc(){
	 if (exmem_latch[1].memwrite ==1)
	{
		memory[exmem_latch[1].word/4]=exmem_latch[1].v2;
		printf("MEMORYACCES \nSSSWWW 가 잘 진행되고 있음 \n");
		printf("memory[0x%08x]= 0x%08x \n", exmem_latch[1].word/4, exmem_latch[1].v2);
		nmem++;
	}

	 if(exmem_latch[1].memread ==1)
	{
		exmem_latch[1].memresult=memory[exmem_latch[1].word/4];
		printf("MEMORYACCESS \nLLLWWW 가 잘 진행하고 있음 \n");
		printf(" 0x%08x=memory[0x%08x]\n",exmem_latch[1].memresult,exmem_latch[1].word/4);
		nmem++;
	}

	memwb_latch[0].inst=exmem_latch[1].inst;
	memwb_latch[0].opcode=exmem_latch[1].opcode;
	memwb_latch[0].memresult=exmem_latch[1].memresult;
	memwb_latch[0].funct=exmem_latch[1].funct;	
	memwb_latch[0].memtoreg=exmem_latch[1].memtoreg;
	memwb_latch[0].regwrite=exmem_latch[1].regwrite;
	memwb_latch[0].v1=exmem_latch[1].v1;
	memwb_latch[0].v2=exmem_latch[1].v2;
	memwb_latch[0].temp=exmem_latch[1].temp;
	memwb_latch[0].writereg=exmem_latch[1].writereg;
}

void writeback(){

	if(memwb_latch[1].regwrite ==1)
	{	
		if(memwb_latch[1].memtoreg ==1)
		{
//0			printf("---------------(M)write back test r[%d] = %d pc %d\n", memwb_latch[1].writereg, memwb_latch[1].memresult, pc);
			r[memwb_latch[1].writereg]=memwb_latch[1].memresult;
			printf("WRITEBACK FUNCTION (1)\nr[%d]=%d\n", memwb_latch[1].writereg, memwb_latch[1].memresult);
//			printf("(M)WRITE BACK \n");
		}//if(memwb_latch[1].memtoreg==1)
		else
		{
			printf("\n\n----------------------write back test r[%d] = %d pc %x\n", memwb_latch[1].writereg, memwb_latch[1].temp, pc);
			if(memwb_latch[1].writereg !=0)
				r[memwb_latch[1].writereg]=memwb_latch[1].temp;//////////////////
			else r[memwb_latch[1].writereg]=0;
				printf("WRITEBACK FUNCTION (2)\nr[%d]=%d", memwb_latch[1].writereg, memwb_latch[1].temp);
//			printf("WRITE BACK \n");
		}
		nreg++;
	}//if(memwb_latch[1].regwrite==1)close	
	else{printf("NOT WRITEBACK FUNCTION\n");}
}



void main(){
	FILE *fp;
	int cycle=0;
	int data=0;
	int i=0;
	int ret=0;
	unsigned inst_temp;
	unsigned temp;
	int inst=0;
	
	r[31]=0xffffffff;
	r[29]=0x100000;
	
	fp=fopen("test_prog/fib.bin","rb");
	while(1){
		ret=fread(&data,sizeof(int),1,fp);
		if(ret==0){printf("not reading fucking file\n"); break;}
		inst=((data & 0xff)<<24) | (((data & 0xff000000)>>24)&0xff);
		inst_temp=((data & 0xff00)<<8) | (((data & 0xff0000)>> 8) & 0xff00);
		inst = (inst | inst_temp);
		printf("memory[0x%08x]: 0x%08x\n", i, inst);
		memory[i++]=inst;
	}
	fclose(fp);



	while(1){
	printf("------------------------cycle : %d------------------\n",cycle);
//	printf("pc : %x\n",pc);

    writeback();
    fetch();	
	decode();
	execution();		
	memoryacc();

//	idex_latch[0].v1=r[idex_latch[0].rs];
		//	idex_latch[0].v2=r[idex_latch[0].rt];
	ifid_latch[1]=ifid_latch[0];
	idex_latch[1]=idex_latch[0];
	exmem_latch[1]=exmem_latch[0];
	memwb_latch[1]=memwb_latch[0];
	if(pc==0xffffffff)break;

	ninst++;
	cycle++;
	}
printf("r[2] %d r[29]= %d,r[30]=%d, r[31]= %d \n ", r[2], r[29],r[30],r[31]);
printf("Total instruction : %d\n", ninst);
printf("Total memory update: %d \n",nmem);
printf("Total register update: %d \n",nreg);
printf("Total execution: %d\n",nexe);
printf("Total branch : %d\n",nbranch);
printf("Total not branch : %d\n",nnotbranch);
printf("Total Jump: %d\n",njump);
}
