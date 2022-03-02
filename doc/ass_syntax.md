***DISCAIMER*** *: the following informations may not be up to date. Refer to the examples if needed*

***INFO*** *: name between `<` and `>` mean they are to be replaced with their corresponding value*

# Naming convention

An **opcode** is a serie of bits correspondig to an hardware instruction.

A **mnemonic** is a keyword, possibly followed by arguments, that represent an opcode.

# Syntax

## General syntax

Comments are in the form of ` //<comment> ` for single lines and ` /*<comment>*/ ` for multi-lines. They can only be inserted at the end of a line, or on their own lines. 

Documentation comments are in the form of ` ///<comment> `

***NOTE*** *: Documentation comments are treated as normal comments for now. This may change in the future.*

Integers are constants or literals used outside of bit templates. They are in base 10 (decimal). Example : `10 423 -84`

Strings are constants used outside of bit templates. They are enclosed in double quotes `""`, which can be escaped with a backslash `\`. Example : `"This is a string" "ab" "" "\"escaped quote"`

Identifiers are alphanumerical strings *not* enclosed in double quotes `""`.

***BUG*** *: escaped double quotes sometimes makes the lexer generator crash.*

## Command

Commands start with a percent `%`.

Example :

```
%param opcode-length 15
%enum reg 4
```

## Bit constant

Bit constants start with an `$`. With no postfix the value is considered to be in base 2, postfixes of 'h', 'o', 'b' respectively mean hexadecimal, octal, binary. You can either use a numerical value, or define a constant using the directive `%constant`. Defined constants have to be written in small caps and cannot include numbers or symbols except underscodes `_`, but they cannot start with an underscore. They are defined without a leading dollars `$` but need one when used.

Example:

```
%constant cnt $541Fh                  // Define a constant value of 0x541F
%constant format_0 $0010              // Define a constant value of 0b0010

%format rrr [$cnt,$32d,$format_0]     // Equivalent to "%format rrr [$541Fh,$32d,$0010]"
```

## Substitution

Substitutions are in all caps, followed by a number enclosed in parenthesis. They are special bit sequences substituted for other values. The number in parenthesis represent the width of the substitution in bits.

Example :

```
%format jmp_format [$0010,reg,reg,IMMEDIATE(8)]    // IMMEDIATE(8) will be substituted for an immediate value of 8 bits during assembly
%format jmp_format [reg,LABEL_REL(10),io,..]       // LABEL_REL(10) will be substituted for an offset of 10 bits computed from a label name
```

## Bit template

Bit templates are enclosed in square brackets `[]`. You can use a constant, a substitution or an enumeration, they must be separated by a comma `,`. The bit template must be of the exact width, or be automatically filled with 0 using an ellipis `...`.

Examples :
```
%format cmp_ld_type [$0010,reg,reg,IMMEDIATE(8)]    // Opcode using this format will produce a word starting with 0010, then two registers and an imediate
%format jmp_format [$001,ID(2),LABEL_ABS(16),...]         // The remaining bits will be replaced with zeros
```

# Commands

## Enumeration

Enumeration start with a letter in small caps, and can use any letter or number, or an underscore after that. Enumeration pattern are enclosed in double quotes `"`, they can use any letter, numbers or symbols except white spaces.

To use an enumeration, first declare the enumration itself using the `%enum` command, specifing its name and bit width. Then you can add any number of pattern you want to the enumeration using the `%pattern` command. 

Usage : `%enum <enum name> <width>`

Enumeration patterns behave like a constant during assembly, with the difference that it can only be used where it is expected. This is useful for things like registers where we don't want a register name to be used as any constant.

Usage : `%pattern <enum name> <pattern> <bit value>`

Example:

```
%enum reg 4                 // Declare an enum called "reg" with a lenght of 4 bits
%pattern reg "ax" $1001     // Anytime the assembler find "ax" where a reg enum is expected, it will replace the 4 bits with 1001
%pattern reg "bx" $1011     // Anytime the assembler find "bx" where a reg enum is expected, it will replace the 4 bits with 1011
%pattern reg "cx" $1111     // Anytime the assembler find "cx" where a reg enum is expected, it will replace the 4 bits with 1101
```

## Opcode formats

Opcode formats are declared using the directive `%format`. This directive declare an opcode format and its corresponding mnemonic format. By default the mnemonic parameters are in the same order as the bit template parameters. A format declared without an `ID()` substitution can only be used by one opcode. The format command is the core of the system, as they represent the way the assembler will build the instructions based on the mnemonics.

Usage : `%format <format name> <bit template> `

Example :
```
// Declare a format called "lit_format" with the 2 MSBs set to 1,
//  the 4 following bit identify the opcode,
//  and the last 8 bits contain an immediate
// This format accepts 1 argument in the form of an immediate value
%format lit_format [$11,ID(4),IMMEDIATE(8)]
```

## Reordering parameters

Sometimes we don't want the mnemonic paramters to have the same order as the opcode parameters. They can be reordered using the `%order` directive. The ellipsis `...` and the contants are also indexed but cannot be used as arguments. The substitution `ID()` cannot be used as a parameter. Index starts at 0.

Usage : `%order <format name> <first parameter index> <second parameter index> <third parameter index> <etc...>`

Example :
```
//Valid
%format cmpld_f   [$0010,reg,reg,IMMEDIATE(8)]     // Mnemonic would be "cmpld <register1>,<register2>,<immediate>"
%order  cmpld_f   3 2 1                            // Mnemonic would now be "cmpld <immediate>,<register2>,<register1>"

//Not valid
%format format_add [$0110,reg,reg,...,ID(4)]
%order format_add 0 1 2                             // Trying to use a constant as an argument, not valid
```

## Opcodes
Opcodes and mnemonics are declared using the directive `%opcode`. Opcode patterns are enclosed in double quotes `"`, they can use any letter, numbers or symbols except white spaces. The id is a bit constant representing the type of instruction inside the opcode format.

Usage : `%opcode <opcode name> <mnemonic pattern> <id>`

Example :
```
%format ri [$1010,ID(4),reg,IMMEDIATE(8)]
%order ri 3 4
%opcode add_immediate "addi" $0000          // Create the addition opcode, where ID will be replaced by 0000
%opcode sub_immediate "subi" $0001          // Create the subtraction opcode, where ID will be replaced by 0000
%opcode mul_immediate "muli" $0010          // Create the multiplication opcode, where ID will be replaced by 0000
```

# Substitutions list

 - `ID` Subsituted for the opcode ID, extended/trucated to width. Expects no arguments.
 - `LABEL_ABS` Substituted for the address of label, extended/trucated to width. Expects a label.
 - `LABEL_REL` Substituted for a relative jump from a label in 2's complement. 0 jump means stay where it is. Sign extended/trucated to width. Expects a label.
 - `IMMEDIATE` Substituted for an immediate value, extended/trucated to width. Expects a value which can be an hexadecimal by prefixing it with `0x`, a decimal value by default, an octal value by prefixing it with `0`(zero), a binary value by prefixing with `0b`, or a ASCII char by enclosing it in quotes `'`


# Parameters list

Parameter for ASS are declared like this :

```
%param <parameter name> <parameter argument> <parameter argument> ...
```

List of parameters :

| Mandatory | Name                           | Argument                         | Description                    |
| :-------: | ------------------------------ | -------------------------------- | ------------------------------ |
|    YES    | `opcode_width <width>`         | width (integer)                  | The width of the opcodes in bits. This value is fixed, meaning you can not have variable length opcodes|
|    YES    | `memory_width <width>`         | width (integer)                  | The width of the programm memory in bits. |
|    YES    | `alignment <width>`            | width (integer)                  | The opcode alignment. |
|    YES    | `address_width <width>`        | width (integer)                  | The width of the address bus. |
|    YES    | `address_start <address>`      | address (integer)                | The address were the programm memory starts. Inclusive. |
|    YES    | `address_stop <address>`       | address (integer)                | The address were the programm memory stops. Inclusive. |
|    YES    | `endianness <endianness>`      | endianness (string)              | The endianness of the opcodes. Either "big" or "little". Big endian means LSB first (bit order is reversed), little endian means MSB first. |
|    NO    | `args_separator <separator>`    | separator (string)               | Define the separator used betweem arguments for mnemonics. By default they are separated by a comma. The separator used is the first character in the string sequence.
|    NO    | `label_postfix <postfix>`       | postfix (string)               | Define the postfix for declaring labels. By default labels end with a colon `:`. The postfix used is the first character in the string.
