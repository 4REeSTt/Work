#pragma warning(disable : 4996)
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>

#define USER_INTERFACE "Choose task: \n\
    2. Get number of Gererators\n\
    3. Add one more Random number generator\n\
    4. Change N on special Generator\n\
    5. Set delay\n\
    6. Get Sum\n\
    1. Exit\n> "

int value = 0;
int idx = 0;
int Sum = 0;
int Delay = 3;
volatile int PROGRAM_RUNNING = 1;

int Data_of_thread_1 = 1;
int Data_of_thread_2 = 2;
int Data_of_thread_3 = 3;

HANDLE Handle_of_Thread_1 = 0;
HANDLE Handle_of_Thread_2 = 0;
HANDLE Handle_of_Thread_3 = 0;
HANDLE Threads_Handles_Arrray[3];
HANDLE REQUEST_EMPTY;

//We can use mutex as well
CRITICAL_SECTION REQUEST_QUEUE_ACCESS;
CRITICAL_SECTION OUTPUT_ACCESS;



//List of requests
typedef struct chain {
    struct chain* Next;
    
    int choice;
    int value;
    int idx;

}Request;

typedef struct {
    Request* First;
    Request* Last;

}Request_list;
Request_list* Request_queue;

//GRN list
typedef struct node {
    struct node* prev;
    struct node* next;
    int N;

}List;
typedef struct {
    List* tail;
    int size;
}List_tail;
List_tail* global_list;





//1.
int GetNumGRN(List_tail* list) {
    printf("Number of GRN is %d now\n", list->size);
    return list->size;
}

//2.
void Add_GRN(List_tail* list, int N) {
    if (list->tail == NULL) {
        list->tail = (List*)malloc(sizeof(List));

        if (list->tail == NULL) {
            EnterCriticalSection(&OUTPUT_ACCESS);

            printf("Couldn't allocate memory for new N");
            LeaveCriticalSection(&OUTPUT_ACCESS);
            return;
        }
        list->tail->next = NULL;
        list->tail->prev = NULL;
        list->tail->N = N;
        list->size = 1;
        return;
    }
    list->tail->next = (List*)malloc(sizeof(List));

    if (list->tail->next == NULL) {
        printf("Coudn't allocate memory for new N");
        return;
    }
    list->tail->next->N = N;
    List* previous = list->tail;
    list->tail = list->tail->next;

    list->tail->next = NULL;
    list->tail->prev = previous;
    list->size++;
}

//3.
void SetN(int idx, int value, List_tail* list) {
    List* current = list->tail;


    if (list->size - idx < 0) {
        EnterCriticalSection(&OUTPUT_ACCESS);

        printf("Idx is out of range GRN_list\n");
        LeaveCriticalSection(&OUTPUT_ACCESS);
        return;
    }
    if (list->size - idx == 0) {
        list->tail->N = value;
        return;
    }
    list->tail = list->tail->prev;
    SetN(++idx, value, list);
    list->tail = current;
}

//4.
void ChangeDelay(int delay) {
    if (delay < 0) {
        EnterCriticalSection(&OUTPUT_ACCESS);

        printf("Delay can't be less than 0\n");
        LeaveCriticalSection(&OUTPUT_ACCESS);
        return;
    }
    Delay = delay;
}

//5.
int GetSum() {
    EnterCriticalSection(&OUTPUT_ACCESS);

    printf("Sum is %d\n", Sum);
    LeaveCriticalSection(&OUTPUT_ACCESS);
    return Sum;
}
int CalculateSum(List_tail* list) {
    List* current = list->tail;
    int summ = 0;
    if (list->tail->prev == NULL)
        return list->tail->N;

    list->tail = list->tail->prev;
    summ = (rand() % list->tail->N) + CalculateSum(list);
    list->tail = current;
    return summ;
}

void scanProtectedUserInterface(int max, int* choice) {
    do {
        scanf("%d", choice);
        fflush(stdin);
    } while (*choice <= 0 || *choice > max);
}
void scanProtectedInput(int* _value) {
    do {
        scanf("%d", _value);
        fflush(stdin);
    } while (*_value <= 0);
}

DWORD WINAPI GRN_thread(LPVOID lpParam) {
    while (PROGRAM_RUNNING) {

        if (Request_queue->First == NULL) {
            SetEvent(REQUEST_EMPTY);
            WaitForSingleObject(REQUEST_EMPTY, INFINITE);
            continue;
        }
        switch (Request_queue->First->choice) {
        case 1:
            PROGRAM_RUNNING = 0;
            break;
        case 3:
            Add_GRN(global_list, Request_queue->First->value);
            break;
        case 4:
            SetN(Request_queue->First->idx, Request_queue->First->value, global_list);
            break;
        default:
            continue;
        }
        EnterCriticalSection(&REQUEST_QUEUE_ACCESS);
        Request* current_req = Request_queue->First;
        if (Request_queue->First->Next != NULL)
            Request_queue->First = Request_queue->First->Next;
        else {
            Request_queue->First = NULL;
            Request_queue->Last  = NULL;
        }
        free(current_req);
        LeaveCriticalSection(&REQUEST_QUEUE_ACCESS);
    }
    return 0;
}


DWORD WINAPI thread_1(LPVOID lpParam) {
    while (PROGRAM_RUNNING) {
        Sleep(Delay);
        Sum = CalculateSum(global_list);
    }
    return 0;
}

void CreateRequest(int choice, int value, int idx) {
    //Lock RGN_thread
    EnterCriticalSection(&REQUEST_QUEUE_ACCESS);

    if (Request_queue->First == NULL) {
        Request_queue->First = (Request*)malloc(sizeof(Request));
        if (Request_queue->First == NULL) {  
            LeaveCriticalSection(&REQUEST_QUEUE_ACCESS);
            EnterCriticalSection(&OUTPUT_ACCESS);

            printf("Couldn't allocate memory\n");
            LeaveCriticalSection(&OUTPUT_ACCESS);
            return;
        }
        Request_queue->Last = Request_queue->First;
        Request_queue->Last->Next = NULL;

        Request_queue->Last->choice = choice;
        Request_queue->Last->value = value;
        Request_queue->Last->idx = idx;

        LeaveCriticalSection(&REQUEST_QUEUE_ACCESS);
        return;
    }

    EnterCriticalSection(&REQUEST_QUEUE_ACCESS);

    Request_queue->Last->Next = (Request*)malloc(sizeof(Request));
    if (Request_queue->Last->Next == NULL) {
        LeaveCriticalSection(&REQUEST_QUEUE_ACCESS);
        EnterCriticalSection(&OUTPUT_ACCESS);

        printf("Couldn't allocate memory\n");
        LeaveCriticalSection(&OUTPUT_ACCESS);
        return;
    }
    Request_queue->Last = Request_queue->Last->Next;
    Request_queue->Last->choice = choice;
    Request_queue->Last->value = value;
    Request_queue->Last->idx = idx;
    Request_queue->Last->Next = NULL;

    //Unlock GRN_thread
    LeaveCriticalSection(&REQUEST_QUEUE_ACCESS);
}

//main user interface
DWORD WINAPI thread_2(LPVOID lpParam) {

    while (PROGRAM_RUNNING) {
        int choice = 0;

        printf(USER_INTERFACE);
        scanProtectedUserInterface(6, &choice);

        switch (choice) {
        case 1:
            PROGRAM_RUNNING = 0;
            return 0;
        case 2:
            EnterCriticalSection(&OUTPUT_ACCESS);
            GetNumGRN(global_list);
            LeaveCriticalSection(&OUTPUT_ACCESS);
            continue;
        case 3:
            EnterCriticalSection(&OUTPUT_ACCESS);

            printf("Enter value for  generator\n> ");

            LeaveCriticalSection(&OUTPUT_ACCESS);
            scanProtectedInput(&value);
            CreateRequest(choice, value, 0);
            ResetEvent(REQUEST_EMPTY);
            continue;
        case 4:
            EnterCriticalSection(&OUTPUT_ACCESS);

            printf("Enter index for  generator\n> ");
            scanProtectedInput(&idx);
            printf("Enter value for  generator\n> ");
            scanProtectedInput(&value);

            LeaveCriticalSection(&OUTPUT_ACCESS);

            CreateRequest(choice, value, idx);
            ResetEvent(REQUEST_EMPTY);
            continue;
        case 5:
            EnterCriticalSection(&OUTPUT_ACCESS);

            printf("Enter value to set delay\n> ");

            LeaveCriticalSection(&OUTPUT_ACCESS);
            scanProtectedInput(&Delay);
            continue;
        case 6:
            GetSum();
            continue;
        default:
            continue;
        }
    }
    return 0;
}

void init() {
    Request_queue = (Request_list*)malloc(sizeof(Request_list));
    global_list = (List_tail*)malloc(sizeof(List_tail));

    if (global_list == NULL || Request_queue == NULL) {
        printf("Couldn't allocate memory for the start. Quitting!");
        exit(1);
    }
    global_list->tail    = NULL;
    Request_queue->First = NULL;
    Request_queue->Last  = NULL;


    Add_GRN(global_list, 10);
    Add_GRN(global_list, 20);
    Add_GRN(global_list, 30);

    InitializeCriticalSection(&REQUEST_QUEUE_ACCESS);
    InitializeCriticalSection(&OUTPUT_ACCESS);
}

int main() {
    init();

    REQUEST_EMPTY = CreateEvent(NULL, FALSE, TRUE, NULL);

    Handle_of_Thread_1 = CreateThread(NULL, 0, thread_1, &Data_of_thread_1, 0, NULL);
    if (Handle_of_Thread_1 == NULL)
        ExitProcess(Data_of_thread_1);
    Handle_of_Thread_2 = CreateThread(NULL, 0, thread_2, &Data_of_thread_2, 0, NULL);
    if (Handle_of_Thread_2 == NULL)
        ExitProcess(Data_of_thread_2);
    Handle_of_Thread_3 = CreateThread(NULL, 0, GRN_thread, &Data_of_thread_3, 0, NULL);
    if (Handle_of_Thread_3 == NULL)
        ExitProcess(Data_of_thread_2);

    Threads_Handles_Arrray[0] = Handle_of_Thread_1;
    Threads_Handles_Arrray[1] = Handle_of_Thread_2;
    Threads_Handles_Arrray[2] = Handle_of_Thread_3;

//Waiting all threads are done their current operation
    WaitForMultipleObjects(3, Threads_Handles_Arrray, TRUE, INFINITE);
    printf("Threads are done");


//Free memory
    if (global_list->tail != NULL) {
        while (global_list->tail->prev != NULL) {
            global_list->tail = global_list->tail->prev;
            free(global_list->tail->next);
        }
        free(global_list->tail);
        free(global_list);
    }
    if (Request_queue->First != NULL) {
        while (Request_queue->First->Next != NULL){
            Request* tmp = Request_queue->First;
            Request_queue->First = Request_queue->First->Next;
            free(tmp);
        }
        free(Request_queue->First);
        free(Request_queue);
    }
    
    CloseHandle(Handle_of_Thread_1);
    CloseHandle(Handle_of_Thread_2);
    CloseHandle(Handle_of_Thread_3);

}