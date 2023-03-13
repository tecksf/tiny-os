#include <picirq.h>
#include <trap.h>
#include <x86.h>
#include "keyboard.h"

#define NO              0

#define SHIFT           (1<<0)
#define CTL             (1<<1)
#define ALT             (1<<2)

#define CAPSLOCK        (1<<3)
#define NUM_LOCK        (1<<4)
#define SCROLL_LOCK     (1<<5)

#define E0ESC           (1<<6)

#define C(x) (x - '@')

static uint8 normal_map[256] = {
        NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
        '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
        'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
        'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
        'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
        '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
        'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
        NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
        NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
        '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
        '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
        [0xC7] KEY_HOME,    [0x9C] '\n' /*KP_Enter*/,
        [0xB5] '/' /*KP_Div*/,  [0xC8] KEY_UP,
        [0xC9] KEY_PGUP,    [0xCB] KEY_LF,
        [0xCD] KEY_RT,      [0xCF] KEY_END,
        [0xD0] KEY_DN,      [0xD1] KEY_PGDN,
        [0xD2] KEY_INS,     [0xD3] KEY_DEL
};

static uint8 shift_map[256] = {
        NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
        '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
        'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
        'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
        'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
        '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
        'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
        NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
        NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
        '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
        '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
        [0xC7] KEY_HOME,    [0x9C] '\n' /*KP_Enter*/,
        [0xB5] '/' /*KP_Div*/,  [0xC8] KEY_UP,
        [0xC9] KEY_PGUP,    [0xCB] KEY_LF,
        [0xCD] KEY_RT,      [0xCF] KEY_END,
        [0xD0] KEY_DN,      [0xD1] KEY_PGDN,
        [0xD2] KEY_INS,     [0xD3] KEY_DEL
};

static uint8 ctrl_map[256] = {
        NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
        NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
        C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
        C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
        C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
        NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
        C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
        [0x97] KEY_HOME,
        [0xB5] C('/'),      [0xC8] KEY_UP,
        [0xC9] KEY_PGUP,    [0xCB] KEY_LF,
        [0xCD] KEY_RT,      [0xCF] KEY_END,
        [0xD0] KEY_DN,      [0xD1] KEY_PGDN,
        [0xD2] KEY_INS,     [0xD3] KEY_DEL
};

static uint8 shift_code[256] = {
        [0x1D] CTL,
        [0x2A] SHIFT,
        [0x36] SHIFT,
        [0x38] ALT,
        [0x9D] CTL,
        [0xB8] ALT
};

static uint8 toggle_code[256] = {
        [0x3A] CAPSLOCK,
        [0x45] NUM_LOCK,
        [0x46] SCROLL_LOCK
};

static uint8 *char_code[4] = {
        normal_map,
        shift_map,
        ctrl_map,
        ctrl_map
};


static int process_keyboard_input(void)
{
    int c;
    uint8 data;
    static uint32 shift;

    // 输出缓冲寄存器没有数据
    if ((inb(KBSTATP) & KBS_DIB) == 0)
    {
        return -1;
    }

    data = inb(KBDATAP);

    if (data == 0xE0)
    {
        // E0 打头的扩展码
        shift |= E0ESC;
        return 0;
    }
    else if (data & 0x80)
    {
        // 断码，即键盘松开
        data = (shift & E0ESC ? data : data & 0x7F);
        shift &= ~(shift_code[data] | E0ESC);
        return 0;
    }
    else if (shift & E0ESC)
    {
        // Last character was an E0 escape; or with 0x80
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shift_code[data];
    shift ^= toggle_code[data];

    c = char_code[shift & (CTL | SHIFT)][data];
    if (shift & CAPSLOCK)
    {
        if ('a' <= c && c <= 'z')
            c += 'A' - 'a';
        else if ('A' <= c && c <= 'Z')
            c += 'a' - 'A';
    }

    // Process special keys
    // Ctrl-Alt-Del: reboot
    if (!(~shift & (CTL | ALT)) && c == KEY_DEL)
    {
        // printf("Rebooting!\n");
        outb(0x92, 0x3); // courtesy of Chris Frost
    }
    return c;
}

void keyboard_init(void)
{
    pic_enable(IRQ_KBD);
}

int keyboard_interrupt(void)
{
    return process_keyboard_input();
}