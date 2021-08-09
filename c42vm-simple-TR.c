//
// HEY ENGLISH SPEAKING PEOPLE ! 
// 
// CHECK OUT c2vm-v1-ENG.c or c42vm-full-ENG.c FILES FOR SOURCE CODES WITH ENGLISH COMMENTS
//
//

#include <stdio.h>

// ONEMLI NOT :
// BU SANAL ISLEMCI SADECE INTEGER SAYILARLA ISLEM YAPMAK UZERE
// TASARLANDI. STRING FLOATING POINT VB DATA YAPILARI ILE ISLEMLERI YAPMAK ISTERSEK
// BU IHTIYACA GORE EK BYTECODE VE MEMORY YONETIMI KODLAMASI
// YAPMAK GEREKIR !


// stack boyutu genellikle sabit oluyor 
// ve istenirse sanal islemci run olmadan hemen once ayarlanabiliyor
// run olurken de ayarlanabilir fakat data kaybina neden olacagi icin
// buyutmeye izin verilebilir fakat kucultme islemi yapilmamasi gerekir ! 
#define STACK_SIZE 100

typedef struct {
    int* heap;    // stack disinda tutulacak datalarin baslangic adresi ( pointer )
    int* code;      // calistirilacak kodlarin arrayine ait referans ( pointer )
    int* stack;     // yaratilan stackin baslangic adresini tutan referans ( pointer )
    int pc;         // program pozisyonunu tutan degisken
    int sp;         // stack pozisyonunu tutan degisken
    int fp;         // goreceli pozisyonlama icin kullanilan cercevenin pozisyonunu tutan degisken
} C42VM;

C42VM* newC42VM(
    int* code,    // calistirilacak bytecode'larin bulundugu dizinin referansi
    int pc,             // baslangic program pozisyonu, ilk calistirilacak kodun pozisyonu
    int heapMemorySize // lokal data ( stack disindaki data ) icin belirlenen boyut
){      
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
    ADD_I32 = 1,    // tam sayi toplama
    CONST_I32 = 2,  // tam sayi degeri stacke koy
    PRINT = 3,     // stackin en tepesindeki sayiyi ekrana yazdir
    EXIT = 4,      // programi durdur
};

#define S_PUSH(vm, v) vm->stack[++vm->sp] = v // stackin tepesine bir deger koy ( C derlenirken kodlarda S_PUSH olan yere arka planda "vm->stack[++vm->sp] = v" yazar )
#define S_POP(vm)     vm->stack[vm->sp--]     // stackin tepesinden bir deger al
#define NEXT_BYTE_CODE(vm)   vm->code[vm->pc++]      // siradaki bytecode degerini oku ( bytecode bir komut, deger ya da referans olabilir illa ki komut olmak zorunda degil ! )

void runVM(C42VM* vm){
    do{
        int opcode = NEXT_BYTE_CODE(vm);        // bir bytecode oku
        int value, addr, offset, a, b, argc, rval;

        switch (opcode) {   // kodu biz yazdigimiz icin mantiken bu bytecode bir komuttur ve switch icerisinde ilgilenen ilgilendigi degeri okuma islemini ayrica yapacak
            case EXIT: 
                return;  // program ( ya da sonsuz dongu durur ve haliye exit gerceklesir )
            case CONST_I32:
                value = NEXT_BYTE_CODE(vm);   // bu islem siradaki bytecode degerini okur
                S_PUSH(vm, value);     // ... ve stack'in en tepesine koyar
                break;
            case ADD_I32:
                b = S_POP(vm);        // bu islem stackin en tepesinden degeri okur ( stack 1 geri gider )
                a = S_POP(vm);        // ...sonra tekrar en tepedeki degeri okur ( bir onceki islemde stack 1 geri gitmisti... )
                S_PUSH(vm, a + b);    // ... okudugu 2 degeri toplar ve stackin en tepesine koyar
                break;
            case PRINT:
                value = S_POP(vm);        // stackin en tepesindeki degeri okuruz
                printf("%d\n", value);  // ... ve ekrana bastiririz
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
  