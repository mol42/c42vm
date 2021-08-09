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

C42VM* newC42VM(int* code,    // calistirilacak bytecode'larin bulundugu dizinin referansi
    int pc,             // baslangic program pozisyonu, ilk calistirilacak kodun pozisyonu
    int heapMemorySize) {      // lokal data ( stack disindaki data ) icin belirlenen boyut
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
    SUB_I32 = 2,    // tam sayi cikarma
    MUL_I32 = 3,    // tam sayi carpma
    SQUARE_I32 = 4, // tam sayinin karesini al
    LT_I32 = 5,     // tam sayi kucuktur kontrolu ( x < 5 gibi )
    EQ_I32 = 6,     // tam sayi esittir kontrolu
    JMP = 7,        // dallan ( verilen program adresine git )
    JMPT = 8,       // eger true ise dallan
    JMPF = 9,       // eger false ise dallan
    CONST_I32 = 10,  // tam sayi degeri stacke koy
    LLOAD = 11,      // lokal verilerden yukle
    GLOAD = 12,     // global verilerden yukle
    LSTORE = 13,     // lokal veriyi global verilere sakla
    GSTORE = 14,    // global memory alanina sakla
    PRINT = 15,     // stackin en tepesindeki sayiyi ekrana yazdir
    POP = 16,       // stackin en disindaki sayiyi cikart ( ama kullanmadan )
    EXIT = 17,      // programi durdur
    EXEC_ROUTINE = 18,  // bir programcik calistir ( verilen fonksyionu calistir )
    RETURN = 19       // fonksiyondan donus yap
};

#define S_PUSH(vm, v) vm->stack[++vm->sp] = v // stackin tepesine bir deger koy ( C derlenirken kodlarda S_PUSH olan yere arka planda "vm->stack[++vm->sp] = v" yazar )
#define S_POP(vm)     vm->stack[vm->sp--]     // stackin tepesinden bir deger al
#define NEXT_BYTE_CODE(vm)   vm->code[vm->pc++]      // siradaki bytecode degerini oku ( bytecode bir komut, deger ya da referans olabilir illa ki komut olmak zorunda degil ! )

void runVM(C42VM* vm){
    do{
        int opcode = NEXT_BYTE_CODE(vm);        // bir bytecode oku
        int v, addr, offset, a, b, argc, rval;

        switch (opcode) {   // kodu biz yazdigimiz icin mantiken bu bytecode bir komuttur ve switch icerisinde ilgilenen ilgilendigi degeri okuma islemini ayrica yapacak
            case EXIT: 
                return;  // program ( ya da sonsuz dongu durur ve haliye exit gerceklesir )
            case CONST_I32:
                v = NEXT_BYTE_CODE(vm);   // bu islem siradaki bytecode degerini okur
                S_PUSH(vm, v);     // ... ve stack'in en tepesine koyar
                break;
            case ADD_I32:
                b = S_POP(vm);        // bu islem stackin en tepesinden degeri okur ( stack 1 geri gider )
                a = S_POP(vm);        // ...sonra tekrar en tepedeki degeri okur ( bir onceki islemde stack 1 geri gitmisti... )
                S_PUSH(vm, a + b);    // ... okudugu 2 degeri toplar ve stackin en tepesine koyar
                break;
            case LT_I32:
                b = S_POP(vm);        // bu islem stackin en tepesinden degeri okur ( stack 1 geri gider )
                a = S_POP(vm);        // ...sonra tekrar en tepedeki degeri okur ( bir onceki islemde stack 1 geri gitmisti... )
                S_PUSH(vm, (a<b) ? 1 : 0); // ... okunan 2 deger karsilastirilir ve bulunan sonuc stackin en tepesine koyulur ( ikinci degisken birinciden kucuk mu kontrolu yapiliyor )
                break;
            case JMP:
                vm->pc = NEXT_BYTE_CODE(vm);  // program pozisyonunu verilen adrese gecirir yani programimiz oraya ziplar ( herhangi bir kontrol yapmadan )
                break;
            case JMPT:
                addr = NEXT_BYTE_CODE(vm);  // siradaki bytecode degerini okur ( bu deger ziplanacak adrestir )
                if(S_POP(vm)) {      // ... stackin en tepesindeki degeri cikartir ve kontrol eder
                    vm->pc = addr; // ... eger kontrol dogru ise verilen adrese ziplar
                }
                break;    
            case JMPF:
                addr = NEXT_BYTE_CODE(vm);  // siradaki bytecode degerini okur ( bu deger ziplanacak adrestir )
                if(!S_POP(vm)) {      // ... stackin en tepesindeki degeri cikartir ve kontrol eder
                    vm->pc = addr; // ... eger kontrol yanlis (false) ise verilen adrese ziplar
                }
                break; 
            case SUB_I32:
                b = S_POP(vm);        // stackin en tepesinden 2. degeri okur ( stack 1 geri gider )
                a = S_POP(vm);        // stackin en tepesinden 1. degeri okur ( stack 1 geri gider )
                S_PUSH(vm, a - b);    // ... 1.degerden 2.deger cikartilir ve bulunan sonuc stackin en tepesine koyulur
                break;
            case MUL_I32:
                b = S_POP(vm);        // stackin en tepesinden 2. degeri okur ( stack 1 geri gider )
                a = S_POP(vm);        // stackin en tepesinden 1. degeri okur ( stack 1 geri gider )
                S_PUSH(vm, a * b);    // ... 1.deger ile 2.deger carpilir ve bulunan sonuc stackin en tepesine koyulur
                break;
            case SQUARE_I32:
                a = S_POP(vm);        // stackin en tepesinden degeri okur ( stack 1 geri gider )
                S_PUSH(vm, a * a);    // okunan degerin karesi alinir ve bulunan deger stacke eklenir
                break;                
            case GLOAD:
                addr = S_POP(vm);             // siradaki bytecode degerini okur ( bu deger bir adrestir )
                v = vm->heap[addr];         // ... verilen adresteki degeri okur ( deger bir tam sayidir )
                S_PUSH(vm, v);                // ... ve stackin en tepesine koyar
                break;
            case GSTORE:
                v = S_POP(vm);                // stackin en tepesindeki degeri okuruz ( ve stack 1 geri gider )
                addr = NEXT_BYTE_CODE(vm);           // ... siradaki bytecode degerini okuuz ( bu deger bir adrestir )
                vm->heap[addr] = v;         // ... verilen adrese saklamak istedigimiz degeri yazariz
                break;
            case EQ_I32:
                b = S_POP(vm);        // stackin en tepesinden 2. degeri okur ( stack 1 geri gider )
                a = S_POP(vm);        // stackin en tepesinden 1. degeri okur ( stack 1 geri gider )
                S_PUSH(vm, a == b);    // ... 2 degerin esitligini kontrol eder ve sonucu stackin en tepesine koyariz
                break;
            case LLOAD:                  // lokal bir degeri ( ya da diger adiya fonksiyon degiskenini okuruz )
                offset = NEXT_BYTE_CODE(vm);     // fonksiyonlara aktarilan degerleri okumak icin cerceve adresinden yararlaniyoruz ve cerceveye gore offseti siraki bytecode olarak okuruz
                S_PUSH(vm, vm->stack[vm->fp+offset]); // ... fonksiyon parametresinin degerini cerceve adresi + offset yardimi ile okuruz ve stackin en tepesine koyariz ( cunku stack islemler stack uzerinden gidiyor... )
                break;
            case LSTORE:                 // lokal bir degeri ya da fonksiyon parametresinin degerini global data alaninda sakla
                v = S_POP(vm);            // stackin en tepesindeki degeri okuruz
                offset = NEXT_BYTE_CODE(vm);     // ... cerceve icinde yer alan globale saklamak istedigimiz adres konumunu okuruz
                vm->heap[vm->fp+offset] = v;  // ... cerceve adresi yardimi ile bu fonksiyon parametresinin degerini stackten okunan deger ile guncelleriz
                break;
            case EXEC_ROUTINE:
                // bir fonksyion cagrilirken butun parametrelerin set edilmesi gerekiyor ( aslinda fonksiyon cagirmiyoruz bir adrese atliyoruz yani kavramsal olarak sanki fonksiyon cagirmis gibi oluyoruz )
                addr = NEXT_BYTE_CODE(vm); // atlanacak fonksiyonun adresini bytecode olarak okuruz ...
                argc = NEXT_BYTE_CODE(vm); // ... cagrilacak fonksiyonun bekledigi parametre adedini bytecode olarak okuruz ...
                S_PUSH(vm, argc);   // ... parametre adedini stacke koyariz ...
                S_PUSH(vm, vm->fp); // ... o anki cercevenin adresini saklariz ( cagrilacak fonksiyonun cercevesi degil bu !! ) ...
                S_PUSH(vm, vm->pc); // ... o anki program adim degerini adresini saklariz ...
                vm->fp = vm->sp;  // ... yeni cercevenin adresini guncelleriz ( cerceve stack'in fonksiyon cagrilirken tuttugu adres degeridir ) ...
                vm->pc = addr;    // ... program adim degerini fonksiyonun adresi ile guncelleriz ( yani teknik olarak cagirmis oluyoruz bu sayede sihir gibi di mi ? )
                break;
            case RETURN:
                rval = S_POP(vm);     // stackin en tepesinde geri donulecek deger vardir ve bunu okuruz
                vm->sp = vm->fp;    // ... akfit fonksiyon cagrilmadan onceki stack adimina geceriz ( yani stack fonksiyon cagrilirken kullanilan adet kadar geri gider )
                vm->pc = S_POP(vm);   // ... aktif fonksiyon cagrilmadan onceki program adim degerini stackten okuruz ve yeni program adim degeri olarak set ederiz ( yani teknik olarak aktif fonksiyonun cagrildigi noktaya donmus olduk ) ...
                vm->fp = S_POP(vm);   // ... bir onceki cerceve adresini stackten okuruz ...
                argc = S_POP(vm);     // ... donulen fonksiyonun parametre adedini stackten okuruz ...
                vm->sp -= argc;     // ... stack adim degerini parametre adedi kadar geri iteriz ...
                S_PUSH(vm, rval);     // ... donulecek degeri stacke pushlariz ( kodu biz yazdigimiz icin stackteki bu geri donus degerinin yonetimiz bizde yani ihtiyac duymuyorsak mutlaka stackten cikarmaliyiz yoksa sonraki islemlerde karisikliga neden olur )
                break;
            case POP:
                --vm->sp;      // stacki 1 geri iter ve o anki deger ne ise gormezden gelinir
                break;
            case PRINT:
                v = S_POP(vm);        // stackin en tepesindeki degeri okuruz
                printf("%d\n", v);  // ... ve ekrana bastiririz
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
        SQUARE_I32,
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
  