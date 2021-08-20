#pragma warning(disable : 4996) //turns off scanf_s  warning for VS 2019
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define USER_INTERFACE "Choose task: \n\
    2. Get number of Gererators\n\
    3. Add one more Random number generator\n\
    4. Change N on special Generator\n\
    5. Set delay\n\
    6. Get Sum\n\
    1. Exit\n> "


int program_running_flag = 1;
int value = 0;
int idx = 0;
int Sum = 0;
int Gsize = 10;
int Gcounter = 0;
int GRN_counter = 0;
int Delay = 3;

int* Array_GRN;

void copyArr(int* Arr1, int* Arr2, int size) {
    if (Arr2 == NULL)
        return;
    for (int i = 0; i < size; i++)
        Arr1[i] = Arr2[i];
    free(Arr2);
}
//1.
int GetNumGRN() {
    printf("Number of GRN is %d now\n", Gcounter);
    return Gcounter;
}

int Reallocation(int* Arr, int size) {
    int* tmp = Arr;
    Arr = (int*)malloc(sizeof(int) * size * 2);
    if (Arr == NULL)
        return 12;

    for (int i = 0; i < size; i++)
        Arr[i] = tmp[i];

    free(tmp);
    Array_GRN = Arr;
    Gsize = size * 2;

    return 0;
}
//2.
int CreateGRN(int N) {
    if (Gcounter == Gsize)
        if (Reallocation(Array_GRN, Gsize) == 12) {
            printf("Coudn't reallocate memory. Didn't create new generator");
            return 12;
        }

    Array_GRN[Gcounter++] = N;
    return 0;
}
//3.
void SetN(int idx, int value) {
    if (idx > Gcounter || idx < 0) {
        printf("Out of range N's\n");
        return;
    }
    Array_GRN[idx-1] = value;
}

//4.
void ChangeDelay(int delay) {
    if (delay < 0) {
        printf("Delay can't be less then 0\n");
        return;
    }
    Delay = delay;
}
//5.
int GetSum() {
    printf("Sum is %d\n", Sum);
    return Sum;
}
void CalculateSum() {
    int summ = 0;
    for (int i = 0; i < Gcounter; i++) {
        summ += rand() % Array_GRN[i];
    }
    Sum = summ;
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

//There could be potential bug that 2 mini threads are running at the same time, and one of them changing global value
DWORD WINAPI thread_3(LPVOID lpParam) {
    CreateGRN(value);
    value = 0;
    return 0;
}
DWORD WINAPI thread_4(LPVOID lpParam) {
    SetN(idx, value);
    value = 0;
    idx = 0;
    return 0;
}
DWORD WINAPI thread_5(LPVOID lpParam) {
    ChangeDelay(value);
    value = 0;
    return 0;
}


DWORD WINAPI thread_1(LPVOID lpParam) {
    while (program_running_flag) {
        Sleep(Delay);
        CalculateSum();
    }
    return 0;
}

//main user interface
DWORD WINAPI thread_2(LPVOID lpParam) {
    int Data_of_thread_3 = 3;
    HANDLE Handle_of_Thread_3 = 0;

    while (program_running_flag) {
        int choice = 0;

        printf(USER_INTERFACE);
        scanProtectedUserInterface(6, &choice);

        if (choice == 1) {
            program_running_flag = 0;
            return 0;
        }
        if (choice == 2) {

            GetNumGRN();
            continue;
        }
        if (choice == 3) {
            printf("Enter value for  generator\n> ");
            scanProtectedInput(&value);
            Handle_of_Thread_3 = CreateThread(NULL, 0, thread_3, &Data_of_thread_3, 0, NULL);
            continue;
        }
        if (choice == 4) {
            printf("Enter element(A[0] = 1) to change\n> ");
            scanProtectedInput(&idx);
            printf("Enter value for  generator\n> ");
            scanProtectedInput(&value);
            Handle_of_Thread_3 = CreateThread(NULL, 0, thread_4, &Data_of_thread_3, 0, NULL);
            continue;
        }
        if (choice == 5) {
            printf("Enter value to set delay\n> ");
            scanProtectedInput(&value);
            Handle_of_Thread_3 = CreateThread(NULL, 0, thread_5, &Data_of_thread_3, 0, NULL);
            continue;
        }
        if (choice == 6)
            GetSum();
        continue;
    }
}

void init() {
    Array_GRN = (int*)malloc(sizeof(int) * Gsize);
    Array_GRN[Gcounter++] = 10;
    Array_GRN[Gcounter++] = 20;
    Array_GRN[Gcounter++] = 30;
}

int main() {
    init();

    int Data_of_thread_1 = 1;
    int Data_of_thread_2 = 2;

    HANDLE Handle_of_Thread_1 = 0;
    HANDLE Handle_of_Thread_2 = 0;

    HANDLE Threads_Handles_Arrray[2];

    Handle_of_Thread_1 = CreateThread(NULL, 0, thread_1, &Data_of_thread_1, 0, NULL);
    if (Handle_of_Thread_1 == NULL)
        ExitProcess(Data_of_thread_1);
    Handle_of_Thread_2 = CreateThread(NULL, 0, thread_2, &Data_of_thread_2, 0, NULL);
    if (Handle_of_Thread_2 == NULL)
        ExitProcess(Data_of_thread_2);

    Threads_Handles_Arrray[0] = Handle_of_Thread_1;
    Threads_Handles_Arrray[1] = Handle_of_Thread_2;

    WaitForMultipleObjects(2, Threads_Handles_Arrray, TRUE, INFINITE);
    printf("Threads are done");
    free(Array_GRN);
    CloseHandle(Handle_of_Thread_1);
    CloseHandle(Handle_of_Thread_2);

}
