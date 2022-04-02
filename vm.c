// Standard C Libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// Other Libraries
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>



// Macros definition
#define magic 0x6d736100 //this is WASM magic, WASM file start with this. 
#define version 0x01 //version will be right right after WASM magic



// Structs definition, this is to follow the module-based implementation. During parsing, we look for opcodes, and use switch case to fill them to the right module section; Webassembly is about calling the functions.
typedef struct Stack{
    int stack[64];
    int count;
} Stack;
// this section will be filled when we parse out 0x07, which is the function section
typedef struct Func{
    char *func_name;
    int func_pc;
} Func;
// this simply stores the information of different section in wasm file, and each will be a section part of the module
typedef struct Section{ 
    uint8_t section_id;
    uint8_t section_size;
    int pointer;
} Section;
// duruing pasring, module will be filled; during parsing, we looking for 0x0a and 0x07 for func section and code section.
typedef struct Module{
    Section sect[11];
    Func func[5];
} Module;



// Helper Functions
uint32_t read_four_bytes (uint8_t *byte_pointer, uint32_t *pos ){
    *pos +=4;
    return ((uint32_t *)(byte_pointer + *pos -4))[0];
}

uint8_t read_one_byte(uint8_t *byte_pointer, uint32_t *pos){
/**    uint64_t result_64 = 0;
    uint64_t byte;
    int count = *pos;
    int shift = 0;
    uint8_t result_8 = 0;
    
    shift = ((count%4)*2);
    result_64 = 0xff << shift;
    result_64 = result_64 & ((uint32_t *)(byte_pointer + *pos))[0];
    result_8 = (uint8_t)(result_64 >> shift);
    *pos += 1;
  return result_8; **/
    *pos += 1;
    return byte_pointer[*pos-1];
}
// get the size of the input file
int getsize(char *file){
    int fd;
    void *addr;
    struct stat sb;
    fd = open(file, O_RDONLY);
    if (fd < 0) {perror("OPEN Failed"); exit(-1);}
    if (fstat(fd, &sb)<0) {perror("fstat FAILED"); exit(-1);}
    return sb.st_size;
}
// this is the mapping functions, will the core is mmap(). 
uint8_t * mapWasmFile(char *file) {
    int fd;
    void *addr;
    struct stat sb;
    fd = open(file, O_RDONLY);
    if (fd < 0) {perror("OPEN Failed"); exit(-1);}// if failed 
    if (fstat(fd, &sb)<0) {perror("fstat FAILED"); exit(-1);}
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);// The mmap() function is used for mapping between a process address space and either files or devices
    if (addr == MAP_FAILED) {perror("MMAP: FAILED");exit(-1);}
    return addr;
}



// Function that parses the input file and produces the Module
Module *parse_all_bytes(uint8_t *byte_pointer, int byte_count){
    uint32_t pos = 0, sect_pos = 0, four_bytes;
    uint8_t sec_id,sec_size, num;
    Module *m; // this is our module
    Section *sec;
    Func *f; // we will eventually point a section inside Module to func. 

    m = calloc(1,sizeof(Module));
    sec = calloc(1,sizeof(Section));
    f = calloc(1,sizeof(Func));
    //start with Magic file first, so we parse the first 4 bytes that stores the Wasm_magic info
    four_bytes = read_four_bytes(byte_pointer,&pos);
    printf("Wasm_Magic: %" PRIu32 "\n",four_bytes);
    // next for will be Wasm_version
    four_bytes = read_four_bytes(byte_pointer,&pos);
    printf("Wasm_Version: %" PRIu32 "\n",four_bytes);
    // start to fill up the module, and for each look we will en counter a new sec id. 
    while (pos < byte_count){
    /*
    the following set ups acts on both debugging and program-user display. It tells user which sections that the module are storing and the order of storage
    */
      sec_id = read_one_byte(byte_pointer,&pos);
      printf("Accessing the %dth byte \n",pos);
      printf("Section id is %" PRIu8 "\n",sec_id);
      sec_size = read_one_byte(byte_pointer,&pos);
      printf("Accessing the %dth byte \n",pos);
      printf("Section Size is %" PRIu8 "\n",sec_size);
      sec->section_id = sec_id;
      sec->section_size = sec_size;
      sec->pointer = pos;
      m->sect[(int)sec_id] = *sec;
      // it keep on looking for the Function Section and the code section until it finds it. 
      switch (sec_id){
          case 0x07:
              sect_pos = pos;
              num = read_one_byte(byte_pointer,&sect_pos);
              for (int i=0; i < (int)num; i++){
                  uint32_t str_len = read_one_byte(byte_pointer, &sect_pos);
                  char * str = malloc(str_len+1);
                  memcpy(str, byte_pointer+sect_pos, str_len);
                  str[str_len] = '\0';
                  sect_pos += (str_len + 2);
                  f->func_name = str;
                  m->func[i] = *f; // now we have an access to the func section
              }
        // this is the code seciont
          case 0x0a: 
              sect_pos = pos;
              num = read_one_byte(byte_pointer,&sect_pos);
              for (int i=0; i < (int)num; i++){
                  uint32_t len = read_one_byte(byte_pointer, &sect_pos);
                  m->func[i].func_pc = (sect_pos+1);  // machine knows where to find the opcode (it does not look at the opcode at all)
                  sect_pos += (int)len;
              }
      }
      pos += (int)sec->section_size;
    }
    return m;
}

/* // this is going to be our future attemp, we may added logic flow and print features. and this table is going to show to readers what each opcode function is. (idea got from Kanaka)
char operation_define[][10] = {
    // basic logic flow and access, 
    "unreachable",           // 0x00, implemented
    "nop",                   // 0x01
    "block",                 // 0x02
    "loop",                  // 0x03
    "if",                    // 0x04
    "else",                  // 0x05
    "unreachable",           // 0x00
    "end",                   // 0x0b
    "return",                // 0x0f
    // Variable access
    "local.get",             // 0x20
    "local.set",             // 0x21
    "global.get",            // 0x23
    "global.set",            // 0x24

    // Comparison operators
    "i32.eqz",               // 0x45
    "i32.eq",                // 0x46
    "i32.ne",                // 0x47
    // Numeric_ex operators
    "i32.add",               // 0x6a
    "i32.sub",               // 0x6b
    "i32.mul",               // 0x6c
    "i32.div_s",             // 0x6d
    "i32.div_u",             // 0x6e
    "i32.rem_s",             // 0x6f
    "i32.rem_u",             // 0x70

};
*/



// The interpreter
int execute_op(uint8_t *file, int pc, int *local){
    uint8_t byte;
    uint32_t exe_pos = pc;
    int a,b;
    int stack[5];
    int sp = 0;
    bool end = true;
    // it keep on switch for the function cases. 
    while (end){
        byte = read_one_byte(file, &exe_pos);
        switch (byte){
            case 0x20: // local get, to get the program argument (input) and push to stack
                byte = read_one_byte(file, &exe_pos);
                stack[sp]=local[(int)byte];
                sp ++;
                break;
            case 0x6a ... 0x78:
                a = stack[sp-1];
                sp --;
                b = stack[sp-1];
                switch (byte){
                    case 0x6A:    // Add
                        stack[sp-1] = b + a;
                        break;
                    case 0x6B:    // Sub
                        stack[sp-1] = b - a;
                        break;
                    case 0x6C:    // Mul
                        stack[sp-1] = b * a;
                        break;
                    case 0x6E:    // Div
                        if (a == 0) {exit(-1);}
                        stack[sp-1] = b / a;
                        break;
                    case 0x70:    // Rem
                        stack[sp-1] = b % a;
                        break;
                    case 0x71:    // And
                        stack[sp-1] = b & a;
                        break;
                    case 0x72:    // Or
                        stack[sp-1] = b | a;
                        break;
                    case 0x73:    // XOR
                        stack[sp-1] = b ^ a;
                        break;
                    case 0x74:    // Left Shift
                        stack[sp-1] = b << a;
                        break;
                    case 0x76:    // Right Shift
                        stack[sp-1] = b >> a;
                        break;
                }
            case 0x0B:
                end = false;
                break;
        }
    }
    return stack[sp-1];
}



// Driver main() function
int main(int argc, char **argv){
    int locals[5];
    Stack *s;
    uint8_t *wasm;
    uint32_t pos = 0, four_bytes;
    uint8_t one_byte;
    int byte_idx = 0, byte_size;
    uint8_t sz;
    int pc;
    bool found;
    
    wasm = mapWasmFile(argv[1]);
    byte_size = getsize(argv[1]);
    printf("Byte size: %d \n", byte_size);
  
    Module *m = parse_all_bytes(wasm,byte_size);
    
    char * func_arg = argv[2];
    locals[0] = atoi(argv[3]);
    locals[1] = atoi(argv[4]);
    
    for (int j = 0; j < 5; j++){
      if (strcmp(m->func[j].func_name, func_arg)==0){
          pc = m->func[j].func_pc;
          break;
      }
    }
    
    int result = execute_op(wasm, pc,locals);
    printf("result is %d \n", result); 
}

