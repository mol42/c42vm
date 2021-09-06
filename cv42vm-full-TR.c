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
    int* heap;      // stack disinda tutulacak datalarin baslangic adresi ( pointer )
    int* code;      // calistirilacak kodlarin arrayine ait referans ( pointer )
    int* stack;     // yaratilan stackin baslangic adresini tutan referans ( pointer )
    int pc;         // program pozisyonunu tutan degisken
    int sp;         // stack pozisyonunu tutan degisken
    int fp;         // goreceli pozisyonlama icin kullanilan cercevenin pozisyonunu tutan degisken
                    // genel olarak fonksiyon cagirirken pushlanan degiskenleri offset vererek
                    // okuyabilmek icin kullanilir ve degeri bir fonksiyon tam cagrilirken ki
                    // stack pointer degerini tutan degiskendir.
                    // ---> DIKKAT makina dili calistiran uygulamalarda kodlama 
                    // matiginda fonksiyon yok mantiksal olarak X bir indexten baslayip bir yere kadar
                    // giden kod bolgesi fonksiyon oluyor.
} C42VM;

C42VM* newC42VM(int* code,      // calistirilacak bytecode'larin bulundugu dizinin referansi
    int pc,                     // baslangic program pozisyonu, ilk calistirilacak kodun pozisyonu
    int heapMemorySize) {       // lokal data ( stack disindaki data ) icin belirlenen boyut
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
    LOAD_PARAM = 11,      // lokal verilerden yukle
    GLOAD = 12,     // global verilerden yukle
    STORE_PARAM = 13,     // lokal veriyi global verilere sakla
    GSTORE = 14,    // global memory alanina sakla
    PRINT = 15,     // stackin en tepesindeki sayiyi ekrana yazdir
    POP = 16,       // stackin en disindaki sayiyi cikart ( ama kullanmadan )
    EXIT = 17,      // programi durdur
    EXECUTE_FUNCTION = 18,  // bir programcik calistir ( verilen fonksyionu calistir )
    RETURN = 19       // fonksiyondan donus yap
};

#define S_PUSH(vm, v) vm->stack[++vm->sp] = v       // stackin tepesine bir deger koy ( C derlenirken kodlarda S_PUSH olan yere arka planda "vm->stack[++vm->sp] = v" yazar )
                                                    // DIKKAT --> "++vm->sp" su demek once vm->sp'nin degerini 1 arttir ve vm->stack[NN] icinde o yeni deger ile kullan.
#define S_POP(vm)     vm->stack[vm->sp--]           // stackin tepesinden bir deger al
#define NEXT_BYTE_CODE(vm)   vm->code[vm->pc++]     // siradaki bytecode degerini oku ( bytecode bir komut, deger ya da referans olabilir illa ki komut olmak zorunda degil ! )
#define TRUE 1
#define FALSE 0


void printStack(C42VM* vm, int activePC, int isBeforeOpcode) {
    printf("\n");
    if (isBeforeOpcode == TRUE) {
        printf("****************************************\n");
    }
    printf("\n");
    printf(isBeforeOpcode == TRUE ? "BEFORE EXECUTION\n" : "AFTER EXECUTION\n");
    printf("\n");
    printf("vm->pc -> %d\n", activePC);
    printf("vm->sp -> %d\n", vm->sp);
    printf("vm->fp -> %d\n", vm->fp);
    printf("\n");
    printf("----- STACK BEGIN -----\n");
    for (int i = 0; i <= vm->sp; i++) {
        printf("i : %d, value : %d\n", i, vm->stack[i]);
    }
    printf("----- STACK END -------\n");
    printf("\n");
    if (isBeforeOpcode == FALSE) {
        printf("****************************************\n");
    }
    printf("\n");
}

void runVM(C42VM* vm){
    do{
        int previousPC = vm->pc;
        int opcode = NEXT_BYTE_CODE(vm);  // bir bytecode oku
        // bu noktada vm->pc 1 artmis oldu !!!
        int v, addr, offset, a, b, argc, rval, temp;
        
        printStack(vm, previousPC, TRUE);

        switch (opcode) {   // kodu biz yazdigimiz icin mantiken bu bytecode bir komuttur ve switch icerisinde ilgilenen ilgilendigi degeri okuma islemini ayrica yapacak
            case EXIT: 
                printf("EXIT\n"); 
                return;  // program ( ya da sonsuz dongu durur ve haliye exit gerceklesir )
            case CONST_I32:
                v = NEXT_BYTE_CODE(vm);   // bu islem siradaki bytecode degerini okur
                printf("CONST_I32 v -> %d\n", v);
                S_PUSH(vm, v);     // ... ve stack'in en tepesine koyar
                printStack(vm, vm->pc, FALSE);
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
                a = S_POP(vm);
                if(a) {      // ... stackin en tepesindeki degeri cikartir ve kontrol eder
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
                printf("SQUARE_I32 a -> %d\n", a);
                S_PUSH(vm, a * a);    // okunan degerin karesi alinir ve bulunan deger stacke eklenir
                printStack(vm, vm->pc, FALSE);
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
                printf("EQ_I32 a -> %d b -> %d\n", a, b);
                S_PUSH(vm, a == b ? 1 : 0);    // ... 2 degerin esitligini kontrol eder ve sonucu stackin en tepesine koyariz
                printStack(vm, vm->pc, FALSE);
                break;
            case LOAD_PARAM:                            // lokal bir degeri ( ya da diger adiya fonksiyon degiskenini okuruz )
                offset = NEXT_BYTE_CODE(vm);            // fonksiyonlara aktarilan degerleri okumak icin cerceve adresinden yararlaniyoruz ve cerceveye gore offseti siraki bytecode olarak okuruz
                printf("LLOAD offset -> %d vm->stack[vm->fp+offset] -> %d\n", offset, vm->stack[vm->fp+offset]);
                S_PUSH(vm, vm->stack[vm->fp+offset]);   // ... fonksiyon parametresinin degerini cerceve adresi + offset yardimi ile okuruz ve stackin en tepesine koyariz ( cunku stack islemler stack uzerinden gidiyor... )
                printStack(vm, vm->pc, FALSE);
                break;
            case STORE_PARAM:                   // lokal bir degeri ya da fonksiyon parametresinin degerini global data alaninda sakla
                v = S_POP(vm);                  // stackin en tepesindeki degeri okuruz
                offset = NEXT_BYTE_CODE(vm);    // ... cerceve icinde yer alan globale saklamak istedigimiz adres konumunu okuruz
                vm->heap[vm->fp+offset] = v;    // ... cerceve adresi yardimi ile bu fonksiyon parametresinin degerini stackten okunan deger ile guncelleriz
                break;
            case EXECUTE_FUNCTION:
                // bir fonksyion cagrilirken butun parametrelerin set edilmesi gerekiyor ( aslinda fonksiyon cagirmiyoruz bir adrese atliyoruz yani kavramsal olarak sanki fonksiyon cagirmis gibi oluyoruz )
                addr = NEXT_BYTE_CODE(vm);  // atlanacak fonksiyonun adresini bytecode olarak okuruz ...
                argc = NEXT_BYTE_CODE(vm);  // ... cagrilacak fonksiyonun bekledigi parametre adedini bytecode olarak okuruz ...
                S_PUSH(vm, argc);           // ... parametre adedini stacke koyariz ...
                S_PUSH(vm, vm->fp);         // ... bir onceki cercevenin adresini saklariz ( cagrilacak fonksiyonun cercevesi degil yani !! ) ...
                S_PUSH(vm, vm->pc);         // ... o anki program adim degerini adresini saklariz ...
                printf("EXECUTE_FUNCTION addr -> %d argc -> %d vm->fp -> %d vm->pc -> %d\n", addr, argc, vm->fp, vm->pc);
                // DIKKAT bu noktada vm->sp 3 index artmis oldu 3 kez S_PUSH cagirdigimiz icin
                vm->fp = vm->sp;  // ... yeni cercevenin adresini atiyoruz ( cerceve stack'in fonksiyon cagrilirken tuttugu adres degeridir ) ...
                                  // ... bu noktada onemli detay su, vm->sp degeri aktarilan parametreler kadar ilerlemis oluyor yani 2 parametre pushlamis isek
                                  // ... cagrilacak metodu bu 2 parametre cagrilan metod sonrasi temizlenmeli aksi takdirde programin stackinde kalir
                                  // ... ve gereksiz yer isgal eder ve ayrica programin akisinin bozulmasina neden olur
                vm->pc = addr;    // ... program adim degerini fonksiyonun adresi ile guncelleriz ( yani teknik olarak cagirmis oluyoruz bu sayede sihir gibi di mi ? )
                printStack(vm, vm->pc, FALSE);
                break;
            case RETURN:
                rval = S_POP(vm);       // stackin en tepesinde geri donulecek deger vardir ve bunu okuruz
                vm->sp = vm->fp;        // ... aktif fonksiyon cagrilmadan onceki stack adimina geceriz ( yani stack fonksiyon cagrilirken kullanilan adet kadar geri gider )
                                        // EXECUTE_FUNCTION icine baktiginizda "vm->fp = vm->sp" kodunu goreceksiniz yani tam bu return edilen metod cagrilmadan onceki vm->sp
                                        // degeri vm->fp ye aktariliyor yani parametre degerleri + stack'e sakladigimiz state'lerin hemen sonrasina donuyor stack pointer.
                printf("RETURN vm->fp -> %d\n", vm->fp);
                vm->pc = S_POP(vm);     // ... aktif fonksiyon cagrilmadan onceki program adim degerini stackten okuruz ve yeni program adim degeri olarak set ederiz ( yani teknik olarak aktif fonksiyonun cagrildigi noktaya donmus olduk ) ...
                vm->fp = S_POP(vm);     // ... bir onceki cerceve adresini stackten okuruz ...
                argc = S_POP(vm);       // ... donulen fonksiyonun parametre adedini stackten okuruz ...
                temp = S_POP(vm);              // ... stack adim degerini parametre adedi kadar geri iteriz ...
                S_PUSH(vm, rval);     // ... donulecek degeri stacke pushlariz ( kodu biz yazdigimiz icin stackteki bu geri donus degerinin yonetimiz bizde yani ihtiyac duymuyorsak mutlaka stackten cikarmaliyiz yoksa sonraki islemlerde karisikliga neden olur )
                printf("RETURN rval -> %d vm->sp -> %d vm->fp -> %d argc -> %d\n", rval, vm->sp, vm->fp, argc);
                printStack(vm, vm->pc, FALSE);
                break;
            case POP:
                --vm->sp;      // stacki 1 geri iter ve o anki deger ne ise gormezden gelinir
                break;
            case PRINT:
                v = S_POP(vm);        // stackin en tepesindeki degeri okuruz
                printf("PRINT v -> %d\n", v);  // ... ve ekrana bastiririz
                S_PUSH(vm, v);
                printStack(vm, vm->pc, FALSE);
                break;
            default:
                break;
        }

    }while(1);
}

int main() {
    int program[] = {
        CONST_I32, 7, // cagrilacak metoda aktarilacak parametre degeri
        EXECUTE_FUNCTION, 7, 1,
        PRINT,
        EXIT,
        // 2 sayinin karesini alip donen metod
        LOAD_PARAM, -3, // bu metoda aktarilmak istenen degeri stackin en tepesine kopyalayip ekliyoruz
        SQUARE_I32,
        RETURN,
        // main burasi
        CONST_I32, 6,
        CONST_I32, 6,
        EQ_I32,
        JMPT, 0,
        CONST_I32, 5,
        CONST_I32, 99,
        ADD_I32,
        PRINT,
        EXIT            
    };
    
    // initialize virtual machine
    C42VM* vm = newC42VM(program,
                       11, 
                       100);
    runVM(vm);

    return 0;
}
  