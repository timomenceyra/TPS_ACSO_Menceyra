; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    ; Llamamos a malloc(sizeof(string_proc_list)) = malloc(16)
    mov rdi, 16              ; tamaño del struct
    call malloc              ; devuelve puntero en rax


    test rax, rax            ; ¿es NULL?
    je .malloc_failed             ; si sí, devolvemos NULL

    ; Inicializamos los campos: list->first = NULL; list->last = NULL
    mov QWORD [rax], 0       ; first
    mov QWORD [rax + 8], 0   ; last

    ; Devolvemos el puntero a la lista
    ret

.malloc_failed:
    ; Manejo de error de malloc
    xor rax, rax             ; si malloc falla, devolvemos NULL
    ret

string_proc_node_create_asm:
    ; Argumentos: rdi = type (uint8_t), rsi = hash (char*)
    ; Llamamos a malloc(sizeof(string_proc_node)) = malloc(32)
    push rdi                 ; guardamos type
    push rsi                 ; guardamos hash
    mov rdi, 32              ; tamaño del struct
    call malloc              ; devuelve puntero en rax
    pop rsi                  ; recuperamos hash
    pop rdi                  ; recuperamos type

    test rax, rax            ; ¿es NULL?
    je .malloc_failed             ; si sí, devolvemos NULL

    ; Inicializamos los campos: node->next = NULL; node->prev = NULL; node->type = type; node->hash = hash
    mov QWORD [rax], 0       ; next
    mov QWORD [rax + 8], 0   ; prev
    mov BYTE [rax + 16], dil ; type
    mov QWORD [rax + 24], rsi; hash

    ; Devolvemos el puntero al nodo
    ret

.malloc_failed:
    ; Manejo de error de malloc
    xor rax, rax             ; si malloc falla, devolvemos NULL
    ret

string_proc_list_add_node_asm:
    ; Argumentos: rdi = list, rsi = type (uint8_t), rdx = hash (char*)

    ; Llamamos a string_proc_node_create_asm(type, hash)
    push rdi             ; guardamos list
    mov rdi, rsi         ; pasamos type
    mov rsi, rdx         ; pasamos hash
    call string_proc_node_create_asm
    pop rdi              ; recuperamos list

    test rax, rax        ; ¿es NULL?
    je .return           ; si sí, devolvemos NULL


    mov r8, rax        ; guardamos el nuevo nodo en r8
    mov r9, [rdi]      ; r9 = list->first
    test r9, r9        ; ¿list->first == NULL?
    jz .empty_list     ; si sí, vamos a la lista vacía

    ; Caso lista no vacía
    mov r10, [rdi + 8] ; r10 = list->last
    mov [r8 + 8], r10  ; new_node->prev = list->last
    mov [r10], r8      ; list->last->next = new_node
    mov [rdi + 8], r8  ; list->last = new_node
    jmp .return

.empty_list:
    ; Caso lista vacía
    mov [rdi], r8      ; list->first = new_node
    mov [rdi + 8], r8  ; list->last = new_node

.return:
    ret

string_proc_list_concat_asm:
    ; Argumentos: rdi = list, rsi, type (uint8_t), rdx = hash (char*)

    mov r8, rdx       ; guardamos el hash en r8
    mov r9, [rdi]     ; r9 = list->first
    mov r10, rsi      ; r10 = type
    mov r11, rdx      ; r11 = hash original

.loop:
    test r9, r9       ; ¿current == NULL?
    je .done          ; si sí, terminamos

    movzx rax, byte [r9 + 16] ; rax = current->type
    cmp rax, r10      ; comparamos con el tipo
    jne .next         ; si no son iguales, vamos al siguiente nodo

    ; Concatenamos el hash
    mov rdi, r8       ; rdi = hash
    mov rsi, [r9 + 24] ; rsi = current->hash
    push r8
    push r9
    push r10
    push r11
    call str_concat
    pop r11
    pop r10
    pop r9
    pop r8

    cmp r8, r11       ; comparamos el nuevo hash con el original
    je .skip_free     ; si son iguales, no liberamos
    mov rdi, r8       ; rdi = nuevo hash
    push rax
    push r9
    push r10
    push r11
    call free         ; liberamos el hash anterior
    pop r11
    pop r10
    pop r9
    pop rax           ; recuperamos el nuevo hash

.skip_free:
    mov r8, rax       ; guardamos el nuevo hash en r8

.next:
    mov r9, [r9]      ; r9 = current->next
    jmp .loop         ; volvemos al inicio del bucle

.done:
    ; Devolvemos el hash concatenado
    mov rax, r8
    ret