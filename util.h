#ifndef _INC_STDIO
#include <stdio.h>
#endif
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef _STDBOOL
#include <stdbool.h>
#endif
#ifndef _WINDOWS_
#include <windows.h>
#endif
#ifndef _INC_TIME
#include <time.h>
#endif
#pragma warning(disable: 4996)

#ifdef _WIN32
#define WINDOWS true
#define LiNUX false
#define MAC false
#endif
#ifdef __APPLE__
#define WINDOWS false
#define LINUX false
#define MAC true
#endif
#ifdef __MACH__
#define WINDOWS false
#define LINUX false
#define MAC true
#endif
#ifdef __linux__
#define WINDOWS false
#define LINUX true
#define MAC false
#endif

#ifndef __UTIL__
#define __UTIL__

#define BUF_MAX 128
#define FILE_BUFFER_SIZE 10240
#define ERR_EMPTY -4000
#define ERR_MALLOC_FAIL -4001
#define ERR_OUT_OF_RANGE -4002

int _;

char* ERRNO_TO_STR(int i) {
	system("color a");
	printf("\nutil.h : error searching..\n");
	Sleep(1000);
	system("cls");
	system("color f");
	printf("error : ");
	switch (i) {
	case(ERR_EMPTY):
		return "empty";
	case(ERR_MALLOC_FAIL):
		return "malloc fail";
	case(ERR_OUT_OF_RANGE):
		return "out of range";
	default:
		return "not error";
	}
}
void debug(int i) {
	printf(ERRNO_TO_STR(i));
}

typedef void(*voidcallback)(void);
typedef void(*voidfunccallback)(voidcallback);
typedef void(*listcallback)(linkedList);
typedef void(*stackcallback)(stack);
typedef void(*queuecallback)(queue);
typedef void(*stringcallback)(string);
typedef void(*strcallback)(char*);
typedef void(*constcharcallback)(const char*);
typedef void(*twostrcallback)(const char*, const char*);

typedef struct {
	char* value;
	int length;
} string;

typedef struct {
	int size;
	int cursor;
	int* value;
} queue;

typedef struct {
	int size;
	int cursor;
	int* value;
} stack;

typedef struct node {
	struct node* next;
	int value;
} linkedList;

typedef struct {
	voidcallback greet_ko;
	voidcallback greet_en;
	voidcallback greet_jp;
	listcallback list;
	stringcallback string;
	stackcallback stack;
	queuecallback queue;
	voidcallback new_line;
} printer;

typedef struct {
	int length;
	string* values;
} split_result;

#define SWAP(type, a, b) { type temp = a; a = b; b = temp; }

char* read_file(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		return NULL;
	}
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return NULL;
	}
	long filesize = ftell(file);
	if (filesize == -1L) {
		fclose(file);
		return NULL;
	}
	if (filesize > FILE_BUFFER_SIZE) {
		fclose(file);
		return NULL;
	}
	rewind(file);
	char* buffer = (char*)malloc(filesize + 1);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}
	size_t read_size = 0;
	while (read_size < filesize) {
		size_t result = fread(buffer + read_size, 1, filesize - read_size, file);
		if (result == 0) {
			if (ferror(file)) {
				free(buffer);
				fclose(file);
				return NULL;
			}
			break;
		}
		read_size += result;
	}
	buffer[filesize] = '\0';
	fclose(file);
	return buffer;
}
char* get_current_time() {
	time_t now;
	struct tm* timeinfo;
	char* time_str = (char*)malloc(9 * sizeof(char));
	if (time_str == NULL) { return NULL; }
	time(&now);
	timeinfo = localtime(&now);
	sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return time_str;
}

int sizeof_str(char* value) {
	if (value == NULL) { return ERR_EMPTY; }
	return strlen(value);
}
char charAt(string str, int index) {
	return *(str.value + index);
}
string str_create(const char* value) {
	string temp;
	if (value == NULL) {
		temp.value = NULL;
		temp.length = ERR_EMPTY;
		return temp; //제발 오류를 고쳐주시옵소서 c컴파일러의 신이시야-ㅕ~
	}
	temp.length = sizeof_str(value);
	temp.value = (char*)malloc(temp.length + 1);
	if (temp.value == NULL) {
		temp.length = ERR_MALLOC_FAIL;
		return temp;
	}
	strcpy(temp.value, value);
	return temp;
}
string str_create_empty(void) {
	string temp;
	temp.length = 0;
	temp.value = NULL;
	return temp;
}
char* str_add(string s1, string s2) {
	char* result = (char*)malloc(s1.length + s2.length + 1);
	if (result == NULL) { return ERRNO_TO_STR(ERR_MALLOC_FAIL); }
	for (int i = 0; i < s1.length; i++) {
		result[i] = s1.value[i];
	}
	for (int i = 0; i < s2.length; i++) {
		result[(i + s1.length)] = s2.value[i];
	}
	result[s1.length + s2.length] = '\0';
	return result;
}
void str_set(string* s, char* value) {
	if (s == NULL || value == NULL) {
		return;
	}
	s->value = value;
	s->length = strlen(s->value);
}
char* add(char* s1, char* s2) {
	if (s1 == NULL || s2 == NULL) {
		return ERRNO_TO_STR(ERR_EMPTY);
	}
	int len1 = strlen(s1), len2 = strlen(s2);
	char* result = (char*)malloc(len1 + len2 + 1);
	if (result == NULL) { return ERRNO_TO_STR(ERR_MALLOC_FAIL); }
	for (int i = 0; i < len1; i++) {
		result[i] = s1[i];
	}
	for (int i = 0; i < len2; i++) {
		result[(i + len1)] = s2[i];
	}
	result[len1 + len2] = '\0';
	return result;
}
char* str_slice(string str, int start, int end) {
	if (start < 0 || end > str.length || start > end) { return NULL; }
	char* result = (char*)malloc(end - start + 1);
	if (result == NULL) { return NULL; }
	for (int i = 0; i < end - start; i++) { result[i] = str.value[start + i]; }
	result[end - start] = '\0';
	return result;
}
char* slice(char* str, int start, int end) {
	char* result = (char*)malloc(end - start + 1);
	if (result == NULL) { return ERRNO_TO_STR(ERR_MALLOC_FAIL); }
	for (int i = 0; i < end - start; i++) {
		result[i] = str[start + i];
	}
	result[end - start] = '\0';
	return result;
}
bool str_equal(string str1, string str2) {
	if (str1.length != str2.length) { return false; }
	for (int i = 0; i < str1.length; i++) {
		if (str1.value[i] != str2.value[i]) {
			return false;
		}
	}
	return true;
}
bool equal(char* s1, char* s2) {
	if (s1 == NULL || s2 == NULL) {
		return false;
	}
	if (strlen(s1) != strlen(s2)) { return false; }
	for (int i = 0; i < strlen(s1); i++) {
		if (s1[i] != s2[i]) {
			return false;
		}
	}
	return true;
}
bool str_contain(string str1, string str2) {
	if (strstr(str1.value, str2.value) == NULL) {
		return false;
	}
	return true;
}
bool contain(char* s1, char* s2) {
	if (strstr(s1, s2) == NULL) {
		return false;
	}
	return true;
}
int find_char(char* s, char c) {
	for (int i = 0; i < strlen(s); i++) {
		if (*(s + i) == c) {
			return i;
		}
	}
	return -1;
}
int str_find_char(string str, char c) {
	return find_char(str.value, c);
}
bool str_start_with(string str, string start) {
	return equal(str_slice(str, 0, start.length), start.value);
}
bool start_with(char* s, char* start) {
	return str_start_with(str_create(s), str_create(start));
}
bool end_with(const char* s, const char end) {
	return (s[strlen(s)-1] == end);
}
bool str_empty(string str) {
	return (str.value == NULL);
}
split_result str_split(string str, char delimiter) {
	split_result result;
	bool empty = true;
	result.length = 1;
	for (int i = 0; i < str.length; i++) {
		if (str.value[i] == delimiter) {
			result.length++;
			empty = false;
		}
	}
	result.values = (string*)malloc(result.length * sizeof(string));
	if (result.values == NULL) {
		result.length = ERR_MALLOC_FAIL;
		return result;
	}
	int start = 0;
	int index = 0;
	for (int i = 0; i <= str.length; i++) {
		if (i == str.length || str.value[i] == delimiter) {
			char* slice = str_slice(str, start, i);
			if (slice != NULL) {
				result.values[index++] = str_create(slice);
				free(slice);
			}
			start = i + 1;
		}
	}
	if (empty) { result.length = 0; }
	return result;
}
split_result split(char* s, char delimiter) {
	return str_split(str_create(s), delimiter);
}
char* input(char* str) {
	char* result = (char*)malloc(BUF_MAX);
	if (result == NULL) { return ERRNO_TO_STR(ERR_MALLOC_FAIL); }
	printf("%s", str);
	int _ = scanf("%s", result);

	return result;
}
string str_input(char* str) {
	return str_create(input(str));
}
split_result split_result_shift(split_result input, int shift) {
	split_result result;
	if (input.values == NULL || shift < 0 || shift >= input.length) {
		result.length = 0;
		result.values = NULL;
		return result;
	}
	result.length = input.length - shift;
	result.values = (string*)malloc(result.length * sizeof(string));
	if (result.values == NULL) {
		result.length = ERR_MALLOC_FAIL;
		return result;
	}
	for (int i = 0; i < result.length; i++) {
		result.values[i] = input.values[i + shift];
	}

	return result;
}
void split_result_free(split_result result) {
	if (result.values != NULL) {
		free(result.values);
	}
}
string int_to_str(int value) {
	char buffer[12];
	snprintf(buffer, sizeof(buffer), "%d", value);
	string result;
	result.length = strlen(buffer);
	result.value = (char*)malloc(result.length + 1);
	if (result.value != NULL) { strcpy(result.value, buffer); }
	else { result.length = ERR_MALLOC_FAIL; }

	return result;
}
void queue_add(queue* que, int value) {
	if (que == NULL) { return; }
	if (que->cursor == que->size) { return; }
	que->value[que->cursor] = value;
	que->cursor++;
}
int queue_front(queue que) {
	if (que.cursor == 0) { return ERR_EMPTY; }
	return que.value[0];
}
int queue_remove(queue* que) {
	if (que == NULL) { return ERR_EMPTY; }
	if (que->cursor == 0) { return ERR_EMPTY; }
	int result = queue_front(*que);
	int temp;
	for (int i = 0; i < que->size - 1; i++) {
		temp = que->value[i + 1];
		que->value[i + 1] = que->value[i];
		que->value[i] = temp;
	}
	que->cursor--;
	return result;
}
int queue_peek(queue que) {
	if (que.cursor == 0) { return ERR_EMPTY; }
	return que.value[que.cursor - 1];
}
bool queue_empty(queue que) {
	return (que.cursor == 0);
}
void stack_push(stack* stk, int value) {
	if (stk == NULL) { return; }
	if (stk->cursor == stk->size) { return; }
	stk->value[stk->cursor] = value;
	stk->cursor++;
}
int stack_top(stack stk) {
	if (stk.cursor == 0) { return ERR_EMPTY; }
	return stk.value[(stk.cursor - 1)];
}
int stack_pop(stack* stk) {
	if (stk == NULL) { return ERR_EMPTY; }
	if (stk->cursor == 0) { return ERR_EMPTY; }
	int result = stack_top(*stk);
	stk->value[stk->cursor - 1] = 0;
	stk->cursor--;
	return result;
}
bool stack_empty(stack stk) {
	return (stk.cursor == 0);
}
linkedList* list_create(int value) {
	linkedList* newNode = (linkedList*)malloc(sizeof(linkedList));
	if (newNode == NULL) { return list_create(ERR_MALLOC_FAIL); }
	newNode->value = value;
	newNode->next = NULL;
	return newNode;
}
void list_append(linkedList** head, int value) {
	if (head == NULL || *head == NULL) { return; }
	linkedList* newNode = list_create(value);
	if (*head == NULL) {
		*head = newNode;
		return;
	}
	linkedList* current = *head;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = newNode;
}
void list_print(linkedList* head) {
	if (head == NULL) { return; }
	linkedList* current = head;
	while (current->next != NULL) {
		printf("%d -> ", current->value);
		current = current->next;
	}
	printf("%d\n", current->value);
}
void list_update(linkedList* head, int index, int value) {
	if (head == NULL) { return; }
	for (int i = 0; head->next != NULL && i < index; i++) {
		head = head->next;
	}
	head->value = value;
}
int list_at(linkedList* head, int index) {
	if (head == NULL) { return ERR_EMPTY; }
	for (int i = 0; head->next != NULL && i < index; i++) {
		head = head->next;
	}
	return head->value;
}
int list_length(linkedList* head) {
	if (head == NULL) { return ERR_EMPTY; }
	int result = 0;
	while (head != NULL) {
		head = head->next;
		result++;
	}
	return result;
}
void list_delete(linkedList** head, int index) {
	if (head == NULL || *head == NULL) { return; }
	linkedList* temp = *head;
	if (index == 0) {
		*head = temp->next;
		free(temp);
		return;
	}
	for (int i = 0; temp != NULL && i < index - 1; i++) {
		temp = temp->next;
	}
	if (temp == NULL || temp->next == NULL) { return; }
	linkedList* next = temp->next->next;
	free(temp->next);
	temp->next = next;
}
void __printer_ko(void) {
	printf(" ȳ  ϼ   \n");
}
void __printer_en(void) {
	printf("hello\n");
}
void __printer_jp(void) {
	printf("    ˪   \n");
}
void __printer_list(linkedList list) {
	while (list.next != NULL) {
		printf("%d -> ", list.value);
		list = *(list.next);
	}
	printf("%d\n", list.value);
}
void __printer_string(string s) {
	printf("%s\n", s.value);
}
void __printer_stack(stack s) {
	printf("___\n\n");
	for (int i = (s.cursor - 1); i >= 0; i--) {
		printf("%d\n", *(s.value + i));
	}
	printf("___\n");
}
void __printer_queue(queue q) {
	for (int i = 0; i < (q.cursor - 1); i++) {
		printf("%d << ", *(q.value + i));
	}
	printf("%d", queue_peek(q));
}
void __printer_new(void) {
	printf("\n");
}
printer getPrinter(void) {
	printer temp;
	temp.greet_en = __printer_en;
	temp.greet_jp = __printer_jp;
	temp.greet_ko = __printer_ko;
	temp.list = __printer_list;
	temp.string = __printer_string;
	temp.stack = __printer_stack;
	temp.queue = __printer_queue;
	temp.new_line = __printer_new;
	return temp;
}

#define STR(identifier, value) string identifier = { value, strlen(value) }
#define QUEUE(identifier, size) queue identifier = { size, 0, (int*)malloc(size*sizeof(int)) }
#define STACK(identifier, size) stack identifier = { size, 0, (int*)malloc(size*sizeof(int)) }
#define LIST(identifier, value) linkedList* identifier = list_create(value)

#define EXEC(command) system(command)
#define MALLOC(type, ptr, size) ptr = (type)malloc(size); if (ptr == NULL) { printf("\nMemory allocation failed\n"); exit(0); }

#endif