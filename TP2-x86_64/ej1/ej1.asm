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
    je .ret_null             ; si sí, devolvemos NULL

    ; Inicializamos los campos: list->first = NULL; list->last = NULL
    mov QWORD [rax], 0       ; first
    mov QWORD [rax + 8], 0   ; last

.ret_null:
    ret

string_proc_node_create_asm:
    ; Guardamos el argumento en rdi (value)
    push rdi                 ; salvamos el valor original en stack

    ; Llamamos a malloc(16)
    mov rdi, 16
    call malloc              ; malloc devuelve puntero en rax

    test rax, rax
    je .ret_null             ; si malloc falla, devolvemos NULL

    ; Restauramos value y seteamos node->value = value
    pop rdx                  ; rdx = value
    mov [rax], rdx           ; node->value

    ; node->next = NULL
    mov QWORD [rax + 8], 0

    ret

.ret_null:
    add rsp, 8               ; limpiamos la pila en caso de early return
    ret

string_proc_list_add_node_asm:
    ; if (list->head == NULL)
    mov rax, [rdi]           ; rax = list->head
    test rax, rax
    jne .else_case           ; si no es NULL, vamos al else

    ; list->head = node
    mov [rdi], rsi
    jmp .set_tail

.else_case:
    ; list->tail->next = node
    mov rcx, [rdi + 8]       ; rcx = list->tail
    mov [rcx + 8], rsi       ; tail->next = node

.set_tail:
    ; list->tail = node
    mov [rdi + 8], rsi
    ret

string_proc_list_concat_asm:
    ; if (list1->head == NULL)
    mov rax, [rdi]            ; rax = list1->head
    test rax, rax
    jne .else_case            ; if list1->head != NULL -> else

    ; list1->head = list2->head
    mov rcx, [rsi]            ; rcx = list2->head
    mov [rdi], rcx
    jmp .check_tail2

.else_case:
    ; list1->tail->next = list2->head
    mov rax, [rdi + 8]        ; rax = list1->tail
    mov rcx, [rsi]            ; rcx = list2->head
    mov [rax + 8], rcx        ; list1->tail->next = list2->head

.check_tail2:
    ; if (list2->tail != NULL)
    mov rax, [rsi + 8]        ; rax = list2->tail
    test rax, rax
    je .end_concat

    ; list1->tail = list2->tail
    mov [rdi + 8], rax

.end_concat:
    ret