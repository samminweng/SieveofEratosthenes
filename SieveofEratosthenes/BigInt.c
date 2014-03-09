#include "Header.h"

typedef struct node {
	int digit;
	struct node *next;
	struct node *prev;
} DIGITS;

/*
Check the range of the upper limit.
The limits for each data type is shown in here(http://msdn.microsoft.com/en-us/library/s3f49ktz.aspx).
The range of short is 32,767.
The range of int/long variables is 2,147,483,647.
The range of long long variables is 9,223,372,036,854,775,807.
*/
long long checkIntegerRange_Upper(long u_1, long u_2){
	if (u_2 <= u_1){
		return u_1;
	}
	else{
		if (u_2 <= SHRT_MAX)
			return SHRT_MAX;
		else if (u_2 <= INT_MAX)
			return INT_MAX;
		else if (u_2 < _I64_MAX)
			return _I64_MAX;
	}
}


/*=============================================================*/
/*Get the length of the linked list.*/
/*=============================================================*/
int lengthOf(DIGITS *number){
	int length;
	length = 0;
	while (number != NULL)
	{
		length++;
		number = number->next;
	}
	return length;
}
/*=============================================================*/
/*Return the end DIGITS of the linked list.*/
/*=============================================================*/
DIGITS* moveToEnd(DIGITS *head){
	DIGITS *end;
	end = head;
	while (end->next != NULL){
		end = end->next;
	}
	return end;
}

/*=============================================================*/
/*Convert the linked DIGITS list and display the number.*/
/*=============================================================*/
void printNumber(DIGITS *head){
	//Print out the number backward.
	DIGITS *ptr;
	ptr = head;
	while (ptr != NULL){
		printf("%d", ptr->digit);
		ptr = ptr->next;
	}
	printf("\n");
}

/*=============================================================*/
/*Free the linked list.*/
/*=============================================================*/
void freeNodes(DIGITS *number){
	DIGITS *ptr;
	DIGITS *next;
	ptr = number;
	while (ptr != NULL){
		next = ptr->next;
		free(ptr);
		ptr = next;
	}

}

/*=============================================================*/
/*Create a new DIGITS with the DIGITS and add it to the head DIGITSs.*/
/*=============================================================*/
DIGITS* addDigit(DIGITS *head, int digit){
	DIGITS *prev = NULL;
	DIGITS *ptr = NULL;

	ptr = (DIGITS*)malloc(sizeof(DIGITS));
	ptr->digit = digit;
	ptr->next = NULL;

	if (head == NULL){
		ptr->prev = NULL;
		head = ptr;
	}
	else{
		//Move to the end DIGITS.
		prev = moveToEnd(head);
		prev->next = ptr;
		ptr->prev = prev;
	}

	return head;
}

/*=============================================================*/
/*Create a new DIGITS with the DIGITS and add it to the head DIGITSs.*/
/*=============================================================*/
DIGITS* insertDigit(DIGITS *head, int digit){
	DIGITS *ptr;

	ptr = (DIGITS *)malloc(sizeof(DIGITS));
	ptr->digit = digit;
	ptr->prev = NULL;

	if (head == NULL){
		ptr->next = NULL;
		head = ptr;
	}
	else{
		head->prev = ptr;
		ptr->next = head;
	}
	return ptr;
}

/*=============================================================*/
/*Parse the number and return the linked list.*/
/*=============================================================*/
DIGITS* parseNumber(char *input){
	int pos, start;
	DIGITS *head;
	head = NULL;
	pos = 0;
	start = pos;
	while (pos < strlen(input) && isdigit(input[pos])){
		int digit = input[pos] - '0';
		head = addDigit(head, digit);
		pos++;
	}

	return head;
}



/*=============================================================*/
/*Add two numbers and return the summation.*/
/*=============================================================*/
DIGITS* add(DIGITS *p, DIGITS *q){
	int sum, remainder, len;
	DIGITS *head, *big, *small;

	head = NULL;
	big = moveToEnd(p);
	small = moveToEnd(q);

	remainder = 0;
	len = 1;

	while (big != NULL){
		if (small != NULL){
			sum = big->digit + small->digit + remainder;
			small = small->prev;
		}
		else{
			sum = big->digit + remainder;
		}

		if (sum > 9){
			sum = 0;
			remainder = 1;
		}
		else{
			remainder = 0;
		}
		head = insertDigit(head, sum);
		big = big->prev;
	}

	if (remainder == 1){
		head = insertDigit(head, 1);
	}

	freeNodes(p);
	freeNodes(q);
	return head;
}

/*=============================================================*/
/*Compare p and q and return the result.
if p > q, then return +1.
if p < q, then return -1.
if p = q, then return 0.
*/
/*=============================================================*/
int compare(DIGITS *p, DIGITS *q){
	int p_len, q_len;
	DIGITS *p_ptr, *q_ptr;

	//Check if p/q is infinity. 
	if (q == NULL){
		return -1;
	}

	if (p == NULL){
		return 1;
	}
	
	p_len = lengthOf(p);
	q_len = lengthOf(q);
	if (p_len > q_len){
		return 1;
	}
	else if (p_len < q_len){
		return -1;
	}

	//Compare each DIGITS of these two numbers.
	p_ptr = &p;
	q_ptr = &q;
	while (p_ptr != NULL){
		int p_DIGITS = p_ptr->digit;
		int q_DIGITS = q_ptr->digit;
		if (p_DIGITS  > q_DIGITS)
			return 1;
		else if (p_DIGITS < q_DIGITS)
			return -1;
		else{
			p_ptr = p_ptr->next;
			q_ptr = q_ptr->next;
		}
	}

	return 0;
}

int convertToInt(DIGITS *number){
	DIGITS *ptr;
	int len, result;
	ptr = number;

	len = lengthOf(ptr);
	result = 0;
	while (ptr != 0){
		if (len > 1)
			result += pow(10, len - 1)*(ptr->digit);
		else
			result += ptr->digit;
		len--;
		ptr = ptr->next;
	}

	/*printf("%d", result);*/
	return result;
}

//int checkIntegerRange(DIGITS *number){
//	int r_type;
//
//	DIGITS *range;
//	char r[200];
//	//The maximum of int.		
//	sprintf_s(r, 200, "%d", INT_MAX);
//	range = parseNumber(r);
//	//Check if the number holds the limits of integer data types, i.e. int, long long and infinity.
//	if (compare(number, range) < 0){
//		return 0;
//	}
//	else{
//		//The maximum of long long.
//		sprintf_s(r, 200, "%lld", _I64_MAX);
//		range = parseNumber(r);
//		if (compare(number, range) < 0){
//			return 1;
//		}
//		else{
//			//The inifinity.
//			return 2;
//		}
//	}
//}


