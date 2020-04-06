#define _GNU_sOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool isoperator(char* Opcode);
long seperate_R(char* temp);
long seperate_0x(char* temp);
long calculate(char* Opcode, long operand1, long operand2);
long R[10] = {0};

int
main()
{
        FILE *stream;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        char* result;
	char* item[2];
	char* Opcode;
	char* temp1;
	char* temp2;
	long operand1, operand2;
        char **arr = NULL;
        int t = 0;
	int x, y;

	scanf("%d %d", &x, &y);
        arr = (char**)malloc(sizeof(char*)*x);
	for(int i=0; i<x; i++)
        {
               *(arr + i) = (char*)malloc(sizeof(char)*y);
        }

        stream = fopen("input.txt", "r");
	if (stream == NULL)
        {
	        exit(EXIT_FAILURE);
	}
        while ((read = getline(&line, &len, stream)) != -1)
        {
		result = strtok_r(line, " ", &item[0]);
		strcpy(arr[t], result);
		Opcode = arr[t];
	
		if(!strncmp(Opcode, "H", 1))
		{
			return 0;
		}

		result = strtok_r(arr[t], " ", &item[1]);
		result = strtok_r(NULL, " ", &item[0]);
		t++;
		strcpy(arr[t], result);
		temp1 = arr[t];

		result = strtok_r(arr[t], " ", &item[1]);
		result = strtok_r(NULL, " ", &item[0]);
		t++;
		strcpy(arr[t], result);
		temp2 = arr[t];
		

		if(isoperator(Opcode))
		{
			if(!strncmp(temp1, "R", 1))
			{
				if(!strncmp(temp2, "R", 1))
				{
					operand1 = R[seperate_R(temp1)];
					operand2 = R[seperate_R(temp2)];
					R[0] = calculate(Opcode, operand1, operand2);
					printf("R[0] is %ld <- %ld %s %ld\n\n", R[0], operand1, Opcode, operand2);
				}
				else
				{
					operand1 = R[seperate_R(temp1)];
					operand2 = seperate_0x(temp2);
					R[0] = calculate(Opcode, operand1, operand2);
                                        printf("R[0] is %ld <- %ld %s %ld\n\n", R[0], operand1, Opcode, operand2);
				}
			}
			else
			{
				if(!strncmp(temp2, "R", 1))
				{
					operand1 = seperate_0x(temp1);
					operand2 = R[seperate_R(temp2)];
					R[0] = calculate(Opcode, operand1, operand2);
                                        printf("R[0] is %ld <- %ld %s %ld\n\n", R[0], operand1, Opcode, operand2);
				}
				else
				{
					operand1 = seperate_0x(temp1);
					operand2 = seperate_0x(temp2);
					R[0] = calculate(Opcode, operand1, operand2);
                                        printf("R[0] is %ld <- %ld %s %ld\n\n", R[0], operand1, Opcode, operand2);
				}
			}
		}
		else
		{
			if(!strcmp(Opcode, "M"))
			{
				if(!strncmp(temp2, "R", 1))
				{
					R[seperate_R(temp1)] = R[seperate_R(temp2)];
					printf("R[%ld] becomes %ld\n\n", seperate_R(temp1), R[seperate_R(temp2)]);
				}
				else
				{
					operand2 = seperate_0x(temp2);
					R[seperate_R(temp1)] = operand2;
					printf("R[%ld] becomes %ld\n\n", seperate_R(temp1), operand2);
				}
			}
			else
			{
				printf("Error\n");
			}
		}
		t++;
	}
	free(line);
	fclose(stream);

	return 0;
}

long
calculate(char* Opcode, long operand1, long operand2)
{
	long end;
	switch(*Opcode){
	case '+':
		end = operand1 + operand2;
		return end;
		break;
        case '-':
                end = operand1 - operand2;
                return end;
                break;
        case '*':
                end = operand1 * operand2;
                return end;
                break;
        case '/':
		if(operand2 == 0)
		{
			printf("The Denominator cannot be 0.\n\n");
			return R[0];
			break;
		}
		else
                {
			end = operand1 + operand2;
                	return end;
                	break;
		}
	default :
		printf("Wrong Opcode used.\n\n");
		break;
	}
}

long
seperate_0x(char* temp)
{
	long num;
	num = strtol(temp, NULL, 16);
	return num;
}

long
seperate_R(char* temp)
{
	int index;
	char* a;
	a = temp + 1;
	index = atoi(a);

	return index;
}

bool
isoperator(char* Opcode)
{
        if(*Opcode == '+' || *Opcode == '-' || *Opcode == '*' || *Opcode == '/')
                return true;
        else
                return false;
}

