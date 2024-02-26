#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "value.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "compiler.h"

VM vm;

void reset_stack() {
    vm.stack_top = vm.stack;
}

static void runtime_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.module->code;
    int line = vm.module->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

void initialize_vm() {
//    vm->module = module;
    reset_stack();
    vm.objects = NULL;
}

void free_vm() {
//    vm->module = NULL;
    free_objects();
}

void push(Value value) {
    // Set the stack top pointer to the new value, then increment the stack top pointer.
    *vm.stack_top = value;
    vm.stack_top++;
}

Value pop() {
    // Decrement the stack top pointer, then return the value that it points to.
    vm.stack_top--;
    return *vm.stack_top;
}

static Value peek(int distance) {
    return vm.stack_top[-1 - distance];
}

static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    // Get the two strings from the stack.
    ObjString *b = AS_STRING(peek(0));
    ObjString *a = AS_STRING(peek(1));

    // determine the length of the new string
    int length = a->length + b->length;

    // allocate the appropriate amount of memory for the new string
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);

    // null-terminate the new string
    chars[length] = '\0';

    // create the new string object and push it onto the stack
    ObjString *result = take_string(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.module->constants.values[READ_BYTE()])
// TODO: Make the invalid operand runtime error more descriptive.
#define BINARY_OP(value_type, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Invalid operands."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(value_type(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // Print the stack.
        printf("          ");
        // Loop through each value in the stack and print it.
        for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        // Disassemble the current instruction.
        disassemble_instruction(vm.module, (int)(vm.ip - vm.module->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                // Get the constant value and push it onto the stack.
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL:
                push(NIL_VAL);
                break;
            case OP_TRUE:
                push(BOOL_VAL(true));
                break;
            case OP_FALSE:
                push(BOOL_VAL(false));
                break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(values_equal(a, b)));
                break;
            }
            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtime_error("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;
            case OP_NOT:
                push(BOOL_VAL(is_falsey(pop())));
                break;
            case OP_NEGATE:
                // Pop a value from the stack, negate it, then push the result back onto the stack.
                if (!IS_NUMBER(peek(0))) {
                    runtime_error("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_RETURN: {
                // Pop the value from the stack and print it.
                print_value(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source) {
    Module module;
    initialize_module(&module);

    if (!compile(source, &module)) {
        free_module(&module);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.module = &module;
    vm.ip = vm.module->code;

    InterpretResult result = run();

    free_module(&module);
    return result;
}
