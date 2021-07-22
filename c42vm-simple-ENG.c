#include <stdio.h>


#define STACK_SIZE 100

typedef struct {
    int* heap;    
    int* code;      
    int* stack;     
    int pc;         
    int sp;         
    int fp;  
} C42VM;

C42VM* newC42VM(int* code,
    int pc,
    int heapMemorySize) {
    C42VM* vm = (C42VM*)malloc(sizeof(C42VM));
    vm->code = code;
    vm->pc = pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->heap = (int*)malloc(sizeof(int) * heapMemorySize);
    vm->stack = (int*)malloc(sizeof(int) * STACK_SIZE);

    return vm;
}

void destroyC42VM(C42VM* vm){
    free(vm->heap);
    free(vm->stack);
    free(vm);
}

enum {
    ADD_I32 = 1,
    SUB_I32 = 2,
    MUL_I32 = 3,
    SQUARE_I32 = 4,
    LT_I32 = 5,
    EQ_I32 = 6,
    JMP = 7,
    JMPT = 8,
    JMPF = 9,
    CONST_I32 = 10,
    LLOAD = 11,
    GLOAD = 12,
    LSTORE = 13,
    GSTORE = 14,
    PRINT = 15,
    POP = 16,
    EXIT = 17,
    EXEC_ROUTINE = 18,
    RETURN = 19
};

#define S_PUSH(vm, v) vm->stack[++vm->sp] = v
#define S_POP(vm)     vm->stack[vm->sp--]     
#define NEXT_BYTE_CODE(vm)   vm->code[vm->pc++] 

void runVM(C42VM* vm){
    do{
        int opcode = NEXT_BYTE_CODE(vm); 
        int v, addr, offset, a, b, argc, rval;

        switch (opcode) {
            case EXIT: 
                return;
            case CONST_I32:
                v = NEXT_BYTE_CODE(vm);
                S_PUSH(vm, v);
                break;
            case ADD_I32:
                b = S_POP(vm); 
                a = S_POP(vm); 
                S_PUSH(vm, a + b);
                break;
            case PRINT:
                v = S_POP(vm);   
                printf("%d\n", v);
                break;
            default:
                break;
        }

    }while(1);
}

int main() {
    int program[] = {
        CONST_I32, 2,
        CONST_I32, 6,
        ADD_I32,
        PRINT,
        EXIT            
    };
    
    // initialize virtual machine
    C42VM* vm = newC42VM(program,
                       0, 
                       100);
    runVM(vm);

    return 0;
}
  