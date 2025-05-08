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
    ; Reserva memoria para una lista vacía y la inicializa con NULLs
    mov rdi, 16
    call malloc

    test rax, rax               ; Verifica si rax es NULL
    je .malloc_failed           ; Si es NULL, salta a la etiqueta de fallo

    ; Inicializa la lista
    mov QWORD [rax], 0          ; Inicializa el primer puntero a NULL
    mov QWORD [rax + 8], 0      ; Inicializa el segundo puntero a NULL

    ret

.malloc_failed:
    xor rax, rax                ; Asigna NULL a rax
    ret

string_proc_node_create_asm:
    ; Reserva memoria para un nodo de cadena y lo inicializa
    push rdi                        ; Guarda el tipo
    push rsi                        ; Guarda el hash
    mov rdi, 32                     ; Tamaño del nodo
    call malloc
    pop rsi                         ; Restaura el hash
    pop rdi                         ; Restaura el tipo

    test rax, rax                   ; Verifica si rax es NULL
    je .malloc_failed               ; Si es NULL, salta a la etiqueta de fallo

    mov QWORD [rax], 0              ; Inicializa el puntero siguiente a NULL
    mov QWORD [rax + 8], 0          ; Inicializa el puntero anterior a NULL
    mov BYTE [rax + 16], dil        ; Almacena el tipo
    mov QWORD [rax + 24], rsi       ; Almacena el hash

    ret

.malloc_failed:
    xor rax, rax                    ; Asigna NULL a rax
    ret

string_proc_list_add_node_asm:
    ; Agrega un nodo a la lista de cadenas
    push rdi                            ; Guarda el puntero de la lista
    mov rdi, rsi                        ; Mueve el tipo al registro rdi
    mov rsi, rdx                        ; Mueve el hash al registro rsi
    call string_proc_node_create_asm    ; Crea un nuevo nodo
    pop rdi                             ; Restaura el puntero de la lista

    test rax, rax                       ; Verifica si rax es NULL
    je .return                          ; Si es NULL, salta a la etiqueta de retorno

    mov r8, rax                         ; Carga el nuevo nodo en r8
    mov r9, [rdi]                       ; Carga el puntero al primer nodo
    test r9, r9                         ; Verifica si r9 es NULL
    jz .empty_list                      ; Si está vacía, salta a la lógica de lista vacía

    ; Caso lista no vacía
    mov r10, [rdi + 8]                  ; Carga el puntero al último nodo
    mov [r8 + 8], r10                   ; Establece el siguiente nodo del nuevo nodo
    mov [r10], r8                       ; Establece el nuevo nodo como el anterior del siguiente
    mov [rdi + 8], r8                   ; Actualiza el puntero de la lista al nuevo nodo
    jmp .return

.empty_list:
    ; Manejo de caso de lista vacía
    mov [rdi], r8                       ; Establece el primer nodo de la lista
    mov [rdi + 8], r8                   ; Establece el puntero de siguiente del primer nodo

.return:
    ret

string_proc_list_concat_asm:
    ; Concatena dos listas de cadenas
    mov r8, rdx                         
    mov r9, [rdi]
    mov r10, rsi                ; Guarda el tipo enn r10
    mov r11, rdx                ; Guarda el hash original en r11

.loop:
    test r9, r9                 ; Verifica si se ha llegado al final de la lista
    je .done                    ; Si es así, salta a la etiqueta de fin

    movzx rax, byte [r9 + 16]   ; Carga el tipo del nodo actual
    cmp rax, r10                ; Compara los tipos
    jne .next                   ; Si no son iguales, salta al siguiente nodo

    mov rdi, r8
    mov rsi, [r9 + 24]
    push r8                     ; Guarda el puntero de la lista de destino
    push r9                     ; Guarda el puntero del nodo actual
    push r10                    ; Guarda el valor a comparar
    push r11                    ; Guarda el puntero de la lista de destino
    call str_concat             ; Llama a la función de concatenación
    pop r11                     ; Restaura el puntero de la lista de destino
    pop r10                     ; Restaura el valor a comparar
    pop r9                      ; Restaura el puntero del nodo actual
    pop r8                      ; Restaura el puntero de la lista de destino

    cmp r8, r11                 ; Compara el resultado de la concatenación
    je .skip_free               ; Si son iguales, salta a la etiqueta de omitir liberación
    mov rdi, r8
    push rax                    ; Guarda el resultado de la concatenación
    push r9                     ; Guarda el puntero del nodo actual
    push r10                    ; Guarda el tipo
    push r11                    ; Guarda el hash original
    call free                   ; Libera la memoria del resultado anterior
    pop r11                     ; Restaura el hash original
    pop r10                     ; Restaura el tipo
    pop r9                      ; Restaura el puntero del nodo actual
    pop rax                     ; Restaura el resultado de la concatenación

.skip_free:
    mov r8, rax                 ; Actualiza r8 con el nuevo puntero de la cadena concatenada

.next:
    mov r9, [r9]                ; Avanza al siguiente nodo en la lista origen
    jmp .loop

.done:
    mov rax, r8                 ; Retorna el puntero a la lista concatenada
    ret