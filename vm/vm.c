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
    initialize_table(&vm.globals);
    initialize_table(&vm.strings);
}

void free_vm() {
//    vm->module = NULL;
    free_table(&vm.globals);
    free_table(&vm.strings);
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
    // Return the stack element that is 'distance' elements below the top of the stack.
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
// grabs the next two bytes from the chunk and builds a 16-bit unsigned integer from them.
// TODO: When the VM is ported to a wider bit system, this will need to be increased.
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))
#define READ_CONSTANT() (vm.module->constants.values[READ_BYTE()])
#define READ_STRING() (AS_STRING(READ_CONSTANT()))
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
        for (Value *slot = vm.stack; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        // Disassemble the current instruction.
        disassemble_instruction(vm.module, (int) (vm.ip - vm.module->code));
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
            case OP_POP:
                pop();
                break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm.stack[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm.stack[slot] = peek(0);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString *name = READ_STRING();
                Value value;
                if (!table_get(&vm.globals, name, &value)) {
                    runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString *name = READ_STRING();
                table_set(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString *name = READ_STRING();
                if (table_set(&vm.globals, name, peek(0))) {
                    table_delete(&vm.globals, name);
                    runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
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
            case OP_LESS_EQUAL:
                BINARY_OP(BOOL_VAL, <=);
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
            case OP_PRINTLN:
                print_value(pop());
                printf("\n");
                break;
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (is_falsey(peek(0))) vm.ip += offset;
                break;
            }
            case OP_JUMP_IF_TRUE: {
                uint16_t offset = READ_SHORT();
                if (!is_falsey(peek(0))) vm.ip += offset;
                break;
            }
            case OP_LOOP: {
                // the only difference between this and OP_JUMP is that the offset is negative
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }
            case OP_BUILD_LIST: {
                // stack before: [a, b, c, ...] and after: [list]
                ObjList *list = allocate_list();
                uint8_t count = READ_BYTE();

                // add each element to the list
                for (int i = 0; i < count; i++) {
                    append_to_list(list, peek(count - i - 1));
                }

                // pop the list items from the stack
                while (count--) {
                    pop();
                }

                // push the list onto the stack
                push(OBJ_VAL(list));
                break;
            }
            case OP_BUILD_RANGE: {
                // Retrieve end and start of the range from the stack
                Value endValue = pop();
                Value startValue = pop();

                // Ensure both values are numbers
                if (!IS_NUMBER(startValue) || !IS_NUMBER(endValue)) {
                    runtime_error("Range boundaries must be numbers.");
                }

                double start = AS_NUMBER(startValue);
                double end = AS_NUMBER(endValue);

                // Allocate and initialize the range object
                ObjRange *range = allocate_range(start, end);

                // Push the range object onto the stack
                push(OBJ_VAL(range));
                break;
            }
            case OP_INDEX_LIST: {
                // stack before: [list, index] and after: [index(list, index)]
                Value stack_index = pop();
                Value stack_list = pop();
                Value result;

                // ensure that the value is a list
                if (!IS_LIST(stack_list)) {
                    runtime_error("Index operator must be used with a list.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjList *list = AS_LIST(stack_list);

                if (!IS_NUMBER(stack_index)) {
                    runtime_error("Index must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                int index = AS_NUMBER(stack_index);

                if (!is_valid_index(list, index)) {
                    runtime_error("Index out of bounds.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                result = read_list(list, index);
                push(result);
                break;
            }
            case OP_STORE_LIST: {
                // Stack before: [list, index, item] and after: [item]
                Value item = pop();
                Value stack_index = pop();
                Value stack_list = pop();

                if (!IS_LIST(stack_list)) {
                    runtime_error("Cannot store value in a non-list.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjList *list = AS_LIST(stack_list);

                if (!IS_NUMBER(stack_index)) {
                    runtime_error("List index is not a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                int index = AS_NUMBER(stack_index);

                if (!is_valid_index(list, index)) {
                    runtime_error("Invalid list index.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                store_list(list, index, item);
                push(item);
                break;
            }
            case OP_INCREMENT: {
                // Currently only supports incrementing numbers.
                // stack before: [value] and after: [value + 1]
                if (IS_NUMBER(peek(0))) {
                    double incrementedValue = AS_NUMBER(pop()) + 1; // Increment the value
                    push(NUMBER_VAL(incrementedValue)); // Push the incremented value back onto the stack
                    break;
                }

                double incrementedValue = AS_NUMBER(pop()) + 1; // Increment the value
                push(NUMBER_VAL(incrementedValue)); // Push the incremented value back onto the stack
                break;
            }
            case OP_RANGE_START: {
                // stack before: [range] and after: [range, start]
                ObjRange *range = AS_RANGE(peek(0));
                push(NUMBER_VAL(range->start));
                break;
            }
            case OP_RANGE_END: {
                // stack before: [range] and after: [range, end]
                ObjRange *range = AS_RANGE(peek(0));
                push(NUMBER_VAL(range->end));
                break;
            }
            case OP_INCREMENT_RANGE: {
                // stack before: [range, value] and after: [range, value + 1]
                Value value = pop();
                ObjRange *range = AS_RANGE(peek(0));

                if (!IS_NUMBER(value)) {
                    runtime_error("Range increment value must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                // ensure the value is within the range
                if (AS_NUMBER(value) < range->start || AS_NUMBER(value) > range->end) {
                    runtime_error("Increment value is out of range.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                double incrementedValue = AS_NUMBER(value) + 1; // Increment the value
                push(NUMBER_VAL(incrementedValue)); // Push the incremented value back onto the stack
                break;
            }
            case OP_RETURN: {
                // exit interpreter
                return INTERPRET_OK;
            }
            case OP_DUPLICATE: {
                // stack before: [value] and after: [value, value]
                push(peek(0));
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
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
