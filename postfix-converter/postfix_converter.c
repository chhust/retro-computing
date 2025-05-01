// This code is meant as a first experiment with one part of an implementation
// of a programming language: it would analyze terms and calculate their
// outcome, including a stub variable handling.

// Infix to postfix conversion and calculation:
// (, ), *, /, +, -; one-digit numbers, variables a-z
//
// ISSUES: no ^ operator, no [] brackets
//         stack implementation via char => only small numbers allowed
//         char variables for integer numbers => strange conversions
//         stack functions return TRUE/FALSE flags that are not used
//         no syntax checks for input terms (only division by zero check)
//         static variable array needs much memory but is easy and fast
//         no negative numbers, no two- or more-digit numbers

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE  1

const int STACK_SIZE = 50;
char stack[50];
int stack_top = -1;

int is_empty();													// stack implementation
int is_full();
int peek(char*);												// stack would better be a struct
int pop(char*);													// from int or float and a flag
int push(char);													// to show if it's operator or operand

int is_bracket(char);											// checks for input types
int is_character(char);
int is_number(char);
int is_operator(char);

int calculate_result(char[]);									// calculate result from postfix term
void generate_postfix(char[], char[]);							// converts infix to postfix term
int precedence(char);											// */ versus +-
void prepare_input(char[], char[]);								// clean up input string

int main() {
	char input[100];
	char infix[100];
	char postfix[100];

	printf("Please enter infix term: ");
	fgets(input, sizeof(input), stdin);
	prepare_input(input, infix);
	generate_postfix(infix, postfix);
	printf("For the infix term \'%s\', the postfix notation is \'%s\'.\n", infix, postfix);
	printf("The result is %d.\n", calculate_result(postfix));
	return 0;
}

int calculate_result(char term[]) {
	char operand1, operand2;							        // operands pushed from stack
	char result;												// end result
	int i;
	int value;													// value of variable
	char table[26][2];											// stores variables a-z: value in [][1],
	for(int j=0; j < 26; j++) table[j][0] = FALSE;				// flag (known/unknown) in [][0]

	for(i = 0; i < strlen(term); i++) {
		if(is_number(term[i])) push(term[i]);					// push numbers to stack
		else {
			if(is_character(term[i])) {							// variable?
				if(! table[term[i] - 'a'][0]) {					// if not yet defined: get input,
					printf("Please enter value for variable %c: ", term[i]);
					scanf("%d", &value);
					table[term[i] - 'a'][1] = value + '0';		// store as ASCII), mark as defined
					table[term[i] - 'a'][0] = TRUE;
				}
				push(table[term[i] - 'a'][1]);
			} else {											// neither bracket nor operand: operator
				pop(&operand1);									// get two values from stack
				pop(&operand2);
				operand1 -= 48;									// convert ASCII to number values
				operand2 -= 48;
				switch(term[i]) {
					case '*':
						push((operand2 * operand1) + 48);		// calculate and push
						break;
					case '/':
						if(!operand1) {
							puts("Error: division by zero.");
							exit(0);
						}
						push((operand2 / operand1) + 48);
						break;
					case '+':
						push((operand2 + operand1) + 48);
						break;
					case '-':
						push((operand2 - operand1) + 48);
						break;
				}
			}
		}
	}
	peek(&result);										        // get result
	return(result - 48);										// convert ASCII to number
}

void generate_postfix(char infix[], char postfix[]) {
	char stack_element;
	int i, j = 0;

	for(i = 0; i < strlen(infix); i++) {					    //       go through input string
		if(is_character(infix[i]) || is_number(infix[i])) {		// 1.    copy operands to postfix term
			postfix[j++] = infix[i];
		} else {												// 2.    next character is an operator
			if(infix[i] == '(')	push(infix[i]);			        // 2.1   push '(' to stack
			if(infix[i] == ')') {								// 2.2   empty stack until next '('
				while(1) {
					pop(&stack_element);
					if(stack_element == '(') break;
					postfix[j++] = stack_element;
				}
			} else {											// 2.3   other operators: */+-
				while(1) {										// 2.3.1 check precedence
					if((!peek(&stack_element) || stack_element == '(') || (precedence(stack_element) < precedence(infix[i]))) break;
					pop(&stack_element);				        //       pop stack according to precedence
					postfix[j++] = stack_element;				//       and store in postfix string
				}
				push(infix[i]);							        // 2.3.2 push next element to stack
			}
		}
	}
	while(pop(&stack_element))							        // 3.    empty stack
		if(stack_element != '(') postfix[j++] = stack_element;
	postfix[j] = '\0';											// mark end of string
}

int is_bracket(char character) {							    // not sure I actually needed this
	switch(character) {
		case '(':
			return 2;
		case ')':
			return 1;
		default:
			return 0;
	}
}

int is_character(char character) {								// allows a-z as variable names
	return(character >= 'a' && character <= 'z');
}

int is_empty() {												// stack empty?
	return(stack_top == -1);
}

int is_full() {													// stack full?
	return((stack_top + 1) == STACK_SIZE);
}

int is_number(char character) {									// is currenct character a number?
	return(character >= '0' && character <= '9');
}

int is_operator(char character) {								// is current character a known operator?
	switch(character) {
		case '*':
		case '/':
		case '+':
		case '-':
			return TRUE;
		default:
			return FALSE;
	}
}

int peek(char* element) {										// peek at top element of stack
	if(!is_empty()) {
		*element = stack[stack_top];
		return TRUE;
	} else return FALSE;
}

int pop(char* element) {										// pop element from stack
	if(!is_empty()) {
		*element = stack[stack_top--];
		return TRUE;
	} else return FALSE;
}

int precedence(char operator) {									// determine operator precedence
	switch(operator) {
		case '*':
		case '/':
			return 2;
		default:
			return 1;
	}
}

void prepare_input(char input_string[], char infix_string[]) {	// clean up input: only known elements stay
	int i, j = 0;
	
	for(i = 0; i < strlen(input_string); i++)
		if(is_bracket(input_string[i]) || is_character(input_string[i]) || is_number(input_string[i]) || is_operator(input_string[i])) infix_string[j++] = input_string[i];
	infix_string[j] = '\0';										// mark end of infix string
}

int push(char element) {										// push element to stack
	if(!is_full()) {
		stack[(++stack_top)] = element;
		return TRUE;
	} else return FALSE;
}
