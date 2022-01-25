***DISCAIMER*** *: not done yet, this is just information thrown in without sorting or anything*

***INFO*** *: name between `<` and `>` mean they are to be replaced with their corresponding value*

# Naming convention

An **opcode** is a number correspondig to a machine instruction.

A **menemonic** is a string corresponding to one or multiple opcodes.

# Syntax

## General syntax

Comments are in the form of ` //<comment> ` for single lines and ` /*<comment>*/ ` for multi-lines.

Documentation comments are in the form of ` ///<comment> `

Integers are constants used outside of bit patterns. They are in base 10 (decimal). Example : `10 423 -84`

Strings are constants used outside of bit patterns. They are enclosed in double quotes `""`, which can be escaped with a backslash `\`. Example : `"This is a string" "ab" "" "\"escaped quote"`

## Command

Commands start with a percent `%`.

Example :

```
%param opcode-length 15
%enum reg 4
```

## Substitution

Substitutions are in all caps, they used for ...blabla... bit length in parenthesis.

Example :

```
%format jmp_format [$0010,reg,reg,IMMEDIATE(8)]    // IMMEDIATE(8) will be substituted for an immediate value of 8 bits during assembly
%format jmp_format [reg,LABEL_REL(10),io,..]       // LABEL_REL(10) will be substituted for an offset with computed from a label name
```

## Enumeration

Enumeration start with a letter in small caps, and can use any letter or number, or an underscore after that. Enumeration pattern are enclosed in double quotes `"`, they can use any letter, numbers or symbols except white spaces. Double quotes `"` need to be escaped with a backslash `\`. Enumerations are not checked against pattern collison (for now). Use the directive 

Example:

```
%enum reg 4                 // Declaring an enum called "reg" of a lenght of 4 bits
%pattern reg "ax" $1001     // Anytime the assembler find "ax" where a reg enum is expected, it will replace the 4 bits with 1001
%pattern reg "bx" $1011     // Anytime the assembler find "bx" where a reg enum is expected, it will replace the 4 bits with 1011
%pattern reg "cx" $1111     // Anytime the assembler find "cx" where a reg enum is expected, it will replace the 4 bits with 1101
```

## Bit constant

Bit constants start with an `$`. With no postfix the value is considered to be in base 2, postfixes of 'h', 'd', 'o', 'b' respectively mean hexadecimal, decimal, octal, binary. You can either use a numerical value, or define a constant using the directive `%constant`. Defined constants have to be written in small caps and cannot include numbers or symbols except underscodes `_`, but they cannot start with an underscore. They are defined without a leading dollars `$` but needs it when used.

Example:

```
%constant cnt $541Fh                  // Define a constant value of 0x541F
%constant format_0 $0010              // Define a constant value of 0b0010

%format rrr [$cnt,$32d,$format_0]     // Equivalent to "%format rrr [$541Fh,$32d,$0010]"
```

## Bit pattern

Bit patterns are enclosed in square brackets `[]`. You can use a constant, a substitution or an enumeration, they must be separated by a comma `,`. The bit pattern must be of exactly the correct size, or be filled with 0 using an ellipis `...`. 

Examples :
```
%format cmp_ld_type [$0010,reg,reg,IMMEDIATE(8)]    // Opcode using this format will produce a word starting with 0010, then two registers and an imediate
%format jmp_format [$001,ID(2),LABEL_ABS(16),...]         // The remaining bits will be replaced with zeros
```

# Directives

## Opcode formats

Opcode formats are declared using the directive `%format`. This directive declare an opcode format and its corresponding mnemonic format. By default the mnemonic parameters are in the same order as the bit pattern parameters. A format declared without an `ID()` substitution can only be used by one opcode.

Usage : `%format <format name> <bit pattern> `

Example :
```
[blabla some examples]
```

## Reordering parameters

Sometimes we don't want the mnemonic paramters to have the same order as the opcode parameters. They can be reordered using the `%order` directive. The ellipsis `...` and the contants are also indexed but cannot be used as arguments. The substitution `ID()` cannot be used as a parameter. Index starts at 0.

Usage : `%order <format name> <first parameter index> <second parameter index> <third parameter index>`

Example :
```
//Valid
%format cmpld_f   [$0010,reg,reg,IMMEDIATE(8)]     // Mnemonic would be "cmpld <register1> <register2> <immediate>"
%order  cmpld_f   3 2 1                            // Mnemonic would now be "cmpld <immediate> <register2> <register1>

//Not valid
%format format_add [$0110,reg,reg,...,ID(4)]
%order format_add 0 1 2                             // Trying to use a constant as an argument, not valid
```

## Opcodes
Opcodes and mnemonics are declared using the directive `%opcode`. Opcode patterns are enclosed in double quotes `"`, they can use any letter, numbers or symbols except white spaces. They are not check against collision. The id is a bit constant representing the type of instruction inside the opcode format.

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




# Parameters list

Parameter for ASS are declared like this :

```
%param <parameter name> <parameter argument> <parameter argument> ...
```

List of parameters :

| Mandatory | Name                           | Argument                         | Description                    |
| :-------: | ------------------------------ | -------------------------------- | ------------------------------ |
|    YES    | `opcode_width <width>`         | width (integer)                  | Configure the width of the opcodes in bits. This value is fixed, meaning you can not have variable lenght opcodes|
|    YES    | `address_width <width>`        | width (integer)                  | Configure the width of the address bus. |
|    NO    | `separators <seprator list>`   | separator list (string)          | Define the separtor used betweem arguments for mnemonics. By default they are separated by a space. You can define multiple separators.
